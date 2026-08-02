#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Physics/Collider.h"

class Gate : public Component
{
    C_REFLECT(Gate)
public:
    Gate() = default;
    ~Gate() = default;

    void Start() override;
    void Update(float deltaTime) override;

    void CloseGate();
    void OpenGate();

	// ゲートが閉じているかを返す関数
	bool IsClosed() const { return isClosed; }

private:
    C_PROPERTY()
        float closeAngle = 50.0f; // 閉じたときの角度

    C_PROPERTY()
        float gateForce = 180.0f; // フリッパーのflipForceに相当

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("BallObserver"))
	ObjectId ballObserverReference; // BallObserver コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("SpringPlunger"))
	ObjectId springPlungerReference; // SpringPlunger コンポーネントへの参照


    float startAngle = 0.0f;
    float currentRate = 0.0f; // 0.0f(開) 〜 1.0f(閉)
    int gateDirection = 0;    // 1:閉じる -1:開く 0:停止
    bool isClosed = false;
};