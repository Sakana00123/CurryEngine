#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Rendering/Material.h"
#include "Engine/Core/Math/BoundingBox.h"

#define USE_MATERIAL

class Renderer : public Component
{
	C_REFLECT(Renderer)
public:
	Renderer() = default;
	virtual ~Renderer() override = default;
	
	// 描画順序を設定
	//void SetRenderOrder(int order) { renderOrder = order; }
	//int GetRenderOrder() const { return renderOrder; }

	Math::BoundingBox GetAABB() const { return boundingBox; }

	/**
	 * @brief AABBを計算して返す
	 * @return AABB (ワールド座標系)
	 * @details 継承先でオーバーライドして計算処理を実装すること
	 */
	virtual Math::BoundingBox CalculateAABB() const { return boundingBox; }

#ifdef USE_IMGUI
	// デバッグ GUI の描画
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI


	// シリアライズ
	json Serialize() const override;

	// デシリアライズ
	void Deserialize(const json& j) override;

public:
	/* 
	 * @brief マテリアル
	 * @details マテリアルが設定されていない場合は描画されません。
	 * @note 継承先で `Render()` 内でマテリアルの `Apply()` を呼び出すこと。
	 */
	std::shared_ptr<Material> material; // マテリアル
	Math::BoundingBox boundingBox; // AABB（ワールド座標系）
private:
	//int renderOrder = 0; // 描画順序

};