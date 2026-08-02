#include "pch.h"
#include "Gate.h"
#include "Ball.h"
#include "Engine/Physics/BoxCollider.h"
#include "Engine/Scenes/Scene.h"
#include "BallObserver.h"
#include "Gadget.h"
#include "SpringPlunger.h"
#include "TutorialSystem.h"

REGISTER_COMPONENT(Gate, "UserScripts")

void Gate::Start()
{
    startAngle = GetTransform()->GetEulerAngles().y; // 初期角度を保存

    GameObject* gateTrigger = GetScene()->GetObjectManager()->Find("GateTrigger");
    if (gateTrigger)
    {
        if (BoxCollider* collider = gateTrigger->GetComponent<BoxCollider>())
        {
            collider->AddOnTriggerEnterEvent([this](const TriggerInfo& triggerInfo) {
                if (triggerInfo.other->GetComponent<Ball>())
                {
					BallObserver* observer = GetScene()->FindComponentById<BallObserver>(ballObserverReference);
					if (observer && observer->GetBallCount() == 0)
                    {
                        CloseGate();
                    }
                }
                });
        }
    }
}

void Gate::Update(float deltaTime)
{
    switch (gateDirection)
    {
    case 1: // 閉じる方向
        currentRate += gateForce * deltaTime / closeAngle;
        if (currentRate >= 1.0f)
        {
            currentRate = 1.0f;
            gateDirection = 0;
        }
        break;
    case -1: // 開く方向
        currentRate -= gateForce * deltaTime / closeAngle;
        if (currentRate <= 0.0f)
        {
            currentRate = 0.0f;
            gateDirection = 0;
        }
        break;
    default:
        return;
    }

    float targetAngle = startAngle + closeAngle * currentRate;
    Vector3 eulerAngles = GetTransform()->GetEulerAngles();
    eulerAngles.y = targetAngle;
    GetTransform()->SetRotation(eulerAngles);


    if (BallObserver* observer = GetScene()->FindComponentById<BallObserver>(ballObserverReference))
    {
        if (observer->GetBallCount() > 0 && isClosed)
        {
            OpenGate();
        }
	}
}

void Gate::CloseGate()
{
    if (isClosed) return;
    isClosed = true;
    gateDirection = 1;

    if (SpringPlunger* plunger = GetScene()->FindComponentById<SpringPlunger>(springPlungerReference))
    {
		plunger->canInteract = false;
	}

	// チュートリアルのショットステップを進める
	TutorialSystem* tutorial = TutorialSystem::GetInstance();
	if (tutorial->GetCurrentTutorialStep() == TutorialSystem::TutorialStep::Shot)
    {
		tutorial->AdvanceTutorialStep();
    }
}

void Gate::OpenGate()
{
    if (!isClosed) return;
    isClosed = false;
    gateDirection = -1;

    if (SpringPlunger* plunger = GetScene()->FindComponentById<SpringPlunger>(springPlungerReference))
    {
        plunger->canInteract = true;
    }
}