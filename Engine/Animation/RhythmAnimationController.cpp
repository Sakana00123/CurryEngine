#include "pch.h"
#include "RhythmAnimationController.h"
#include "Engine/Audio/BeatManager.h"
#include "Engine/Core/GameObject.h"

REGISTER_COMPONENT(RhythmAnimationController, "Animation")

void RhythmAnimationController::Start()
{
	// ステートマシンを初期化
	if (!stateMachine.currentStateName.empty())
	{
		stateMachine.Reset(stateMachine.currentStateName);
	}
	else if (!stateMachine.states.empty())
	{
		stateMachine.Reset(stateMachine.states[0].name);
	}
	// 初期化
	elapsedTime = 0.0f;
	currentBeat = 0;
	previousBeat = -1;

	// GltfModelRenderer コンポーネントを取得
	modelRenderer = GetOwner()->GetComponent<GltfModelRenderer>();
}

void RhythmAnimationController::Update(float deltaTime)
{
	// ビートの進行時間を更新
	elapsedTime += deltaTime;
	// 1ビートあたりの秒数を計算
	secondsPerBeat = 60.0f / bpm;
	// 現在のビートを計算
	currentBeat = static_cast<int>(elapsedTime / secondsPerBeat);

	{
		stateMachine.Update();
		previousBeat = currentBeat;
		// 現在のステートに基づいてアニメーションを設定
		RhythmAnimState* currentState = stateMachine.GetState(stateMachine.currentStateName);
		if (currentState && modelRenderer && currentState->clip.sourceAnimation)
		{
			modelRenderer->SetAnimation(currentState->clip.sourceAnimation->name, currentState->loop);
			modelRenderer->SetAnimationTimeRate(animationSpeed);
			modelRenderer->animationBlendTime = animationBlendTime;
			modelRenderer->SetLoop(currentState->loop);
		}
	}
	// アニメーションの進行を更新
	if (modelRenderer)
	{
		// アニメーションを手動制御に設定
		modelRenderer->animationEnable = false;

		if (RhythmAnimState* currentState = stateMachine.GetState(stateMachine.currentStateName))
		{
			// アニメーションの進行時間を計算
			float animationDeltaTime = deltaTime * animationSpeed;

			if (currentState->syncToBeat)
			{
				// ビートに同期させる場合、ビートごとの時間でアニメーションを進行
				animationDeltaTime = secondsPerBeat;
			}
			int currentAnimationIndex = modelRenderer->GetAnimationIndex(currentState->clip.sourceAnimation->name);
			float animationDuration = modelRenderer->GetAnimationDuration(currentAnimationIndex);
			float currentTime = fmodf(modelRenderer->time + animationDeltaTime, animationDuration); // ループさせる場合
			if (!currentState->loop && (modelRenderer->time + animationDeltaTime) >= animationDuration)
			{
				currentTime = animationDuration; // ループしない場合、終了時間に固定
			}
			// アニメーションを進行
			modelRenderer->Animate(currentAnimationIndex, currentTime, modelRenderer->GetModelAsset()->nodes);
		}
	}
}