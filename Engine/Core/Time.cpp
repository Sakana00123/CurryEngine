#include "pch.h"
#include "Time.h"

Time::Time()
{
	// 高精度タイマーの周波数を取得
	LONGLONG countsPerSec = 0;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countsPerSec));
	secondsPerCount = 1.0 / static_cast<double>(countsPerSec);

	// 現在のカウントを取得
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&thisTime));
	baseTime = thisTime;
	lastTime = thisTime;

	timeScale = 1.0f;
}

void Time::Reset()
{
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&thisTime));
	baseTime = thisTime;
	lastTime = thisTime;

	stopTime = 0;
	stopped = false;
}

void Time::Start()
{
	LONGLONG startTime = 0;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&startTime));

	if (stopped) {
		pausedTime += (startTime - stopTime);
		lastTime = startTime;
		stopTime = 0;
		stopped = false;
	}
}

void Time::Stop()
{
	if (!stopped) {
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&stopTime));
		stopped = true;
	}
}

void Time::Tick()
{
	if (stopped) {
		deltaTime = 0.0;
		return;
	}

	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&thisTime));

	deltaTime = (thisTime - lastTime) * secondsPerCount * static_cast<double>(timeScale);
	unscaledDeltaTime = (thisTime - lastTime) * secondsPerCount;

	lastTime = thisTime;

	if (deltaTime < 0.0) {
		deltaTime = 0.0;
	}
}

float Time::TimeStamp() const
{
	if (stopped) {
		return static_cast<float>(((stopTime - pausedTime) - baseTime) * secondsPerCount);
	}
	else {
		return static_cast<float>(((thisTime - pausedTime) - baseTime) * secondsPerCount);
	}
}