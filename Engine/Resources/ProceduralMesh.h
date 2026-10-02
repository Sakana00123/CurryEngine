#pragma once
#include <memory>
#include "Mesh.h"

namespace ProceduralMesh
{
	/** @brief 指定サイズのクワッドメッシュを生成します。 */
	std::shared_ptr<Mesh> CreateQuad(float width, float height);
	/** @brief 指定サイズと分割数の平面メッシュを生成します。 */
	std::shared_ptr<Mesh> CreatePlane(float width, float height, uint32_t widthSegments = 1, uint32_t heightSegments = 1);
	/** @brief 指定サイズのキューブメッシュを生成します。 */
	std::shared_ptr<Mesh> CreateCube(float width, float height, float depth);
	/** @brief 指定半径と分割数の球メッシュを生成します。 */
	std::shared_ptr<Mesh> CreateSphere(float radius, uint32_t longitudeSegments = 16, uint32_t latitudeSegments = 16);
	/** @brief 指定半径、高さ、分割数の円柱メッシュを生成します。 */
	std::shared_ptr<Mesh> CreateCylinder(float radius, float height, uint32_t radialSegments = 16);
	/** @brief 指定半径、高さ、分割数のカプセルメッシュを生成します。 */
	std::shared_ptr<Mesh> CreateCapsule(float radius, float height, uint32_t radialSegments = 16, uint32_t heightSegments = 8);
}
