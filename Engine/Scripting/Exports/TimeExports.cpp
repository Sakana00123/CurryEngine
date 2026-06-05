#include "pch.h"
#include "Engine/Core/Time.h"

ENGINE_API float Time_GetDeltaTime()
{
	return Time::DeltaTime();
}

ENGINE_API float Time_GetUnscaledDeltaTime()
{
	return Time::UnscaledDeltaTime();
}

ENGINE_API float Time_GetTimeScale()
{
	return Time::timeScale;
}

ENGINE_API void Time_SetTimeScale(float scale)
{
	Time::timeScale = scale;
}