#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Physics/CollisionEvent.h"
#include "Ball.h"

class MagnificationZone : public Component
{
	C_REFLECT(MagnificationZone)

public:
	/** @brief 倍率 **/
	C_PROPERTY()
		float magnification = 2.0f;

	/** @brief 最大衝突回数（これを超えると効果が発動しなくなる） **/
	C_PROPERTY()
		int maxCollisions = 1;

	///** @brief ボールがゾーンから出た後、初期位置に戻るまでの時間（秒） **/
	//C_PROPERTY()
	//	float resetDelay = 2.0f;

private:
	/** @brief 衝突回数のカウンタ **/
	C_PROPERTY()
	int collisionCount = 0;

	/** @brief 現在のボール **/
	Ball* currentBall = nullptr;

	/** @brief ボールを初期位置に戻すまでの経過時間 **/
	float resetTimer = 0.0f;

	/** @brief リセット待機中か **/
	bool isWaitingForReset = false;

	// ** @brief ボールの数を減らす変数 **/
	C_PROPERTY()
	int ballCountToDecrease = 1;

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
	ObjectId multiplicationTextRef; // 倍率を表示する Text コンポーネントへの参照


public:
	MagnificationZone() = default;
	virtual ~MagnificationZone() override = default;

	void Start() override;
	void Update(float deltaTime) override;
	void OnDestroy() override;

	/// <summary>
	/// トリガーに入った時のイベント
	/// </summary>
	void OnTriggerEnter(const TriggerInfo& triggerInfo);

	//倍率をセットする関数
	void SetMagnification(float newMagnification, bool isTopRate = false);

	//倍率を取得する関数
	float GetMagnification() const { return magnification; }

};