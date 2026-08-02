#include "pch.h"
#include "BallRespawnTimer.h"
#include "PhaseManager.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"
#include "PreserveValue.h"
#include "AttachManager.h"
#include "RoundManager.h"

REGISTER_COMPONENT(BallRespawnTimer, "UserScripts")

void BallRespawnTimer::Start() {}

void BallRespawnTimer::Update(float deltaTime)
{
    //if (!isRunning || !targetBall) return;
    //timer -= deltaTime;
    //if (timer <= 0.0f)
    //{
    //    isRunning = false;
    //    targetBall->ResetDurability();

    //    // ボールカウントを減らした後に0以下か判定
    //    targetBall->DecreaseBallCount(1);

    //    //if (targetBall->GetBallCount() < 0)
    //    if (0)
    //    {
    //        // MagnificationZoneと同じショップ移行処理
    //        PreserveValue* preserveValue = GetScene()->objectManager->Find("PreserveValue")->GetComponent<PreserveValue>();
    //        PhaseManager* phaseManager = GetScene()->objectManager->Find("PhaseManager")->GetComponent<PhaseManager>();
    //        AttachManager* attachManager = GetScene()->objectManager->Find("AttachManager")->GetComponent<AttachManager>();
    //        RoundManager* roundManager = GetScene()->objectManager->Find("RoundManager")->GetComponent<RoundManager>();

    //        if (preserveValue && preserveValue->IsTargetValueReached() &&
    //            roundManager && roundManager->GetCurrentRound() < roundManager->GetMaxRounds())
    //        {
    //            // 目標達成 → ショップへ
    //            if (attachManager) attachManager->StartWaitingForAttachment(targetBall);
    //            if (phaseManager) phaseManager->SetPhase(PhaseManager::Shop);
    //            targetBall->ResetToInitialPosition();
				//targetBall->GetOwner()->SetActive(true);
    //            Console::Log("Durability depleted. Moved to Shop.");
    //        }
    //        else
    //        {
    //            // 目標未達成 → ゲームオーバー
    //            targetBall->GetOwner()->SetActive(false);
    //            if (preserveValue) preserveValue->ResetPreservedValue();
    //            if (roundManager) roundManager->ResetRounds();
    //            Scene* currentScene = GetScene();
    //            if (currentScene) SceneManager::LoadScene(currentScene->name);
    //            Console::Log("Game Over! No more balls left.");
    //        }
    //    }
    //    else
    //    {
    //        // ボールがまだ残っている → 通常リスポーン
    //        targetBall->GetOwner()->SetActive(true);
    //        targetBall->ResetToInitialPosition();
    //        Console::Log("Ball respawned.");
    //    }

    //    targetBall = nullptr;
    //}
}

void BallRespawnTimer::StartTimer(Ball* ball, float delay)
{
    //targetBall = ball;
    //timer = delay;
    //isRunning = true;
}