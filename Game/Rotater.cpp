#include "pch.h"
#include "Rotater.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/UI/RectTransform.h"
#include <cmath>

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(Rotater, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(Rotater, "UserScripts", ComponentAttributes::None, {})


// cubic easeInOut: 0→1 を緩急つきで補間
static float EaseInOutCubic(float t)
{
	return t < 0.5f
		? 4.0f * t * t
		: 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

// 呼び出すたびに現在の回転角度(degrees)を返す
// elapsedTime : 経過時間（秒）
// period      : 1ステップの周期（秒）、デフォルト1.2s
static float GetLoadingRotation(float elapsedTime, float period = 1.2f)
{
	float t = std::fmod(elapsedTime, period) / period; // [0, 1)
	float eased = EaseInOutCubic(t);
	return eased * 360.0f;
}

void Rotater::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	if (GetOwner()->GetComponent<RectTransform>())
	{
		is2D = true; // RectTransformがある場合は2D回転とみなす
	}
	else
	{
		is2D = false; // それ以外は3D回転とみなす
	}
}

void Rotater::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
	if (is2D)
	{
		// 2D回転の場合はZ軸回りの回転を行う
		float angleDelta = rotationSpeed * deltaTime; // 回転角の増分（度）
		if (rotateZ)
		{
			if (RectTransform* rectTransform = GetOwner()->GetComponent<RectTransform>())
			{
				if (useEasing)
				{
					// イージングを使用して回転角を計算
					elapsedTime += deltaTime; // 経過時間を更新
					rectTransform->angle = -GetLoadingRotation(elapsedTime); // イージング関数で回転角を設定
				}
				else
				{
					rectTransform->angle += angleDelta; // Z軸回りに回転
				}
			}
		}
	}
	else
	{
		// 3D回転の場合は指定された軸回りの回転を行う
		float angleDelta = rotationSpeed * deltaTime; // 回転角の増分（度）
		if (rotateX)
		{
			GetOwner()->GetTransform()->Rotate({ angleDelta, 0, 0 }); // X軸回りに回転
		}
		if (rotateY)
		{
			GetOwner()->GetTransform()->Rotate({ 0, angleDelta, 0 }); // Y軸回りに回転
		}
		if (rotateZ)
		{
			GetOwner()->GetTransform()->Rotate({ 0, 0, angleDelta }); // Z軸回りに回転
		}
	}
}
