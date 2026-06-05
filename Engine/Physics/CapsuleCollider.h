#pragma once
#include "Collider.h"

class CapsuleCollider : public Collider
{
	C_REFLECT(CapsuleCollider)
public:
	/** @brief 初期化処理（デバッグプリミティブ準備など）。*/
	void Initialize() override;
	/** @brief 空間登録。*/
	void Register() override;

	/** @brief コライダーの形状を中心とサイズでフィットさせる。*/
	void FitToBoundingBox(const Vector3& center, const Vector3& size) override;

	/** @brief 物理エンジンとの状態同期。*/
	void SyncWithPhysics() override;
	/** @brief ワイヤーフレーム描画処理 */
	void Render(RenderContext* rtx) override;

	/** @brief プロパティ描画。*/
	void DrawProperty() override;


	/** @brief シリアライズ。*/
	json Serialize() const override;
	/** @brief デシリアライズ。*/
	void Deserialize(const json& j) override;


public:
	/** @brief ローカルオフセット。*/
	C_PROPERTY()
	Vector3 center{ 0,0,0 };
	/** @brief カプセルの半径。*/
	C_PROPERTY()
	float radius{ 1.0f };
	/** @brief カプセルの高さ（中心から中心まで）。*/
	C_PROPERTY()
	float height{ 2.0f };

	std::unique_ptr<GeometricPrimitive> top;
	std::unique_ptr<GeometricPrimitive> bottom;
};