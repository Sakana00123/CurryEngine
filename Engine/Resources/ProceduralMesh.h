#pragma once
#include <memory>
#include "Mesh.h"

namespace ProceduralMesh
{
	// クワッドメッシュを生成
	std::shared_ptr<Mesh> CreateQuad(float width, float height);
	// 平面メッシュを生成
	std::shared_ptr<Mesh> CreatePlane(float width, float height, uint32_t widthSegments = 1, uint32_t heightSegments = 1);
	// キューブメッシュを生成
	std::shared_ptr<Mesh> CreateCube(float width, float height, float depth);
	// 球メッシュを生成
	std::shared_ptr<Mesh> CreateSphere(float radius, uint32_t longitudeSegments = 16, uint32_t latitudeSegments = 16);
	// 円柱メッシュを生成
	std::shared_ptr<Mesh> CreateCylinder(float radius, float height, uint32_t radialSegments = 16);
	// カプセルメッシュを生成
	std::shared_ptr<Mesh> CreateCapsule(float radius, float height, uint32_t radialSegments = 16, uint32_t heightSegments = 8);
}