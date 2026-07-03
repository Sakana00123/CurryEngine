#include "pch.h"
#include "AnimationClip.h"

//REGISTER_RESOURCE(AnimationClip, "AnimationClip")


void ValueTrack::Evaluate(float prevTime, float currentTime, AnimationContext& context)
{
	
}

void ValueTrack::Sort()
{
	std::sort(keys.begin(), keys.end(),
		[](const ValueKeyframe& a, const ValueKeyframe& b)
		{
			return a.time < b.time;
		});
}

void EventTrack::Evaluate(float prevTime, float currentTime, AnimationContext& context)
{
	// イベントトラックの評価ロジックをここに実装
}

void CurveTrack::Evaluate(float prevTime, float currentTime, AnimationContext& context)
{
	// カーブトラックの評価ロジックをここに実装
}

void StateTrack::Evaluate(float prevTime, float currentTime, AnimationContext& context)
{
	// ステートトラックの評価ロジックをここに実装
}