#pragma once
#include "Renderer.h"
#include "Engine/Resources/Mesh.h"

class MeshRenderer : public Renderer
{
	C_REFLECT(MeshRenderer)
public:
	MeshRenderer() = default;
	~MeshRenderer() override = default;
	// 初期化処理
	void Initialize() override;
	// 描画処理
	void Render(RenderContext* rtx) override;
	// AABB計算
	Math::BoundingBox CalculateAABB() const override;
#ifdef USE_IMGUI
	// デバッグ GUI の描画
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI
	// シリアライズ
	json Serialize() const override;
	// デシリアライズ
	void Deserialize(const json& j) override;

	// プリミティブメッシュの設定(テスト用。将来的にはMeshFilterなどで管理することも検討)
	void SetPrimitiveMesh(int type);
public:
	//C_PROPERTY()
	//std::string meshAssetPath; // メッシュアセットのパス

	int primitiveType = 0; // 描画プリミティブタイプ(0: キューブ、1: 球、2: 平面、3: カプセル、4: 円柱)

	std::shared_ptr<Mesh> mesh; // メッシュデータへの参照
};
