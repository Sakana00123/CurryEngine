#include "pch.h"
#include "EasingHandler.h"

void EasingHandler::AddEasing(EaseType type, float start, float end, float duration, float back)
{
	//処理内容を設定
	EaseItem item{};
	ToEasingFunction(type, item.function, item.backFunction);
	item.easeData.timer = 0.0f;
	item.easeData.totalTime = duration;
	item.easeData.startValue = start;
	item.easeData.endValue = end;

	if (item.backFunction) {
		item.easeData.backValue = back;
	}

	//シーケンスに追加
	sequence.emplace_back(item);

	isCompleted = false;
}

void EasingHandler::AddEasing(const EaseItem& item)
{
	//シーケンスに追加
	sequence.emplace_back(item);

	isCompleted = false;
}

void EasingHandler::AddWait(float waitTime)
{
	//処理内容を設定
	EaseItem item{};
	item.function = nullptr;
	item.easeData.timer = 0.0f;
	item.easeData.totalTime = waitTime;

	//シーケンスに追加
	sequence.emplace_back(item);
	
	isCompleted = false;
}

void EasingHandler::Update(float& value, float deltaTime)
{
	if (sequence.empty()) return;

	auto& item = sequence.front();

	//先頭のイージング処理を実行する
	{
		item.easeData.timer += deltaTime;

		//イージング関数
		if (item.function != nullptr)
			value = item.function(item.easeData.timer, item.easeData.totalTime, item.easeData.endValue, item.easeData.startValue);
		else if (item.backFunction != nullptr) {
			value = item.backFunction(item.easeData.timer, item.easeData.totalTime, item.easeData.backValue, item.easeData.endValue, item.easeData.startValue);
		}
		progress = item.easeData.timer / item.easeData.totalTime;
		easedProgress = value / (item.easeData.endValue - item.easeData.startValue);

		if (item.easeData.timer > item.easeData.totalTime)
		{
			if (item.function != nullptr || item.backFunction != nullptr)
				value = item.easeData.endValue;
			sequence.erase(sequence.begin());
		}
	}
	//全ての補完処理が完了したら完了フラグを立てる
	if (sequence.empty() && !isCompleted)
	{
		isCompleted = true;
		sequence.clear();
		ExecuteCompletedFunction();
		return;
	}
}

void EasingHandler::Clear()
{
	sequence.clear();
	isCompleted = false;
	completeFunction = nullptr;
}

void EasingHandler::ToEasingFunction(EaseType type, std::function<float(float, float, float, float)>& function, std::function<float(float, float, float, float, float)>& backFunction)
{
	function = nullptr;
	backFunction = nullptr;

	switch (type)
	{
	case EaseType::InQuad: function = Easing::InQuad<float>; break;
	case EaseType::OutQuad: function = Easing::OutQuad<float>; break;
	case EaseType::InOutQuad:function = Easing::InOutQuad<float>; break;
	case EaseType::InCubic:function = Easing::InCubic<float>; break;
	case EaseType::OutCubic:function = Easing::OutCubic<float>; break;
	case EaseType::InOutCubic:function = Easing::InOutCubic<float>; break;
	case EaseType::InQuart:function = Easing::InQuart<float>; break;
	case EaseType::OutQuart:function = Easing::OutQuart<float>; break;
	case EaseType::InOutQuart:function = Easing::InOutQuart<float>; break;
	case EaseType::InQuint:function = Easing::InQuint<float>; break;
	case EaseType::OutQuint:function = Easing::OutQuint<float>; break;
	case EaseType::InOutQuint:function = Easing::InOutQuint<float>; break;
	case EaseType::InSine:function = Easing::InSine<float>; break;
	case EaseType::OutSine:function = Easing::OutSine<float>; break;
	case EaseType::InOutSine:function = Easing::InOutSine<float>; break;
	case EaseType::InExp:function = Easing::InExp<float>; break;
	case EaseType::OutExp:function = Easing::OutExp<float>; break;
	case EaseType::InOutExp:function = Easing::InOutExp<float>; break;
	case EaseType::InCirc:function = Easing::InCirc<float>; break;
	case EaseType::OutCirc:function = Easing::OutCirc<float>; break;
	case EaseType::InOutCirc:function = Easing::InOutCirc<float>; break;
	case EaseType::InBounce:function = Easing::InBounce<float>; break;
	case EaseType::OutBounce:function = Easing::OutBounce<float>; break;
	case EaseType::InOutBounce:function = Easing::InOutBounce<float>; break;
	case EaseType::Linear:function = Easing::Linear<float>; break;
	case EaseType::InBack:backFunction = Easing::InBack<float>; break;
	case EaseType::OutBack:backFunction = Easing::OutBack<float>; break;
	case EaseType::InOutBack:backFunction = Easing::InOutBack<float>; break;
	default:
		break;
	}
}