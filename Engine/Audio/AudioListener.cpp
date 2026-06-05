#include "pch.h"
#include "AudioListener.h"
#include "Audio.h"
#include "Engine/Core/GameObject.h"
#include "3DAudio.h"

#ifdef X3DAUDIO

REGISTER_COMPONENT(AudioListener, "Audio")

AudioListener::~AudioListener()
{
	//自分が現在のリスナーならリセットする
	if (listener == this)
	{
		listener = nullptr;
	}
}

void AudioListener::Awake()
{
	//最初に生成されたリスナーを現在のリスナーとする
	listener = this;
}

void AudioListener::Update(float deltaTime)
{
	
}

#endif // X3DAUDIO