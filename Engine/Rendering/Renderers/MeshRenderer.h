#pragma once
#include "Renderer.h"
#include "ModelRenderer.h"

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
public:
	C_PROPERTY(CurryEngine::PropertyAttributes::DialogFilter("Mesh Files (*.fbx;*.obj;*.gltf;*.glb)|*.fbx;*.obj;*.gltf;*.glb|All Files (*.*)|*.*|"), CurryEngine::PropertyAttributes::CustomDrawer("String_AssetReference"), CurryEngine::PropertyAttributes::NonSerialized)
	std::string meshAssetPath; // メッシュアセットのパス

	std::shared_ptr<ModelRenderer> modelRenderer; // モデルレンダラー
};
