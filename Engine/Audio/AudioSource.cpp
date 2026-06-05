#include "pch.h"
#include "AudioSource.h"
#include "Engine/Editor/Dialog.h"
#include "Engine/Utils/stdUtiles.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Core/GameObject.h"
#include "3DAudio.h"
#include "Engine/Editor/AssetBrowser.h"

#include <x3daudio.h>

REGISTER_COMPONENT(AudioSource, "Audio")

void AudioSource::SetSource(const std::wstring& filePath)
{
	// 再生中なら停止
	Stop();
	if (sourceVoice)
	{
		sourceVoice->DestroyVoice();
	}
	// ファイルパスに "BGM" が含まれていれば BGM、含まれていなければ SE として扱う
	this->type = std::wstring(filePath).find(L"BGM") != std::wstring::npos ? SoundType::BGM : SoundType::SE;
	// ファイルパスを保存
	this->filePath = filePath;
	// ループ設定
	this->loop = (this->type == SoundType::BGM) ? true : false;
	// オーディオバッファを取得
	m_SptrBuffer = Audio::AudioBuffer::GetResource(filePath);
	if (!m_SptrBuffer)
	{
		std::wstring message = L"Failed to load audio: " + filePath;
		std::string utf8Message = std::string(message.begin(), message.end());
		LOG_ERROR(utf8Message.c_str());
		return;
	}
	// ソースボイスを作成
	Audio::CreateAudioSource(m_SptrBuffer, &sourceVoice, type);
}
AudioSource::~AudioSource()
{
	Stop();
	if (sourceVoice)
	{
		sourceVoice->DestroyVoice();
	}
}

void AudioSource::Start()
{
	if (playOnStart)
	{
		Play();
	}
}

void AudioSource::Update(float deltaTime)
{
	//バッファやソースボイスが設定されていなければ何もしない
	if (!m_SptrBuffer || !sourceVoice)
	{
		return;
	}

	// オーディオアナライザの更新
	if (m_AudioAnalyzer)
	{
		m_AudioAnalyzer->Update(deltaTime);
	}

	//音源が3D音源として扱わない場合は何もしない
	if (!use3DAudio) return;
	
#ifdef X3DAUDIO
	// 3D音源として扱う場合はリスナーと音源の位置関係から音量やパンを計算する
	C3DAudio::Culculate3DAudio(this->gameObject);
#endif // X3DAUDIO
}

