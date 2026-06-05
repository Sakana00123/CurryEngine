#include "pch.h"
#include "BeatManager.h"


void BeatManager::Initialize()
{
	previousBeatCount = -1;
	beatCount = 0;
	isBeatTiming = false;
	bpm = 128.0f;
	beatInterval = 60.0f / bpm;
	previousSongTime = 0.0f;
	songTime = 0.0f;
	beatTolerance = 0.1f;
}

bool BeatManager::IsBeatTiming()
{
	float timeInCurrentBeat = fmod(songTime, beatInterval);
	return (timeInCurrentBeat <= beatTolerance) || (timeInCurrentBeat >= (beatInterval - beatTolerance));
}

void BeatManager::Update(float deltaTime)
{
	// 曲の再生時間の更新
	previousSongTime = songTime;
	songTime += deltaTime;

	// ビートカウントの更新
	beatCount = static_cast<int>(songTime / beatInterval);

	// 初期化
	isBeatTiming = false;

	// 前回のビートカウントと比較して、ビートが変わったかをチェック
	if (beatCount != previousBeatCount)
	{
		previousBeatCount = beatCount;

		// ビートが変わったときの処理
		isBeatTiming = true;
	}
}

