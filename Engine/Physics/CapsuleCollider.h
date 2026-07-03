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
	/** @brief カプセルの半径。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::Getter("GetRadius"), CurryEngine::PropertyAttributes::Setter("SetRadius"))
	float radius{ 1.0f };
	/** @brief カプセルの高さ（中心から中心まで）。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::Getter("GetHeight"), CurryEngine::PropertyAttributes::Setter("SetHeight"))
	float height{ 2.0f };

	/** @brief 中心位置を返します。*/
	C_FUNCTION()
	Vector3 GetCenter() const;
	/** @brief 中心位置を設定します。*/
	C_FUNCTION()
	void SetCenter(const Vector3& newCenter);
	/** @brief 半径を返します。*/
	C_FUNCTION()
	float GetRadius() const;
	/** @brief 半径を設定します。*/
	C_FUNCTION()
	void SetRadius(float newRadius);
	/** @brief 高さを返します。*/
	C_FUNCTION()
	float GetHeight() const;
	/** @brief 高さを設定します。*/
	C_FUNCTION()
	void SetHeight(float newHeight);



	std::unique_ptr<GeometricPrimitive> top;
	std::unique_ptr<GeometricPrimitive> bottom;
};