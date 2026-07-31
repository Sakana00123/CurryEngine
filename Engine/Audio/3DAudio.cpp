#include "pch.h"
#include "3DAudio.h"
#include "Engine/Core/GameObject.h"
#ifdef X3DAUDIO
#include "Audio.h"
#include "AudioListener.h"
#include "AudioSource.h"
#include "Engine/Editor/Console.h"
#include <x3daudio.h>

void C3DAudio::Initialize()
{
	//X3DAudioの初期化
	DWORD channelMask;
	Audio::masterVoice->GetChannelMask(&channelMask);
	channelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT; //ステレオ環境を想定
	X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, x3dAudioHandle);
}

void C3DAudio::Culculate3DAudio(GameObject* source)
{
	if (AudioListener* listener = AudioListener::GetListener())
	{
		//リスナーと音源の位置
		XMFLOAT3 sourcePos = source->transform->GetWorldPosition();
		XMFLOAT3 listenerPos = listener->GetOwner()->transform->GetWorldPosition();
		
		//リスナーの前方ベクトルと右ベクトル、上ベクトルを取得
		XMFLOAT3 listenerForward = listener->GetOwner()->transform->GetForward();
		XMFLOAT3 listenerRight = listener->GetOwner()->transform->GetRight();
		XMFLOAT3 listenerUp = listener->GetOwner()->transform->GetUp();

		listenerForward.z = -listenerForward.z;

		//音源の前方ベクトル
		//XMFLOAT3 emitterForward = source->transform->GetForward();
		XMFLOAT3 emitterForward = { 0,0,0 };
		//XMFLOAT3 emitterRight = source->transform->GetRight();
		XMFLOAT3 emitterRight = { 0,0,0 };
		//XMFLOAT3 emitterUp = source->transform->GetUp();
		XMFLOAT3 emitterUp = { 0,0,0 };
		
		X3DAUDIO_LISTENER listenerData = {};
		listenerData.OrientFront = listenerForward; // 前方ベクトル
		listenerData.OrientTop = listenerUp; // 上ベクトル
		listenerData.Position = listenerPos; // プレイヤーの位置
		listenerData.Velocity = { 0,0,0 }; // プレイヤーの速度（静止している場合は0）
		listenerData.pCone = nullptr; // 音源のコーン（使用しない場合はnullptr）

		X3DAUDIO_EMITTER emitterData = {};
		emitterData.Position = sourcePos; // 音源の位置
		emitterData.Velocity = { 0,0,0 }; // 音源の速度（静止している場合は0）
		emitterData.OrientFront = emitterForward; // 音源の前方ベクトル
		emitterData.OrientTop = emitterUp; // 音源の上ベクトル
		emitterData.ChannelCount = 1;
		emitterData.CurveDistanceScaler = 1.0f;

		X3DAUDIO_DSP_SETTINGS dspSettings = {};
		FLOAT32 matrix[XAUDIO2_MAX_AUDIO_CHANNELS * XAUDIO2_MAX_AUDIO_CHANNELS] = {};
		dspSettings.SrcChannelCount = 1; // モノラル音源なら1
		dspSettings.DstChannelCount = 2; // モノラル音源をステレオに変換するので2
		dspSettings.pMatrixCoefficients = matrix;

		// 3Dオーディオの計算
		X3DAudioCalculate(x3dAudioHandle,
			&listenerData,
			&emitterData,
			X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER,
			&dspSettings);

		// 計算結果のログ出力（デバッグ用）
		LOG_INFO("Left Volume: " + std::to_string(dspSettings.pMatrixCoefficients[0]));
		LOG_INFO("Right Volume: " + std::to_string(dspSettings.pMatrixCoefficients[1]));
		LOG_INFO("Doppler Factor: " + std::to_string(dspSettings.DopplerFactor));

		//AudioSourceコンポーネントを取得
		if (AudioSource* audioSource = source->GetComponent<AudioSource>())
		{
			//計算結果をAudioSourceに反映
			//audioSource->SetVolume(dspSettings.pMatrixCoefficients[0]); // 左チャンネルの音量を設定
			//audioSource->SetPan(dspSettings.pMatrixCoefficients[1] - dspSettings.pMatrixCoefficients[0]); // パンを設定（右-左）
			audioSource->sourceVoice->SetOutputMatrix(NULL, dspSettings.SrcChannelCount, dspSettings.DstChannelCount, matrix); // 出力マトリックスを設定
			audioSource->sourceVoice->SetFrequencyRatio(dspSettings.DopplerFactor); // ドップラー効果の周波数比を設定
		}
	}
}


#endif // X3DAUDIO
