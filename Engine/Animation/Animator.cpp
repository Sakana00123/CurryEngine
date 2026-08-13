#include "pch.h"
#include "Animator.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Rendering/Renderers/GltfModelRenderer.h"

#include "Engine/Resources/AnimationClip.h"

REGISTER_COMPONENT(Animator, "Animation")


void Animator::Initialize()
{
	// 初期化処理をここに実装する
	// 例: コントローラーの初期化、アニメーションの準備など
	controller = std::make_unique<AnimatorController>();
}

void Animator::Update(float deltaTime)
{
	// コントローラーが存在する場合、アニメーションの更新処理を行う
	if (controller)
	{
		// ここでアニメーションの更新ロジックを実装する
		// 例: 現在のアニメーションの時間を進める、トランジションの処理など
		// controller->Update(deltaTime); // 仮の関数呼び出し

		if (targetModelRendererId.IsValid())
		{
			if (auto renderer = GetScene()->FindComponentById<GltfModelRenderer>(targetModelRendererId))
			{
                auto& nodes = renderer->m_asset->nodes;
                auto& animations = renderer->m_asset->animations;

                if (animations.size() <= renderer->animationIndex) return;//アニメーションが無かったらスルー

                if (!renderer->IsAnimationEnable()) return;//アニメーションが無効ならスルー

				// アニメーション名がまだ設定されていない場合、コントローラーにアニメーション名を設定する
                if (controller->animationNames.empty())
                {
                    // コントローラーにアニメーション名を設定する
                    for (const auto& animation : animations)
                    {
                        controller->animationNames.push_back(animation.name);
                    }
                }

                // ノードが存在している場合のみ処理
                if (nodes.size() > 0)
                {
                    // アニメーションの更新
                    renderer->Animate(renderer->animationIndex, renderer->time += (deltaTime * renderer->timeRate), nodes);

                    // アニメーションの時間を取得
                    float animationDuration = animations.at(renderer->animationIndex).duration;

                    // アニメーションイベントの処理
                    for (AnimationEvent::Event& event : renderer->animationEvent.events)
                    {
                        // イベントがまだ発火していない場合
                        if (!event.isCalled)
                        {
                            // イベント発火時間に達したら
                            if (renderer->time >= event.time)
                            {
                                // イベントコールバック関数を呼び出す
                                if (event.func)
                                {
                                    event.func();
                                }
                                // イベント発火済みにする
                                event.isCalled = true;
                            }
                        }
                    }

                    //アニメーションが最後に到達したら
                    if (animationDuration < renderer->time)
                    {
                        if (renderer->loop) {
                            renderer->time = 0;
                            renderer->isBlendStart = false;
                        }
                        else {
                            renderer->isAnimationCompleted = true;
                        }
                    }
                }


				//renderer->SetAnimation(controller->GetCurrentAnimationIndex(), controller->IsBlendEnabled(), controller->GetAnimationEvent());
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
        for (int i = 0; i < controller->animationNames.size(); ++i)
        {
            if (ImGui::Button(controller->animationNames[i].c_str()))
            {
                // アニメーションを切り替える処理
                if (auto renderer = GetScene()->FindComponentById<GltfModelRenderer>(targetModelRendererId))
                {
                    renderer->SetAnimation(i);
                }
			}
		}
	}
}
#endif // USE_IMGUI
