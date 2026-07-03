#include "pch.h"
#include "Audio.h"
#include "3DAudio.h"

HRESULT FindChunk(HANDLE hfile, DWORD fourcc, DWORD& chunkSize, DWORD& chunkDataPosition)
{
	HRESULT hr = S_OK;

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hfile, 0, NULL, FILE_BEGIN))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	DWORD chunkType;
	DWORD chunkDataSize;
	DWORD riffDataSize = 0;
	DWORD fileType;
	DWORD bytesRead = 0;
	DWORD offset = 0;

	while (hr == S_OK)
	{
		DWORD numberOfBytesRead;
		if (0 == ReadFile(hfile, &chunkType, sizeof(DWORD), &numberOfBytesRead, NULL))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		if (0 == ReadFile(hfile, &chunkDataSize, sizeof(DWORD), &numberOfBytesRead, NULL))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		switch (chunkType)
		{
		case 'FFIR'://RIFF
			riffDataSize = chunkDataSize;
			chunkDataSize = 4;
			if (0 == ReadFile(hfile, &fileType, sizeof(DWORD), &numberOfBytesRead, NULL))
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
			break;
		default:
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hfile, chunkDataSize, NULL, FILE_CURRENT))
			{
				return HRESULT_FROM_WIN32(GetLastError());
			}
		}

		offset += sizeof(DWORD) * 2;

		if (chunkType == fourcc)
		{
			chunkSize = chunkDataSize;
			chunkDataPosition = offset;
			return S_OK;
		}

		offset += chunkDataSize;

		if (bytesRead >= riffDataSize)
		{
			return S_FALSE;
		}
	}

	return S_OK;
}

HRESULT ReadChunkData(HANDLE hfile, LPVOID buffer, DWORD bufferSize, DWORD bufferOffset)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hfile, bufferOffset, NULL, FILE_BEGIN))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	DWORD numberOfBytesRead;
	if (0 == ReadFile(hfile, buffer, bufferSize, &numberOfBytesRead, NULL))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
	return hr;
}

void Audio::CreateAudioSource(std::shared_ptr<AudioBuffer> buffer,
	IXAudio2SourceVoice** sourceVoice, SoundType type)
{
	// BGM,SEの送り先設定
	XAUDIO2_SEND_DESCRIPTOR send[SoundType::EnumCount] = { { 0, submixVoices[BGM] }, { 0, submixVoices[SE] } };
	XAUDIO2_VOICE_SENDS sends[SoundType::EnumCount] = { {1, &send[BGM]}, {1, &send[SE]} };

	HRESULT hr = xaudio2->CreateSourceVoice(sourceVoice, (WAVEFORMATEX*)&buffer->wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, &sends[type], nullptr);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void Audio::Initialize()
{
	HRESULT hr;

	//XAudio2の初期化
	hr = XAudio2Create(xaudio2.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	hr = xaudio2->CreateMasteringVoice(&masterVoice);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	// BGMとSE用のサブミキサー・ボイスを作成（ステレオ: 2チャンネル）
	hr = xaudio2->CreateSubmixVoice(&submixVoices[BGM], 2, 44100);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	hr = xaudio2->CreateSubmixVoice(&submixVoices[SE], 2, 44100);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	// X3DAudio の初期化
	C3DAudio::Initialize();

	// ボリュームの初期値を設定
	SetSeVolume(0.5f);
}

Audio::~Audio()
{
	masterVoice->DestroyVoice();
	submixVoices[0]->DestroyVoice();
	submixVoices[1]->DestroyVoice();
	xaudio2->Release();
}

float Audio::AudioBuffer::GetDuration() const
{
	// 再生時間 = バッファサイズ(バイト) / 1秒あたりのバイト数
	const uint32_t byteRate = wfx.Format.nAvgBytesPerSec;
	// byteRate が 0 の場合、無限ループなどで再生時間が定義できない場合があるためガード
	if (byteRate == 0) return 0.0;
	// 秒数を返す
	return static_cast<float>(buffer.AudioBytes) / static_cast<float>(byteRate);
}

std::shared_ptr<Audio::AudioBuffer> Audio::AudioBuffer::GetResource(const std::wstring& filePath)
{
	//すでに存在していたらリソースを返す
	if (resources.contains(filePath) && !resources.at(filePath).expired())
	{
		return resources.at(filePath).lock();
	}

	// ファイルが存在しない場合はエラーログを出して nullptr を返す
	if (!std::filesystem::exists(filePath))
	{
		LOG_ERROR("ファイルが見つかりません: " + std::string(filePath.begin(), filePath.end()));
		return nullptr;
	}

	//存在していなければ新規でオーディオバッファを作成
	{
		HRESULT hr;
		std::shared_ptr<AudioBuffer> audioBuffer = std::make_shared<AudioBuffer>();

		//ファイルオープン
		HANDLE hfile = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (INVALID_HANDLE_VALUE == hfile)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
		}

		if (INVALID_SET_FILE_POINTER == SetFilePointer(hfile, 0, NULL, FILE_BEGIN))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
		}

		DWORD chunkSize;
		DWORD chunkPosition;
		//check the file type, should be 'WAVE' or 'XWMA'
		FindChunk(hfile, 'FFIR'/*RIFF*/, chunkSize, chunkPosition);
		DWORD fileType = 0;
		ReadChunkData(hfile, &fileType, sizeof(DWORD), chunkPosition);
		_ASSERT_EXPR(fileType == 'EVAW'/*WAVE*/, L"Only support 'WAVE'");

		FindChunk(hfile, ' tmf'/*FMT*/, chunkSize, chunkPosition);
		ReadChunkData(hfile, &audioBuffer->wfx, chunkSize, chunkPosition);

		//fill out the audio data buffer with the contents of the fourccDATA chunk
		FindChunk(hfile, 'atad'/*DATA*/, chunkSize, chunkPosition);
		BYTE* data = new BYTE[chunkSize];
		ReadChunkData(hfile, data, chunkSize, chunkPosition);

		audioBuffer->buffer.AudioBytes = chunkSize;  //size of the audio buffer in bytes
		audioBuffer->buffer.pAudioData = data; //buffer containing audio data
		audioBuffer->buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

		resources[filePath] = audioBuffer;
		return audioBuffer;
	}
}