void AudioSource::Play()
{
	//バッファやソースボイスが設定されていなければ何もしない
	if (!m_SptrBuffer || !sourceVoice)
	{
		return;
	}

	HRESULT hr = S_OK;

	XAUDIO2_VOICE_STATE voiceState = {};
	sourceVoice->GetState(&voiceState);

	// すでに再生中なら何もしない
	if (voiceState.BuffersQueued)
	{
		//Stop(false, 0);
		return;
	}

	// 再生停止(テイル無し)
	Stop(false);

	// バッファをソースボイスにセット
	XAUDIO2_BUFFER* pBuffer = &m_SptrBuffer->buffer;
	pBuffer->LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;
	hr = sourceVoice->SubmitSourceBuffer(pBuffer);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	// BGM の場合はオーディオアナライザを初期化
	if (type == SoundType::BGM)
	{
		// オーディオアナライザを初期化
		m_AudioAnalyzer = std::make_unique<AudioAnalyzer>();
		m_AudioAnalyzer->Initialize(sourceVoice, m_SptrBuffer.get());
		// 再生基準をリセット
		m_AudioAnalyzer->ResetPlaybackBase();
	}

	// 再生開始
	hr = sourceVoice->Start(0);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void AudioSource::Stop(bool playTails)
{
	//バッファやソースボイスが設定されていなければ何もしない
	if (!m_SptrBuffer || !sourceVoice)
	{
		return;
	}

	XAUDIO2_VOICE_STATE voiceState{};
	sourceVoice->GetState(&voiceState);

	// 再生中でなければ何もしない
	if (!voiceState.BuffersQueued)
	{
		return;
	}

	HRESULT hr;
	hr = sourceVoice->Stop(playTails ? XAUDIO2_PLAY_TAILS : 0);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	hr = sourceVoice->FlushSourceBuffers();
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	// SamplesPlayed リセット
	hr = sourceVoice->Discontinuity();
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	// オーディオアナライザをクリア
	m_AudioAnalyzer.reset();
}

void AudioSource::Pause()
{
	//バッファやソースボイスが設定されていなければ何もしない
	if (!m_SptrBuffer || !sourceVoice)
	{
		return;
	}
	HRESULT hr = sourceVoice->Stop(0);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void AudioSource::Resume()
{
	//バッファやソースボイスが設定されていなければ何もしない
	if (!m_SptrBuffer || !sourceVoice)
	{
		return;
	}
	HRESULT hr = sourceVoice->Start(0);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void AudioSource::SetVolume(float volume)
{
	this->volume = volume;

	//バッファやソースボイスが設定されていなければ何もしない
	if (!m_SptrBuffer || !sourceVoice)
	{
		return;
	}
	HRESULT hr = sourceVoice->SetVolume(volume);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

float AudioSource::GetVolume()
{
	float volume = 0.0f;
	//バッファやソースボイスが設定されていなければ何もしない
	if (!m_SptrBuffer || !sourceVoice)
	{
		return volume;
	}
	sourceVoice->GetVolume(&volume);
	this->volume = volume;
	return volume;
}

void AudioSource::SetUse3DAudio(bool use3D)
{
	use3DAudio = use3D;
}

bool AudioSource::GetUse3DAudio() const
{
	return use3DAudio;
}

bool AudioSource::IsPlaying()
{
	XAUDIO2_VOICE_STATE voiceState{};
	sourceVoice->GetState(&voiceState);
	return voiceState.BuffersQueued > 0;
}

void AudioSource::SetPan(float pan)
{
	m_Pan = std::clamp(pan, -1.0f, 1.0f);

#if 0
	// TODO:変化無し
	float angle = (m_Pan + 1.0f) * (XM_PI / 4.0f); // -1.0 ～ 1.0 を 0 ～ π/2 に変換
	//float left = cosf(angle);
	//float right = sinf(angle);

	// 入力がステレオなら左と右を平均化してパンを適用
	float leftInputGain = cosf(angle);
	float rightInputGain = sinf(angle);

	// 左入力を左/右に割り振る
	// 右入力を左/右に割り振る
	float matrix[4] = {
		leftInputGain, rightInputGain,  // 左入力
		leftInputGain, rightInputGain   // 右入力
	};
	sourceVoice->SetOutputMatrix(nullptr, 2, 2, matrix);

#else
	// TODO:何故か完全に左に寄ってしまう
	float outputMatrix[8] = {};
	float left = 1.0f - (m_Pan * 0.5f + 0.5f);
	float right = m_Pan * 0.5f + 0.5f;

	DWORD channelMask;
	Audio::masterVoice->GetChannelMask(&channelMask);
	switch (channelMask)
	{
	case SPEAKER_MONO:
	{
		outputMatrix[0] = (left + right) * 0.5f; // Mono
		break;
	}
	case SPEAKER_STEREO:
	case SPEAKER_2POINT1:
	case SPEAKER_SURROUND:
	{
		outputMatrix[0] = left;  // Front Left
		outputMatrix[1] = right; // Front Right
		break;
	}
	case SPEAKER_QUAD:
	{
		outputMatrix[0] = left;  // Front Left
		outputMatrix[1] = right; // Front Right
		outputMatrix[2] = left;  // Back Left
		outputMatrix[3] = right; // Back Right
		break;
	}
	case SPEAKER_4POINT1:
	{
		outputMatrix[0] = left;  // Front Left
		outputMatrix[1] = right; // Front Right
		outputMatrix[2] = 0.0f;  // Front Center
		outputMatrix[3] = 0.0f;  // Low Frequency
		outputMatrix[4] = left;  // Back Left
		outputMatrix[5] = right; // Back Right
		break;
	}
	case SPEAKER_5POINT1:
	{
		outputMatrix[0] = left;  // Front Left
		outputMatrix[1] = right; // Front Right
		outputMatrix[2] = 0.0f;  // Front Center
		outputMatrix[3] = 0.0f;  // Low Frequency
		outputMatrix[4] = left;  // Back Left
		outputMatrix[5] = right; // Back Right
		break;
	}
	case SPEAKER_7POINT1:
	{
		outputMatrix[0] = left;  // Front Left
		outputMatrix[1] = right; // Front Right
		outputMatrix[2] = 0.0f;  // Front Center
		outputMatrix[3] = 0.0f;  // Low Frequency
		outputMatrix[4] = left;  // Side Left
		outputMatrix[5] = right; // Side Right
		outputMatrix[6] = left;  // Back Left
		outputMatrix[7] = right; // Back Right
		break;
	}
	case SPEAKER_5POINT1_SURROUND:
	{
		outputMatrix[0] = left;  // Front Left
		outputMatrix[1] = right; // Front Right
		outputMatrix[2] = 0.0f;  // Front Center
		outputMatrix[3] = 0.0f;  // Low Frequency
		outputMatrix[4] = 0.0f;  // Side Left
		outputMatrix[5] = 0.0f;  // Side Right
		outputMatrix[6] = left;  // Back Left
		outputMatrix[7] = right; // Back Right
		break;
	}
	case SPEAKER_7POINT1_SURROUND:
	{
		outputMatrix[0] = left;  // Front Left
		outputMatrix[1] = right; // Front Right
		outputMatrix[2] = 0.0f;  // Front Center
		outputMatrix[3] = 0.0f;  // Low Frequency
		outputMatrix[4] = 0.0f;  // Side Left
		outputMatrix[5] = 0.0f;  // Side Right
		outputMatrix[6] = left;  // Back Left
		outputMatrix[7] = right; // Back Right
		break;
	}
	default:
		break;
	}

	XAUDIO2_VOICE_DETAILS voiceDetails;
	sourceVoice->GetVoiceDetails(&voiceDetails);

	XAUDIO2_VOICE_DETAILS masterVoiceDetails;
	Audio::masterVoice->GetVoiceDetails(&masterVoiceDetails);

	XAUDIO2_VOICE_DETAILS subVoiceDetails;
	Audio::submixVoices[type]->GetVoiceDetails(&subVoiceDetails);


	HRESULT hr = sourceVoice->SetOutputMatrix(NULL, voiceDetails.InputChannels, subVoiceDetails.InputChannels, outputMatrix);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
#endif // 0

}

void AudioSource::SetLoopOption(float begin, float length)
{
	//バッファやソースボイスが設定されていなければ何もしない
	if (!m_SptrBuffer || !sourceVoice)
	{
		return;
	}
	XAUDIO2_BUFFER* pBuffer = &m_SptrBuffer->buffer;
	const WAVEFORMATEX& format = m_SptrBuffer->wfx.Format;

	const UINT32 sampleRate = format.nSamplesPerSec;
	const UINT32 blockAlign = format.nBlockAlign;
	const UINT32 totalSamples = pBuffer->AudioBytes / blockAlign;

	UINT32 loopBegin = static_cast<UINT32>(sampleRate * begin);
	UINT32 loopLength = static_cast<UINT32>(sampleRate * length);

	// 範囲チェック
	if (loopBegin >= totalSamples) {
		loopBegin = 0;
		loopLength = 0;
		pBuffer->LoopCount = 0; // 無効化
	}
	else if (loopBegin + loopLength > totalSamples) {
		loopLength = totalSamples - loopBegin;
	}

	pBuffer->LoopBegin = loopBegin;
	pBuffer->LoopLength = loopLength;
}

float AudioSource::GetPlaybackTime() const
{
	//バッファやソースボイスが設定されていなければ 0 を返す
	if (!m_SptrBuffer || !sourceVoice)
	{
		return 0.0f;
	}
	XAUDIO2_VOICE_STATE voiceState{};
	sourceVoice->GetState(&voiceState);
	const WAVEFORMATEX& format = m_SptrBuffer->wfx.Format;
	const UINT32 sampleRate = format.nSamplesPerSec;
	const float playbackTime = static_cast<float>(voiceState.SamplesPlayed) / static_cast<float>(sampleRate);
	return playbackTime;
}

float AudioSource::GetPlaybackDeltaTime()
{
	//バッファやソースボイスが設定されていなければ 0 を返す
	if (!m_SptrBuffer || !sourceVoice)
	{
		return 0.0f;
	}
	XAUDIO2_VOICE_STATE state{};
	sourceVoice->GetState(&state);

	uint64_t samples = state.SamplesPlayed;

	// 前回のサンプル数からの差分を計算して秒数に変換
	float deltaTime = (samples - m_LastSamplesPlayed) / static_cast<float>(m_SptrBuffer->wfx.Format.nSamplesPerSec);
	m_LastSamplesPlayed = static_cast<float>(samples);
	return deltaTime;
}


uint32_t AudioSource::GetBufferQueueCount()
{
	//バッファやソースボイスが設定されていなければ 0 を返す
	if (!m_SptrBuffer || !sourceVoice)
	{
		return 0;
	}
	XAUDIO2_VOICE_STATE voiceState{};
	sourceVoice->GetState(&voiceState);
	return voiceState.BuffersQueued;
}

float AudioSource::GetTotalDuration() const
{
	// バッファが設定されていなければ 0 を返す
	if (!m_SptrBuffer)
	{
		return 0.0f;
	}
	return m_SptrBuffer->GetDuration();
}

const AudioAnalyzerResult& AudioSource::GetAudioAnalyzerResult() const
{
	static AudioAnalyzerResult emptyResult;
	if (m_AudioAnalyzer)
	{
		return m_AudioAnalyzer->GetAnalyzerResult();
	}
	return emptyResult;
}

const AudioAnalyzer::Result& AudioSource::GetAudioAnalyzerBasicResult() const
{
	static AudioAnalyzer::Result emptyResult;
	if (m_AudioAnalyzer)
	{
		return m_AudioAnalyzer->GetResult();
	}
	return emptyResult;
}

void AudioSource::DrawProperty()
{
#ifdef USE_IMGUI
	// ファイル選択ダイアログ
	if (ImGui::Button("Source")) {
		static const char* filter = "Audio Files(*.wav*)\0*.wav*;\0All Files(*.*)\0*.*;\0\0)";

		char filePath[256] = { 0 };
		HWND hwnd = Graphics::GetHwnd();
		DialogResult result = Dialog::OpenFileName(filePath, sizeof(filePath), filter, nullptr, hwnd);
		if (result == DialogResult::OK) {
			std::wstring wpath = StringToWstring(std::string(filePath));
			SetSource(wpath);
		}
	}
	// ドラッグ＆ドロップ対応
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
			const char* p = static_cast<const char*>(payload->Data);
			std::filesystem::path path = p ? p : "";
			AssetType assetType = AssetBrowser::DetectAssetTypeFromFile(path);
			if (assetType == AssetType::Sound) {
				SetSource(path.wstring());
			}
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::SameLine();
	// 現在のファイルパスを表示
	std::wstring path = filePath.empty() ? L"No file" : filePath;
	if (!std::filesystem::exists(path)) {
		path = L"File not found: " + path;
	}

	ImGui::Text(std::string(path.begin(), path.end()).c_str());

	// ソースボイスが設定されていなければ何もしない
	if (sourceVoice)
	{
		// 3D音源関連の設定
		{
			// 3D音源として扱うか
			ImGui::Checkbox("3D Audio", &use3DAudio);

			// パン設定
			if (ImGui::SliderFloat("Pan", &m_Pan, -1.0f, 1.0f))
			{
				SetPan(m_Pan);
			}
		}

#if 0
		ImGui::Separator();
		// ループ設定
		static float loopBegin = 0.0f;
		static float loopLength = 0.0f;
		if (ImGui::InputFloat("Loop Begin", &loopBegin)) {
			loopBegin = max(0.0f, loopBegin);
			SetLoopOption(loopBegin, loopLength);
		}
		if (ImGui::InputFloat("Loop Length", &loopLength)) {
			loopLength = max(0.0f, loopLength);
			SetLoopOption(loopBegin, loopLength);
		}
#endif // 0

		// 再生時の自動再生
		ImGui::Checkbox("PlayOnStart", &playOnStart);

		// ループ有無
		ImGui::Checkbox("Loop", &loop);


		// 再生・停止ボタン
		if (ImGui::Button("Play")) {
			Play();
		}
		if (ImGui::Button("Stop")) {
			Stop();
		}

		// 音量スライダー
		volume = GetVolume();
		if (ImGui::SliderFloat("Volume", &volume, 0, 1)) {
			SetVolume(volume);
		}
	}

	ImGui::Separator();

	// マスター音量、BGM音量、SE音量
	{
		if (ImGui::SliderFloat("MasterVolume", &masterVolume, 0, 1)) {
			Audio::SetMasterVolume(masterVolume);
		}
		if (ImGui::SliderFloat("BgmVolume", &bgmVolume, 0, 1)) {
			Audio::SetBgmVolume(bgmVolume);
		}
		if (ImGui::SliderFloat("SeVolume", &seVolume, 0, 1)) {
			Audio::SetSeVolume(seVolume);
		}
	}

	// オーディオアナライザの結果を表示
	if (m_AudioAnalyzer)
	{
		const AudioAnalyzer::Result& result = m_AudioAnalyzer->GetResult();
		ImGui::Text("RMS: %.4f", result.rms);
		ImGui::Text("Peak: %.4f", result.peak);
		// スペクトルデータの簡易表示
		ImGui::PlotLines("Spectrum", result.spectrum.data(), static_cast<int>(result.spectrum.size()), 0, nullptr, 0.0f, 1.0f, ImVec2(0, 100));
		
		// 周波数帯域ごとのレベル表示
		const AudioAnalyzerResult& analyzerResult = m_AudioAnalyzer->GetAnalyzerResult();
		ImGui::Text("Low Peak: %.4f", analyzerResult.lowPeak);
		ImGui::Text("Mid Peak: %.4f", analyzerResult.midPeak);
		ImGui::Text("High Peak: %.4f", analyzerResult.highPeak);

		// 低周波数帯域のレベル表示
		ImGui::PlotLines("Low Bands", 
			reinterpret_cast<const float*>(analyzerResult.lowBands.data()), 
			static_cast<int>(analyzerResult.lowBands.size()), 
			0, nullptr, 0.0f, 1.0f, ImVec2(0, 100));

		// 中周波数帯域のレベル表示
		ImGui::PlotLines("Mid Bands",
			reinterpret_cast<const float*>(analyzerResult.midBands.data()),
			static_cast<int>(analyzerResult.midBands.size()),
			0, nullptr, 0.0f, 1.0f, ImVec2(0, 100));

		// 高周波数帯域のレベル表示
		ImGui::PlotLines("High Bands",
			reinterpret_cast<const float*>(analyzerResult.highBands.data()),
			static_cast<int>(analyzerResult.highBands.size()),
			0, nullptr, 0.0f, 1.0f, ImVec2(0, 100));

		// 全体のRMS値
		ImGui::Text("Overall RMS: %.4f", analyzerResult.overallRMS);
	}


#endif // USE_IMGUI
}

json AudioSource::Serialize() const
{
	json j;
	j["filePath"] = std::string(filePath.begin(), filePath.end());
	j["soundType"] = type;
	j["playOnStart"] = playOnStart;
	j["loop"] = loop;
	j["use3DAudio"] = use3DAudio;
	j["volume"] = volume;
	return j;
}

void AudioSource::Deserialize(const json& j)
{
	if (j.contains("filePath"))
	{
		std::string path = j["filePath"].get<std::string>();
		SetSource(std::wstring(path.begin(), path.end()));
	}
	if (j.contains("soundType"))
	{
		type = static_cast<SoundType>(j["soundType"].get<int>());
	}
	if (j.contains("playOnStart"))
	{
		playOnStart = j["playOnStart"].get<bool>();
	}
	if (j.contains("loop"))
	{
		loop = j["loop"].get<bool>();
	}
	if (j.contains("use3DAudio"))
	{
		use3DAudio = j["use3DAudio"].get<bool>();
	}
	if (j.contains("volume"))
	{
		volume = j["volume"].get<float>();
		SetVolume(volume);
	}
}