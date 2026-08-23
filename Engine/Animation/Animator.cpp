#include "pch.h"
#include "Animator.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Rendering/Renderers/GltfModelRenderer.h"

#include "Engine/Resources/AnimationClip.h"
#include "Engine/Resources/AssetDatabase.h"

REGISTER_COMPONENT(Animator, "Animation")


void Animator::Initialize()
{
	// 初期化処理をここに実装する
	// 例: コントローラーの初期化、アニメーションの準備など
	controller = CurryEngine::Resources::AssetDatabase::LoadAsset<AnimatorController>(controllerAssetId);
	if (controller)
    {
		runtimeController.Initialize(*controller);
    }
}

void Animator::Update(float deltaTime)
{
	// コントローラーが存在する場合、アニメーションの更新処理を行う
	if (controller)
	{
		// コントローラーの更新処理を呼び出す
		runtimeController.Update(deltaTime, *controller);

		// GltfModelRenderer にアニメーションを適用する
		if (targetModelRendererId.IsValid())
		{
			if (auto renderer = GetScene()->FindComponentById<GltfModelRenderer>(targetModelRendererId))
			{
				renderer->ApplyPose(runtimeController.GetPose());
			}
		}

	}
}

#ifdef USE_IMGUI
void Animator::DrawProperty(const PropertyDrawContext& context)
{
    Component::DrawProperty(context); // 基底クラスの描画を呼び出す
    if (context.targets.size() != 1)
    {
        ImGui::Text("Multiple selection is not supported for Animator.");
        return;
	}
    if (controller)
    {
		for (const auto& clip : controller->animationClips)
		{
			ImGui::Text("Clip: %s", clip->name.c_str());
		}
		for (int i = 0; i < controller->states.size(); ++i)
		{
			const auto& state = controller->states[i];
			if (ImGui::Button(("Play State: " + state.name).c_str()))
			{
				runtimeController.Play(*controller, i);
			}
		}
	}
}
#endif // USE_IMGUI
