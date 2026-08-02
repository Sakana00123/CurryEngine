#include "pch.h"
#include "Shaker.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(Shaker, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(Shaker, "UserScripts", ComponentAttributes::None, {})


void Shaker::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}

void Shaker::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。

	if (shakeDuration > 0.0f)
	{
		shakeDuration -= deltaTime;
		float value = 0.0f;
		easingHandler.Update(value, deltaTime);
		float shakeAmount = shakeMagnitude * value; // イージングされたシェイクの強さ
		// シェイクのオフセットをランダムに生成
		float offsetX = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * shakeAmount;
		float offsetZ = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * shakeAmount;

		// Transform にオフセットを適用
		if (Transform* transform = GetTransform())
		{
			transform->SetPosition(originalPosition + Vector3(offsetX, 0.0f, 0.0f));
		}
	}
	// シェイクが終了したら元の位置に戻す
	else if (shakeDuration <= 0.0f && shakeMagnitude > 0.0f)
	{
		if (Transform* transform = GetTransform())
		{
			transform->SetPosition(originalPosition);
		}
		shakeMagnitude = 0.0f; // シェイクの強さをリセット
	}
}

void Shaker::Shake(float duration, float magnitude)
{
	shakeDuration = duration;
	shakeMagnitude = magnitude;

	easingHandler.Clear();
	easingHandler.AddEasing(EaseType::OutCubic, 0.0f, 1.0f, duration);

}

//void Shaker::DrawProperty()
//{
//	// エディタでプロパティを描画するための処理をここに実装します。
//#ifdef USE_IMGUI
//
//	static float duration = 0.5f;
//	static float magnitude = 0.01f;
//
//	ImGui::InputFloat("Duration", &duration);
//
//	ImGui::InputFloat("Magnitude", &magnitude);
//
//
//	if (ImGui::Button("Shake"))
//	{
//		Shake(duration, magnitude); // 例: duration秒間、強さmagnitudeのシェイクを開始
//	}
//
//#endif // USE_IMGUI
//}
