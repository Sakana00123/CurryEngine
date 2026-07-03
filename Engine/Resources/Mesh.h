#pragma once
#include "Resource.h"
#include "Engine/Core/Math/BoundingBox.h"
#include <vector>
#include <d3d11.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

class Mesh : public Resource
{
public:
	struct SubMesh {
		uint32_t indexCount = 0;
		uint32_t indexOffset = 0;
		int materialIndex = -1; // マテリアルインデックス（-1はマテリアルなし）
	};

	// GPUリソース
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	uint32_t vertexStride = 0; // 頂点1つあたりのバイト数
	DXGI_FORMAT indexFormat = DXGI_FORMAT_R32_UINT;

	std::vector<SubMesh> subMeshes;
	Math::BoundingBox localBounds; // ローカル空間でのバウンディングボックス

	// Resourceインターフェースの実装
	bool LoadFromFile(const std::string& path) override;
};