#pragma once
#include <vector>
#include <xaudio2.h>
#include <mmreg.h>
#include "Engine/Audio/Audio.h"
#include "ThirdPerty/kiss_fftr.h"

/**
 * @file
 * @brief オーディオ解析ユーティリティ。
 * @details 音声データの周波数解析や波形解析を行います。
 */


/**
 * @brief スペクトルバンドのレベル値。
 */
struct SpectrumBand
{
	float value = 0.0f; 	//!< バンドのレベル値。(0.0-1.0)
	float smoothed = 0.0f;	//!< 平滑化済みレベル値。(0.0-1.0)
};

/**
 * @brief オーディオ解析結果。
 */
struct AudioAnalyzerResult
{
	std::vector<SpectrumBand> lowBands; 	//!< 低周波数帯域のレベル値群。(Bassの指標)
	std::vector<SpectrumBand> midBands;		//!< 中周波数帯域のレベル値群。(Midの指標)
	std::vector<SpectrumBand> highBands;	//!< 高周波数帯域のレベル値群。(Trebleの指標)

	float lowPeak = 0.0f;   //!< 低周波数帯域のピーク値。(Bassの指標)
	float midPeak = 0.0f;   //!< 中周波数帯域のピーク値。(Midの指標)
	float highPeak = 0.0f;  //!< 高周波数帯域のピーク値。(Trebleの指標)

	float overallRMS = 0.0f;  //!< 全体のRMS値。(音量感の指標)
};

/**
 * @brief オーディオ解析クラス。
 * @details 音声データのRMS/ピーク値計算やFFT解析を行います。
 */
class AudioAnalyzer
{
public:
	/** @brief デストラクタ。*/
	~AudioAnalyzer()
	{
		if (fftConfig) {
			free(fftConfig);
			fftConfig = nullptr;
		}
	}

	/**
	 * @brief 解析結果。
	 */
	struct Result
	{
		float rms; 						//!< RMS（Root Mean Square）値。
		float peak; 					//!< ピーク値。
		std::vector<float> spectrum;	// <! スペクトルデータ。

		/** @brief 指定周波数帯域の平均値を取得します。
		 *  @param freqStart 開始周波数（Hz）。
		 *  @param freqEnd 終了周波数（Hz）。
		 *  @param sampleRate サンプルレート（Hz）。デフォルトは 44100 Hz。
		 *  @result 指定帯域の平均値。
		 */
		[[nodiscard]] float GetBandValue(float freqStart, float freqEnd, int sampleRate = 44100) const
		{
			if (spectrum.empty() || freqStart >= freqEnd || sampleRate <= 0) {
				return 0.0f;
			}
			int fftSize = (static_cast<int>(spectrum.size()) - 1) * 2; // FFT サイズを計算
			int startIndex = static_cast<int>((freqStart / sampleRate) * fftSize);
			int endIndex = static_cast<int>((freqEnd / sampleRate) * fftSize);
			startIndex = std::clamp(startIndex, 0, static_cast<int>(spectrum.size()) - 1);
			endIndex = std::clamp(endIndex, 0, static_cast<int>(spectrum.size()) - 1);
			float sum = 0.0f;
			for (int i = startIndex; i <= endIndex; ++i) {
				sum += spectrum[i];
			}
			return sum / static_cast<float>(endIndex - startIndex + 1);
		}
	};

	/** @brief 初期化処理。*/
	void Initialize(IXAudio2SourceVoice* voice, const Audio::AudioBuffer* buffer);

	/** @brief フレーム更新。*/
	void Update(float deltaTime);

	/** @brief 解析結果を取得します。*/
	[[nodiscard]] const Result& GetResult() const { return result; }

	/** @brief オーディオ解析の詳細結果を取得します。*/
	[[nodiscard]] const AudioAnalyzerResult& GetAnalyzerResult() const { return analyzerResult; }

	/** @brief 再生基準位置のリセット。*/
	void ResetPlaybackBase() { baseCaptured = false; }
private:

	/** @brief RMS とピーク値を計算します。*/
	void ComputeRMSAndPeak(const float* samples, size_t sampleCount);

	/** @brief FFT 解析を行います。*/
	void AnalyzeFFT(const float* samples, size_t sampleCount);

	/** @brief 周波数帯域ごとのレベルを更新します。*/
	void UpdateBands();

private:
	IXAudio2SourceVoice* sourceVoice = nullptr;
	const WAVEFORMATEXTENSIBLE* wfx = nullptr;

	const BYTE* pcmData = nullptr;
	size_t pcmSize = 0;

	Result result = {};
	std::vector<float> previousSmoothed; // 前回の平滑化済みデータ
	AudioAnalyzerResult analyzerResult = {};
	std::vector<float> tempBuffer; // 分析用一時バッファ（floatに変換）

	kiss_fftr_cfg fftConfig = nullptr;
	int fftSize = 1024; // ウィンドウサイズ
	int spectrumSize = fftSize / 2 + 1; // スペクトルサイズ

	// 再生ごとの相対位置計算用の基準
	uint64_t baseSamplesPlayed = 0;
	bool baseCaptured = false;
};