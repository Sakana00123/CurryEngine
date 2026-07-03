#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Rendering/Renderers/GltfModelRenderer.h"
#include "Engine/Animation/RhythmStateMachine.h"

class RhythmAnimationController : public Component
{
	C_REFLECT(RhythmAnimationController)
public:
	/** @brief GltfModelRenderer への参照。*/
	GltfModelRenderer* modelRenderer = nullptr;

	/** @brief リズムステートマシン。*/
	RhythmStateMachine stateMachine;

	/** @brief 現在のビート。*/
	int currentBeat = 0;
	/** @brief 前回のビート。*/
	int previousBeat = -1;
	/** @brief ビートの進行速度（BPM）。*/
	float bpm = 120.0f;
	/** @brief 経過時間（秒）。*/
	float elapsedTime = 0.0f;
	/** @brief 現在の時間（秒）。*/
	float currentTime = 0.0f;
	/** @brief 1ビートあたりの秒数。*/
	float secondsPerBeat = 0.5f;
	/** @brief アニメーションの再生速度。*/
	float animationSpeed = 1.0f;
	/** @brief アニメーションのブレンド時間（秒）。*/
	float animationBlendTime = 0.2f;
	
public:

	// 開始処理
	void Start() override;

	// 更新処理
	void Update(float deltaTime) override;
};