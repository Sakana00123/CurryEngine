#include "pch.h"
#include "AudioAnalyzer.h"
#include "Engine/Audio/Audio.h"

void AudioAnalyzer::Initialize(IXAudio2SourceVoice* voice, const Audio::AudioBuffer* buffer)
{
	// ソースボイスとオーディオバッファを保持
	sourceVoice = voice;
	wfx = &buffer->wfx;
	pcmData = buffer->buffer.pAudioData;
	pcmSize = buffer->buffer.AudioBytes;

	// FFT 用設定を作成
	fftConfig = kiss_fftr_alloc(fftSize, 0, nullptr, nullptr);
	spectrumSize = fftSize / 2 + 1; // 正規化済みスペクトルサイズ
	result.spectrum.reserve(spectrumSize); // スペクトルデータ用に確保
	previousSmoothed.resize(spectrumSize, 0.0f); // 平滑化済みデータ用に確保
	tempBuffer.reserve(fftSize); // 分析用一時バッファ

	// 相対基準の再取得フラグ
	baseCaptured = false;
	baseSamplesPlayed = 0;
}

void AudioAnalyzer::Update(float deltaTime)
{
	if (!sourceVoice) {
		Console::LogError("AudioAnalyzer: Source voice is not initialized.");
	}
	if (!wfx) {
		Console::LogError("AudioAnalyzer: Wave format is not initialized.");
	}
	if (!pcmData || pcmSize == 0) {
		Console::LogError("AudioAnalyzer: PCM data is not initialized.");
	}
	if (!sourceVoice || !wfx || !pcmData || pcmSize == 0) return;

	// 再生位置を取得
	XAUDIO2_VOICE_STATE state = {};
	sourceVoice->GetState(&state);

	// 再生開始後の最初の Update で相対基準を確定
	if (!baseCaptured) {
		baseSamplesPlayed = state.SamplesPlayed;
		baseCaptured = true;
	}

	const uint32_t sampleRate = wfx->Format.nSamplesPerSec;
	const uint16_t channels = wfx->Format.nChannels;
	const uint16_t bits = wfx->Format.wBitsPerSample;

	if (bits != 16)
	{
		// 現状 16bit PCM のみ対応
		Console::LogError("AudioAnalyzer: Only 16-bit PCM is supported.");
		return;
	}

	size_t bytesPerSample = bits / 8;
	size_t frameSize = bytesPerSample * channels;

	// 現在の再生サンプル位置（今回の再生に対する相対位置）を計算
	uint64_t relSamplesPlayed = state.SamplesPlayed - baseSamplesPlayed;
	uint64_t totalSamples = pcmSize / frameSize;

	if (totalSamples == 0)
	{
		Console::LogError("AudioAnalyzer: Total samples is zero.");
		return;
	}

	// ループ処理を考慮して再生サンプル位置を調整
	uint32_t playSample = static_cast<uint32_t>(relSamplesPlayed % totalSamples);

	// 分析ウィンドウ = 1024 サンプル分を取得(モノラル変換)
	constexpr size_t windowSamples = 1024;
	tempBuffer.resize(windowSamples);

	const short* pcm16 = reinterpret_cast<const short*>(pcmData);

	for (size_t i = 0; i < windowSamples; ++i)
	{
		size_t sampleIndex = (playSample + i) % totalSamples; // ループを考慮したサンプルインデックス
		// 1chのみ使用 (とりあえず)
		tempBuffer[i] = static_cast<float>(pcm16[sampleIndex * channels]) / 32768.0f;
	}

	// RMS とピーク値を計算
	ComputeRMSAndPeak(tempBuffer.data(), windowSamples);

	// FFT 処理
	AnalyzeFFT(tempBuffer.data(), windowSamples);

	// 周波数帯域ごとのレベルを更新
	UpdateBands();
}

void AudioAnalyzer::ComputeRMSAndPeak(const float* samples, size_t sampleCount)
{
	float sumSq = 0.0f;
	float peak = 0.0f;

	for (size_t i = 0; i < sampleCount; ++i)
	{
		float s = samples[i];
		sumSq += s * s;
		peak = (std::max)(peak, std::abs(s));
	}

	result.rms = sqrtf(sumSq / static_cast<float>(sampleCount));
	result.peak = peak;
}

