#pragma once
#include "Renderer.h"

C_ENUM()
/** @brief TrailRenderMode を表す列挙型です。 */
enum class TrailRenderMode
{
	Billboard, // 常にカメラに面するビルボード
	Stretched, // 移動方向に沿って伸びるストレッチ
};

/** @brief TrailRenderer を表すクラスです。 */
class TrailRenderer : public Renderer
{
	C_REFLECT(TrailRenderer)
public:
	/**
	 * @brief TrailRenderer を構築します。
	 */
	TrailRenderer() = default;
	/**
	 * @brief TrailRenderer を破棄します。
	 */
	virtual ~TrailRenderer() = default;

	/**
	 * @brief 初期化します。
	 */
	void Initialize() override;

	/**
	 * @brief 状態を更新します。
	 */
	void Update(float deltaTime) override;

	/**
	 * @brief 描画処理を行います。
	 */
	void Render(RenderContext* context) override;
	/**
	 * @brief 描画処理を行います。
	 */
	//virtual void RenderShadowMap(RenderContext* context) override;
	/**
	 * @brief 描画処理を行います。
	 */
	//virtual void RenderDepth(RenderContext* context) override;

private:
	// トレイルの長さ
	C_PROPERTY()
	float trailLength = 5.0f;

	// トレイルの幅
	C_PROPERTY()
	float trailWidth = 0.01f;

	// トレイルの色
	C_PROPERTY()
	Color trailColor = Color::White;

	// トレイルが消えるまでの時間
	C_PROPERTY()
	float fadeDuration = 1.0f;

	// トレイルセグメントを追加するための移動距離の閾値
	C_PROPERTY()
	float trailThreshold = 0.002f;

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Transform"), CurryEngine::PropertyAttributes::Tooltip("Comming Soon"))
	ObjectId targetTransformId; // トレイルを追従させる対象の Transform の ObjectId

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"), CurryEngine::PropertyAttributes::Tooltip("Comming Soon"))
	ObjectId targetObjectId; // トレイルを追従させる対象の GameObject の ObjectId

	C_PROPERTY(CurryEngine::PropertyAttributes::CustomDrawer("String_AssetReference"), CurryEngine::PropertyAttributes::DialogFilter("Texture Files (*.png;*.jpg;*.dds)|*.png;*.jpg;*.dds|All Files (*.*)|*.*|"))
	std::string texturePath; // トレイルに使用するテクスチャのファイルパス

	C_PROPERTY(CurryEngine::PropertyAttributes::Tooltip("Comming Soon"))
	int maxSegments = 50; // トレイルの最大セグメント数

	C_PROPERTY(CurryEngine::PropertyAttributes::CustomDrawer("Enum"), CurryEngine::PropertyAttributes::Enum("TrailRenderMode"))
	int renderMode = 0; // トレイルの描画モード

	/** @brief TrailSegment を表す構造体です。 */
	struct TrailSegment
	{
		Vector3 position; // セグメントの位置
		float age;       // セグメントの経過時間
		Color color;       // セグメントの色
	};
	std::vector<TrailSegment> segments; // トレイルのセグメントリスト

	Vector3 lastPosition; // 前フレームの位置

};