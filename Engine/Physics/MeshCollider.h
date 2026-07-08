#pragma once
#include "Collider.h"

class MeshCollider : public Collider
{
	C_REFLECT(MeshCollider)
public:
	/** @brief 初期化処理（デバッグプリミティブ準備など）。*/
	void Initialize() override;
	/** @brief ブロードキャスト登録（空間構造等への登録）。*/
	void Register() override;
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
	C_PROPERTY(CurryEngine::PropertyAttributes::Getter("IsConvex"), CurryEngine::PropertyAttributes::Setter("SetConvex"))
	bool convex = false; // 凸メッシュかどうかのフラグ

	C_FUNCTION()
	bool IsConvex() const;

	C_FUNCTION()
	void SetConvex(bool isConvex);

	/** @brief メッシュアセットのパス。*/
	//C_PROPERTY()
	//std::string meshAssetPath;


};