void AudioAnalyzer::AnalyzeFFT(const float* samples, size_t sampleCount)
{
	if (!fftConfig) {
		Console::LogError("AudioAnalyzer: FFT config is not initialized.");
		return;
	}

	// kissFFT 用の入力バッファと出力バッファを準備
	std::vector<kiss_fft_cpx> fftOutput(spectrumSize);

	// FFT 実行
	kiss_fftr(fftConfig, samples, fftOutput.data());

	// スペクトルデータを計算（振幅スペクトル）
	result.spectrum.clear();
	result.spectrum.resize(spectrumSize);
	for (int i = 0; i < spectrumSize; ++i)
	{
		float real = fftOutput[i].r;
		float imag = fftOutput[i].i;
		float magnitude = sqrtf(real * real + imag * imag);
		result.spectrum[i] = magnitude;
	}
}

void AudioAnalyzer::UpdateBands()
{
	// バンド分割のインデックス計算
	size_t lowEnd = result.spectrum.size() / 4; // 低周波数帯域の終了インデックス(0 ~ 1/4 : Bass)
	size_t midEnd = result.spectrum.size() / 2; // 中周波数帯域の終了インデックス(1/4 ~ 1/2 : Mid)
	size_t highEnd = result.spectrum.size();    // 高周波数帯域の終了インデックス(1/2 ~ 1 : Treble)

	// バンドデータのリサイズ
	analyzerResult.lowBands.resize(lowEnd);
	analyzerResult.midBands.resize(midEnd - lowEnd);
	analyzerResult.highBands.resize(highEnd - midEnd);

	// 最大値の計算
	float lowMax = 0.0f;
	for (size_t i = 0; i < lowEnd; i++) lowMax = (std::max)(lowMax, result.spectrum[i]);
	float midMax = 0.0f;
	for (size_t i = lowEnd; i < midEnd; i++) midMax = (std::max)(midMax, result.spectrum[i]);
	float highMax = 0.0f;
	for (size_t i = midEnd; i < highEnd; i++) highMax = (std::max)(highMax, result.spectrum[i]);

	// 正規化と平滑化のラムダ関数
	auto normalizeAndSmooth = [this](float raw, float& smoothed, float maxVal) -> float
		{
			// 正規化と平滑化
			//float norm = maxVal > 0.0f ? (raw / maxVal) : 0.0f;
			float norm = raw; // 正規化を無効化
			smoothed = smoothed * 0.8f + norm * 0.2f; // 平滑化
			return norm;
		};

	// 低周波数帯域の更新
	analyzerResult.lowPeak = 0.0f;
	for (size_t i = 0; i < lowEnd; ++i)
	{
		float sm = previousSmoothed[i];
		float val = normalizeAndSmooth(result.spectrum[i], sm, lowMax);
		analyzerResult.lowBands[i].value = val;
		analyzerResult.lowBands[i].smoothed = sm;
		analyzerResult.lowPeak = (std::max)(analyzerResult.lowPeak, val);
		previousSmoothed[i] = sm;
	}

	// 中周波数帯域の更新
	analyzerResult.midPeak = 0.0f;
	for (size_t i = lowEnd; i < midEnd; ++i)
	{
		float sm = previousSmoothed[i];
		float val = normalizeAndSmooth(result.spectrum[i], sm, midMax);
		analyzerResult.midBands[i - lowEnd].value = val;
		analyzerResult.midBands[i - lowEnd].smoothed = sm;
		analyzerResult.midPeak = (std::max)(analyzerResult.midPeak, val);
		previousSmoothed[i] = sm;
	}

	// 高周波数帯域の更新
	analyzerResult.highPeak = 0.0f;
	for (size_t i = midEnd; i < highEnd; ++i)
	{
		float sm = previousSmoothed[i];
		float val = normalizeAndSmooth(result.spectrum[i], sm, highMax);
		analyzerResult.highBands[i - midEnd].value = val;
		analyzerResult.highBands[i - midEnd].smoothed = sm;
		analyzerResult.highPeak = (std::max)(analyzerResult.highPeak, val);
		previousSmoothed[i] = sm;
	}

	// 全体のRMS値を更新
	analyzerResult.overallRMS = result.rms;
}
