#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Physics/CollisionEvent.h"
class Gate;

class Ball : public Component
{
	C_REFLECT(Ball)

public:
	/** @brief ボールの価値 **/
	C_PROPERTY()
		int value = 1;

	/** @brief ボールの耐久値 **/
	C_PROPERTY()
		int durability = 100;

	bool isPlaySound = false; // 音を再生するかどうかのフラグ

	static constexpr float minScaleFactor = 0.5f; // ボールの最小スケール
	static constexpr float maxScaleFactor = 1.3f; // ボールの最大スケール
	static constexpr float initialScale = 0.023f; // ボールの初期スケール
	static constexpr float initialMass = 0.02f; // ボールの初期質量
	static constexpr float minScale = initialScale * minScaleFactor; // ボールの最小スケール
	static constexpr float maxScale = initialScale * maxScaleFactor; // ボールの最大スケール
public:
	Ball() = default;
	virtual ~Ball() override = default;

	void Start() override;
	void Update(float deltaTime) override;

	//void DrawProperty() override; // エディタでプロパティを描画するためのオーバーライド関数

	/** @brief 現在の価値を取得 **/
	int GetValue() const { return value; }

	/** @brief 価値を倍算 **/
	void MultiplicationValue(int amount) { value *= amount; }

	/* @brief 価値を加算 **/
	void IncreaseValue(int amount);

	/** @brief 価値をリセット **/
	void ResetValue() { value = 1; }

	void SetValue(int newValue) { value = newValue; }

	/** @brief 初期位置に戻す **/
	void ResetToInitialPosition();

	/** @brief ボールのスケールを変更（初期スケールに基づいて） **/
	void ScaleBall(float scaleFactor);

	/** @brief ボールのスケールを変更（現在のスケールに基づいて） **/
	void ScaleBallRelative(float scaleFactor);

	/** @brief ボールの相対的なスケールを取得（初期スケールに対する比率） **/
	float GetRelativeScale() const;

	/** @brief ボールの物理マテリアルデータを更新 **/
	void UpdateBallPhysicsMaterialData();
	
	/** @brief ボールがピンに当たった時のイベント **/
	void OnCollisionEnter(const CollisionInfo& collisionInfo);
	void OnCollisionStay(const CollisionInfo& collisionInfo);
	void OnCollisionExit(const CollisionInfo& collisionInfo);

	//耐久値をリセットする関数
	void ResetDurability() { durability = 20; }

	//壁に当たったら価値を減らす状態にする
	void SetWallCurse(bool isCurse) { isCurseByWall = isCurse; }

	//壁に当たったら価値を減らす状態かどうかを取得する関数
	bool IsWallCursed() const { return isCurseByWall; }

	void OnDestroy() override; // ボールが破壊されたときの処理

private:
	bool isCurseByWall = false;

	bool isStacking = false; // ボールが静止して詰んでる状態かどうかのフラグ
	float stackingTimer = 0.0f; // ボールが静止して詰んでる状態のタイマー
	constexpr static float stackingThreshold = 3.0f; // ボールが静止して詰んでる状態とみなすための時間（秒）

	Vector3 previousPosition; // 前フレームの位置を保存する変数

	Gate* gate = nullptr; // ゲートの参照
};
