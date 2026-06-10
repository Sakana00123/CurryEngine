#pragma once
#include "Collider.h"

class BoxCollider : public Collider
{
	C_REFLECT(BoxCollider)
public:
	/** @brief 初期化処理（デバッグプリミティブ準備など）。*/
	void Initialize() override;
	/** @brief ブロードキャスト登録（空間構造等への登録）。*/
	void Register() override;

	/** @brief コライダーの形状を中心とサイズでフィットさせる。*/
	void FitToBoundingBox(const Vector3& center, const Vector3& size) override;

	/** @brief 物理エンジンとの状態同期。*/
	void SyncWithPhysics() override;
	/** @brief デバッグ描画。*/
	void Render(RenderContext* rtx) override;

#ifdef USE_IMGUI
	/** @brief プロパティ描画。*/
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI


	/** @brief シリアライズ。*/
	json Serialize() const override;

	/** @brief デシリアライズ。*/
	void Deserialize(const json& j) override;

public:
	/** @brief ローカルオフセット。*/
	C_PROPERTY()
	Vector3 center{ 0,0,0 };
	/** @brief ボックスの辺長。*/
	C_PROPERTY()
	Vector3 size{ 1,1,1 };

};