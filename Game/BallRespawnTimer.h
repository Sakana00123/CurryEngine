#pragma once
#include "Engine/Core/Component.h"
#include "Ball.h"
#include "Engine/Core/GameObject.h"


class Ball;

class BallRespawnTimer : public Component
{
    C_REFLECT(BallRespawnTimer)
public:
    BallRespawnTimer() = default;
    ~BallRespawnTimer() = default;

    void Start() override;
    void Update(float deltaTime) override;

    void StartTimer(Ball* ball, float delay);

private:
    Ball* targetBall = nullptr;
    float timer = 0.0f;
    bool isRunning = false;
};