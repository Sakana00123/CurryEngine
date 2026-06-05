#pragma once
#include "Renderer.h"

class TrailRenderer : public Renderer
{
	C_REFLECT(TrailRenderer)
public:
	TrailRenderer() = default;
	virtual ~TrailRenderer() = default;

	void Initialize() override;

	void Update(float deltaTime) override;

	void Render(RenderContext* context) override;
	//virtual void RenderShadowMap(RenderContext* context) override;
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

	struct TrailSegment
	{
		Vector3 position; // セグメントの位置
		float age;       // セグメントの経過時間
		Color color;       // セグメントの色
	};
	std::vector<TrailSegment> segments; // トレイルのセグメントリスト

	Vector3 lastPosition; // 前フレームの位置

};