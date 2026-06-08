#include "pch.h"
#include "TrailRenderer.h"
#include "DebugRenderer.h"
#include "Engine/Core/Transform.h"

REGISTER_COMPONENT(TrailRenderer, "Renderer");


void TrailRenderer::Initialize()
{
	// トレイルレンダラーの初期化ロジックをここに実装
	// 例: 頂点バッファの作成、シェーダーのロードなど

}

void TrailRenderer::Update(float deltaTime)
{
	// トレイルの更新ロジックをここに実装
	// 例: 頂点の位置や色の更新、寿命の管理など

	for (auto& segment : segments)
	{
		segment.age += deltaTime;
		segment.color.a = std::max(0.0f, 1.0f - (segment.age / fadeDuration)); // 経過時間に応じてアルファ値を減少
	}
	for (auto it = segments.begin(); it != segments.end();)
	{
		if (it->age >= fadeDuration)
		{
			it = segments.erase(it); // 寿命が尽きたセグメントを削除
		}
		else
		{
			++it;
		}
	}

	if (segments.empty() || (GetTransform()->GetWorldPosition() - lastPosition).Length() > trailThreshold) // 一定距離以上移動したら新しいセグメントを追加
	{
		TrailSegment newSegment;
		newSegment.position = GetTransform()->GetWorldPosition();
		newSegment.age = 0.0f;
		newSegment.color = trailColor;
		segments.push_back(newSegment);
	}


	lastPosition = GetTransform()->GetWorldPosition();
}

void TrailRenderer::Render(RenderContext* context)
{
	// トレイルのレンダリングロジックをここに実装
	// 例: 頂点バッファの設定、シェーダーの適用、描画コールなど
	int numSegments = static_cast<int>(segments.size());
	for (int i = 0; i < numSegments - 1; ++i)
	{
		const auto& segmentA = segments[i];
		const auto& segmentB = segments[i + 1];
		Vector3 direction = segmentB.position - segmentA.position;
		Vector3 perpendicular = Vector3::Cross(direction.Normalize(), Vector3::Up) * (trailWidth * 0.5f);
		// 四角形の頂点を計算
		Vector3 vertices[4] =
		{
			segmentA.position + perpendicular,
			segmentA.position - perpendicular,
			segmentB.position + perpendicular,
			segmentB.position - perpendicular 
		};
		Color colors[4] =
		{
			segmentA.color,
			segmentA.color,
			segmentB.color,
			segmentB.color
		};

		// 頂点を追加（2つの三角形で四角形を描画）
		DebugRenderer::AddVertex(vertices[0], colors[0]); // A+
		DebugRenderer::AddVertex(vertices[1], colors[1]); // A-
		DebugRenderer::AddVertex(vertices[2], colors[2]); // B+

		DebugRenderer::AddVertex(vertices[1], colors[1]); // A-
		DebugRenderer::AddVertex(vertices[3], colors[3]); // B-
		DebugRenderer::AddVertex(vertices[2], colors[2]); // B+
	}
	DebugRenderer::DrawAll(context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}