void Audio::PlayOneShot(const wchar_t* filePath, float volume)
{
	std::shared_ptr<StandaloneAudioSource> source = std::make_shared<StandaloneAudioSource>(filePath);
	source->SetVolume(volume);
	source->Play();
	audioSources.emplace_back(source);
}

void Audio::Update(float deltaTime)
{
	//削除処理
	for (auto& source : audioSources)
	{
		if (!source->IsPlaying())
		{
			erases.emplace_back(source);
		}
	}
	audioSources.erase(std::remove_if(audioSources.begin(), audioSources.end(),
		[&](const auto& audioSource) {
			return std::find(erases.begin(), erases.end(), audioSource) != erases.end();
		}),
		audioSources.end());
	erases.clear();
}

StandaloneAudioSource::StandaloneAudioSource(const wchar_t* filePath)
{
	this->type = std::wstring(filePath).find(L"BGM") != std::wstring::npos ? SoundType::BGM : SoundType::SE;
	sptrBuffer = Audio::AudioBuffer::GetResource(filePath);
	if (!sptrBuffer)
	{
		std::wstring logMessage = L"Failed to load audio file: " + std::wstring(filePath);
		std::string logMessageA(logMessage.begin(), logMessage.end());
		LOG_ERROR(logMessageA);
		sourceVoice = nullptr;
		return;
	}
	Audio::CreateAudioSource(sptrBuffer, &sourceVoice, type);
}
StandaloneAudioSource::~StandaloneAudioSource()
{
	Stop();
	sourceVoice->DestroyVoice();
}

void StandaloneAudioSource::Play(bool loop)
{
	_ASSERT_EXPR(sourceVoice, L"ソースが設定されていません。");

	HRESULT hr;

	XAUDIO2_VOICE_STATE voiceState = {};
	sourceVoice->GetState(&voiceState);

	if (voiceState.BuffersQueued)
	{
		//Stop(false, 0);
		return;
	}

	XAUDIO2_BUFFER* pBuffer = &sptrBuffer->buffer;

	pBuffer->LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;
	hr = sourceVoice->SubmitSourceBuffer(pBuffer);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	hr = sourceVoice->Start(0);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void StandaloneAudioSource::Stop(bool playTails)
{
	XAUDIO2_VOICE_STATE voiceState{};
	sourceVoice->GetState(&voiceState);

	if (!voiceState.BuffersQueued)
	{
		return;
	}

	HRESULT hr;
	hr = sourceVoice->Stop(playTails ? XAUDIO2_PLAY_TAILS : 0);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	hr = sourceVoice->FlushSourceBuffers();
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void StandaloneAudioSource::SetVolume(float volume)
{
	HRESULT hr = sourceVoice->SetVolume(volume);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

float StandaloneAudioSource::GetVolume()
{
	float volume;
	sourceVoice->GetVolume(&volume);
	return volume;
}

bool StandaloneAudioSource::IsPlaying()
{
	XAUDIO2_VOICE_STATE voiceState{};
	sourceVoice->GetState(&voiceState);
	return voiceState.BuffersQueued > 0;
}

void StandaloneAudioSource::DrawGUI()
{
#ifdef USE_IMGUI
	if (ImGui::Button("Play")) {
		Play();
	}
	if (ImGui::Button("Stop")) {
		Stop();
	}

	static float volume;
	if (sourceVoice)
	{
		sourceVoice->GetVolume(&volume);
		if (ImGui::SliderFloat("Volume", &volume, 0, 1)) {
			sourceVoice->SetVolume(volume);
		}
	}

	ImGui::Separator();

	if (ImGui::SliderFloat("MasterVolume", &masterVolume, 0, 1)) {
		Audio::SetMasterVolume(masterVolume);
	}
	if (ImGui::SliderFloat("BgmVolume", &bgmVolume, 0, 1)) {
		Audio::SetBgmVolume(bgmVolume);
	}
	if (ImGui::SliderFloat("SeVolume", &seVolume, 0, 1)) {
		Audio::SetSeVolume(seVolume);
	}
#endif // USE_IMGUI
}