#pragma once
#include "Renderer.h"
#include "ModelRenderer.h"

/** @brief MeshRenderer を表すクラスです。 */
class MeshRenderer : public Renderer
{
	C_REFLECT(MeshRenderer)
public:
	/**
	 * @brief MeshRenderer を構築します。
	 */
	MeshRenderer() = default;
	/**
	 * @brief MeshRenderer を破棄します。
	 */
	~MeshRenderer() override = default;
	// 初期化処理
	/**
	 * @brief 初期化します。
	 */
	void Initialize() override;
	// 描画処理
	/**
	 * @brief 描画処理を行います。
	 */
	void Render(RenderContext* rtx) override;
	// AABB計算
	/**
	 * @brief CalculateAABB の処理を行います。
	 */
	Math::BoundingBox CalculateAABB() const override;
#ifdef USE_IMGUI
	// デバッグ GUI の描画
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI
	// シリアライズ
	/**
	 * @brief Serialize の処理を行います。
	 */
	json Serialize() const override;
	// デシリアライズ
	/**
	 * @brief Deserialize の処理を行います。
	 */
	void Deserialize(const json& j) override;
public:
	C_PROPERTY(CurryEngine::PropertyAttributes::DialogFilter("Mesh Files (*.fbx;*.obj;*.gltf;*.glb)|*.fbx;*.obj;*.gltf;*.glb|All Files (*.*)|*.*|"), CurryEngine::PropertyAttributes::CustomDrawer("String_AssetReference"), CurryEngine::PropertyAttributes::NonSerialized)
	std::string meshAssetPath; // メッシュアセットのパス

	std::shared_ptr<ModelRenderer> modelRenderer; // モデルレンダラー
};
