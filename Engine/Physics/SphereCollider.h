#pragma once
#include "Collider.h"

class SphereCollider : public Collider
{
	C_REFLECT(SphereCollider)
public:
	/** @brief 初期化処理（デバッグプリミティブ準備など）。*/
	void Initialize() override;
	/** @brief ブロードキャスト登録（空間構造等への登録）。*/
	void Register() override;

	/** @brief コライダーの形状を中心とサイズでフィットさせる。*/
	void FitToBoundingBox(const Vector3& center, const Vector3& size) override;

	/** @brief 物理エンジンとの状態同期。*/
	void SyncWithPhysics() override;

#ifdef USE_IMGUI
	/** @brief プロパティ描画。*/
	//void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI


	/** @brief シリアライズ。*/
	json Serialize() const override;

	/** @brief デシリアライズ。*/
	void Deserialize(const json& j) override;

public:
	/** @brief ローカルオフセット。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::Getter("GetCenter"), CurryEngine::PropertyAttributes::Setter("SetCenter"))
	Vector3 center{ 0,0,0 };
	/** @brief 球の半径。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::Getter("GetRadius"), CurryEngine::PropertyAttributes::Setter("SetRadius"))
	float radius{ 1.0f };

	/** @brief 球の中心位置を取得します。*/
	C_FUNCTION()
	Vector3 GetCenter() const;

	/** @brief 球の中心位置を設定します。*/
	C_FUNCTION()
	void SetCenter(const Vector3& newCenter);

	/** @brief 球の半径を取得します。*/
	C_FUNCTION()
	float GetRadius() const;

	/** @brief 球の半径を設定します。*/
	C_FUNCTION()
	void SetRadius(float newRadius);
};
