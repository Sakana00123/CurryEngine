#include "pch.h"
#include "BlendEasing.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(BlendEasing, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(BlendEasing, "UserScripts", ComponentAttributes::None, {})


void BlendEasing::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}

void BlendEasing::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
	float t;
	easingHandler.Update(t, deltaTime);

	if (easingHandler.IsCompleted())
	{
		// ブレンドが完了したときの処理をここに実装します。
	}
	else if (easingHandler.GetSequenceCount() > 0)
	{
		// ブレンド中の処理をここに実装します。
		Vector3 currentPosition = easingHandler.Lerp<Vector3>(startPosition, targetPosition, t);
		// 例えば、currentPosition をオブジェクトの位置に適用するなどの処理を行うことができます。
		GetTransform()->SetPosition(currentPosition);

		Vector3 currentRotation = easingHandler.Lerp<Vector3>(startRotation, targetRotation, t);
		// 例えば、currentRotation をオブジェクトの回転に適用するなどの処理を行うことができます。
		GetTransform()->SetRotation(currentRotation);
	}
}

//void BlendEasing::DrawProperty()
//{
//#ifdef USE_IMGUI
//	// エディタでプロパティを描画する処理をここに実装します。 
//	Component::DrawProperty(); // 基底クラスのプロパティ描画を呼び出す（必要に応じて）
//
//	const char* easingTypes[] = { 
//		"InQuad", "OutQuad", "InOutQuad",
//		"InCubic", "OutCubic", "InOutCubic",
//		"InQuart", "OutQuart", "InOutQuart",
//		"InQuint", "OutQuint", "InOutQuint",
//		"InSine", "OutSine", "InOutSine",
//		"InExpo", "OutExpo", "InOutExpo",
//		"InCirc", "OutCirc", "InOutCirc",
//		"InBounce", "OutBounce", "InOutBounce",
//		"InBack", "OutBack", "InOutBack",
//		"Linear"
//	};
//	ImGui::Combo("EasingType", &easingType, easingTypes, IM_ARRAYSIZE(easingTypes));
//
//
//	if (ImGui::Button("Start Blend"))
//	{
//		StartBlend(1.0f); // 例: t = 1.0f でブレンドを開始
//	}
//	if (ImGui::Button("Revert Blend"))
//	{
//		StartBlend(0.0f); // 例: t = 0.0f でブレンドをリセット
//	}
//
//#endif // USE_IMGUI
//
//}

void BlendEasing::StartBlend(float t)
{
	// ブレンドを開始する処理をここに実装します。
	// 例えば、t を目標位置として、startPosition から targetPosition へのブレンドを開始するなどの処理を行うことができます。
	easingHandler.Clear(); // 既存のイージング要素をクリア
	easingHandler.AddEasing((EaseType)easingType, (1.0f - t), t, blendDuration);
}
