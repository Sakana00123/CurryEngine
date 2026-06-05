#include "pch.h"
#include "ProceduralMesh.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

namespace ProceduralMesh
{
	static void CreateComBuffers(ID3D11Device* device, const void* vertexData, size_t vertexDataSize, const void* indexData, size_t indexDataSize, std::shared_ptr<Mesh>& mesh)
	{
		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.ByteWidth = static_cast<UINT>(vertexDataSize);
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA vertexSubresourceData = {};
		vertexSubresourceData.pSysMem = vertexData;
		if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &mesh->vertexBuffer)))
		{
			LOG_ERROR("Failed to create vertex buffer for procedural mesh.");
			return;
		}
		D3D11_BUFFER_DESC indexBufferDesc = {};
		indexBufferDesc.ByteWidth = static_cast<UINT>(indexDataSize);
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		D3D11_SUBRESOURCE_DATA indexSubresourceData = {};
		indexSubresourceData.pSysMem = indexData;
		if (FAILED(device->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &mesh->indexBuffer)))
		{
			LOG_ERROR("Failed to create index buffer for procedural mesh.");
			return;
		}
	}


	// クワッド（四角形）メッシュの作成
	std::shared_ptr<Mesh> CreateQuad(float width, float height)
	{
		auto device = Graphics::GetDevice();
		// 頂点データの作成
		struct Vertex {
			float x, y, z; // 位置
			float u, v;    // UV座標
		};
		std::vector<Vertex> vertices = {
			{-width / 2, -height / 2, 0.0f, 0.0f, 1.0f}, // 左下
			{ width / 2, -height / 2, 0.0f, 1.0f, 1.0f}, // 右下
			{ width / 2,  height / 2, 0.0f, 1.0f, 0.0f}, // 右上
			{-width / 2,  height / 2, 0.0f, 0.0f, 0.0f}  // 左上
		};
		// インデックスデータの作成（2つの三角形でクワッドを構成）
		std::vector<uint32_t> indices = {
			0, 1, 2,
			2, 3, 0
		};
		// メッシュの設定
		auto mesh = std::make_shared<Mesh>();
		mesh->vertexStride = sizeof(Vertex);
		mesh->indexFormat = DXGI_FORMAT_R32_UINT;
		
		// バッファの作成
		CreateComBuffers(device, vertices.data(), vertices.size() * sizeof(Vertex), indices.data(), indices.size() * sizeof(uint32_t), mesh);

		// サブメッシュの設定
		mesh->subMeshes.push_back({ static_cast<uint32_t>(indices.size()), 0, -1 });

		// バウンディングボックスの計算
		mesh->localBounds.Encapsulate({ -width / 2, -height / 2, 0.0f });
		mesh->localBounds.Encapsulate({ width / 2, height / 2, 0.0f });

		return mesh;
	}

	// プレーンメッシュの作成
	std::shared_ptr<Mesh> CreatePlane(float width, float height, uint32_t widthSegments, uint32_t heightSegments)
	{
		auto device = Graphics::GetDevice();
		// 頂点データの作成
		struct Vertex {
			float x, y, z; // 位置
			float u, v;    // UV座標
		};
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		uint32_t vertexCount = (widthSegments + 1) * (heightSegments + 1);
		uint32_t indexCount = widthSegments * heightSegments * 6;
		vertices.reserve(vertexCount);
		indices.reserve(indexCount);
		// 頂点の生成
		for (uint32_t y = 0; y <= heightSegments; y++)
		{
			for (uint32_t x = 0; x <= widthSegments; x++)
			{
				float u = static_cast<float>(x) / widthSegments;
				float v = static_cast<float>(y) / heightSegments;
				vertices.push_back({ u * width - width / 2, v * height - height / 2, 0.0f, u, v });
			}
		}
		// インデックスの生成
		for (uint32_t y = 0; y < heightSegments; y++)
		{
			for (uint32_t x = 0; x < widthSegments; x++)
			{
				uint32_t i0 = y * (widthSegments + 1) + x;
				uint32_t i1 = i0 + 1;
				uint32_t i2 = i0 + (widthSegments + 1);
				uint32_t i3 = i2 + 1;
				indices.push_back(i0);
				indices.push_back(i1);
				indices.push_back(i3);
				indices.push_back(i3);
				indices.push_back(i2);
				indices.push_back(i0);
			}
		}

		// メッシュの設定
		auto mesh = std::make_shared<Mesh>();
		mesh->vertexStride = sizeof(Vertex);
		mesh->indexFormat = DXGI_FORMAT_R32_UINT;

		// バッファの作成
		CreateComBuffers(device, vertices.data(), vertices.size() * sizeof(Vertex), indices.data(), indices.size() * sizeof(uint32_t), mesh);

		// サブメッシュの設定
		mesh->subMeshes.push_back({ static_cast<uint32_t>(indices.size()), 0, -1 });

		// バウンディングボックスの計算
		mesh->localBounds.Encapsulate({ -width / 2, -height / 2, 0.0f });
		mesh->localBounds.Encapsulate({ width / 2, height / 2, 0.0f });

		return mesh;
	}

	// キューブメッシュの作成
	std::shared_ptr<Mesh> CreateCube(float width, float height, float depth)
	{
		struct Vertex
		{
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT3 normal;
		};
		// 頂点データの作成（各面ごとに4頂点、合計24頂点）
		Vertex vertices[] = {
			// 前面
			{{-width / 2, -height / 2, -depth / 2}, {0, 0, -1}},
			{{ width / 2, -height / 2, -depth / 2}, {0, 0, -1}},
			{{ width / 2,  height / 2, -depth / 2}, {0, 0, -1}},
			{{-width / 2,  height / 2, -depth / 2}, {0, 0, -1}},
			// 背面
			{{-width / 2, -height / 2, depth / 2}, {0, 0, 1}},
			{{ width / 2, -height / 2, depth / 2}, {0, 0, 1}},
			{{ width / 2,  height / 2, depth / 2}, {0, 0, 1}},
			{{-width / 2,  height / 2, depth / 2}, {0, 0, 1}},
			// 左面
			{{-width / 2, -height / 2, depth / 2}, {-1, 0, 0}},
			{{-width / 2, -height / 2, -depth / 2}, {-1, 0, 0}},
			{{-width / 2, height / 2, -depth / 2}, {-1, 0, 0}},
			{{-width / 2, height / 2, depth / 2}, {-1, 0, 0}},
			// 右面
			{{width / 2, -height / 2, -depth / 2}, {1, 0, 0}},
			{{width / 2, -height / 2, depth / 2}, {1, 0, 0}},
			{{width / 2, height / 2, depth / 2}, {1, 0, 0}},
			{{width / 2, height / 2, -depth / 2}, {1, 0, 0}},
			// 上面
			{{-width / 2, height / 2, -depth / 2}, {0, 1, 0}},
			{{width / 2, height / 2, -depth / 2}, {0, 1, 0}},
			{{width / 2, height / 2, depth / 2}, {0, 1, 0}},
			{{-width / 2, height / 2, depth / 2}, {0, 1, 0}},
			// 下面
			{{-width / 2, -height / 2, depth / 2}, {0, -1, 0}},
			{{width / 2, -height / 2, depth / 2}, {0, -1, 0}},
			{{width / 2, -height / 2, -depth / 2}, {0, -1, 0}},
			{{-width / 2, -height / 2, -depth / 2}, {0, -1, 0}},
		};
		// インデックスデータの作成（各面ごとに2つの三角形、合計36インデックス）
		uint32_t indices[] = {
			0, 1, 2, 2, 3, 0,       // 前面
			4, 5, 6, 6, 7, 4,       // 背面
			8, 9, 10, 10, 11, 8,    // 左面
			12, 13, 14, 14, 15, 12, // 右面
			16, 17, 18, 18, 19, 16, // 上面
			20, 21, 22, 22, 23, 20  // 下面
		};

		// メッシュの設定
		auto device = Graphics::GetDevice();
		auto mesh = std::make_shared<Mesh>();

		mesh->vertexStride = sizeof(Vertex);
		mesh->indexFormat = DXGI_FORMAT_R32_UINT;

		// バッファの作成
		CreateComBuffers(device, vertices, sizeof(vertices), indices, sizeof(indices), mesh);

		// サブメッシュの設定
		mesh->subMeshes.push_back({ static_cast<uint32_t>(std::size(indices)), 0, -1 });

		// バウンディングボックスの計算
		mesh->localBounds.Encapsulate({ -width / 2, -height / 2, -depth / 2 });
		mesh->localBounds.Encapsulate({ width / 2, height / 2, depth / 2 });
		return mesh;
	}

	// 球メッシュの作成
	std::shared_ptr<Mesh> CreateSphere(float radius, uint32_t latitudeSegments, uint32_t longitudeSegments)
	{
		struct Vertex
		{
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT3 normal;
		};
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		uint32_t vertexCount = (latitudeSegments + 1) * (longitudeSegments + 1);
		uint32_t indexCount = latitudeSegments * longitudeSegments * 6;
		vertices.reserve(vertexCount);
		indices.reserve(indexCount);
		for (uint32_t lat = 0; lat <= latitudeSegments; lat++)
		{
			float theta = lat * XM_PI / latitudeSegments;
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);
			for (uint32_t lon = 0; lon <= longitudeSegments; lon++)
			{
				float phi = lon * 2 * XM_PI / longitudeSegments;
				float sinPhi = sinf(phi);
				float cosPhi = cosf(phi);
				float x = cosPhi * sinTheta;
				float y = cosTheta;
				float z = sinPhi * sinTheta;
				vertices.push_back({ {radius * x, radius * y, radius * z}, {x, y, z} });
			}
		}
		for (uint32_t lat = 0; lat < latitudeSegments; lat++)
		{
			for (uint32_t lon = 0; lon < longitudeSegments; lon++)
			{
				uint32_t i0 = lat * (longitudeSegments + 1) + lon;
				uint32_t i1 = i0 + 1;
				uint32_t i2 = i0 + (longitudeSegments + 1);
				uint32_t i3 = i2 + 1;
				if (lat != 0) {
					indices.push_back(i0);
					indices.push_back(i2);
					indices.push_back(i1);
				}
				if (lat != latitudeSegments - 1) {
					indices.push_back(i1);
					indices.push_back(i2);
					indices.push_back(i3);
				}
			}
		}
		// メッシュの設定
		auto device = Graphics::GetDevice();
		auto mesh = std::make_shared<Mesh>();
		mesh->vertexStride = sizeof(Vertex);
		mesh->indexFormat = DXGI_FORMAT_R32_UINT;

		// バッファの作成
		CreateComBuffers(device, vertices.data(), vertices.size() * sizeof(Vertex), indices.data(), indices.size() * sizeof(uint32_t), mesh);

		// サブメッシュの設定
		mesh->subMeshes.push_back({ static_cast<uint32_t>(indices.size()), 0, -1 });

		// バウンディングボックスの計算
		mesh->localBounds.Encapsulate({ -radius, -radius, -radius });
		mesh->localBounds.Encapsulate({ radius, radius, radius });
		return mesh;
	}

	// シリンダーメッシュの作成
	std::shared_ptr<Mesh> CreateCylinder(float radius, float height, uint32_t radialSegments)
	{
		struct Vertex
		{
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT3 normal;
		};
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		uint32_t vertexCount = (radialSegments + 1) * 2 + 2; // 円周上の頂点 + 上面中心点 + 下面中心点
		uint32_t indexCount = radialSegments * 12; // 側面の三角形 + 上面の三角形 + 下面の三角形
		vertices.reserve(vertexCount);
		indices.reserve(indexCount);

		// 頂点の生成
		for (uint32_t i = 0; i <= radialSegments; i++)
		{
			float theta = i * 2 * XM_PI / radialSegments;
			float cosTheta = cosf(theta);
			float sinTheta = sinf(theta);
			vertices.push_back({ {radius * cosTheta, -height / 2, radius * sinTheta}, {cosTheta, 0, sinTheta} });
			vertices.push_back({ {radius * cosTheta, height / 2, radius * sinTheta}, {cosTheta, 0, sinTheta} });
		}
		// インデックスの生成
		for (uint32_t i = 0; i < radialSegments; i++)
		{
			uint32_t i0 = i * 2;
			uint32_t i1 = i0 + 1;
			uint32_t i2 = ((i + 1) % (radialSegments + 1)) * 2;
			uint32_t i3 = i2 + 1;
			indices.push_back(i0);
			indices.push_back(i2);
			indices.push_back(i1);
			indices.push_back(i1);
			indices.push_back(i2);
			indices.push_back(i3);
		}
		// 上面と下面の中心点を追加
		vertices.push_back({ {0, height / 2, 0}, {0, 1, 0} }); // 上面中心点
		vertices.push_back({ {0, -height / 2, 0}, {0, -1, 0} }); // 下面中心点
		uint32_t topCenterIndex = static_cast<uint32_t>(vertices.size() - 2);
		uint32_t bottomCenterIndex = static_cast<uint32_t>(vertices.size() - 1);
		// 上面のインデックス生成
		for (uint32_t i = 0; i < radialSegments; i++)
		{
			uint32_t i0 = i * 2 + 1; // 上面の頂点
			uint32_t i1 = ((i + 1) % (radialSegments + 1)) * 2 + 1; // 次の上面の頂点
			indices.push_back(i0);
			indices.push_back(i1);
			indices.push_back(topCenterIndex);
		}
		// 下面のインデックス生成
		for (uint32_t i = 0; i < radialSegments; i++)
		{
			uint32_t i0 = i * 2; // 下面の頂点
			uint32_t i1 = ((i + 1) % (radialSegments + 1)) * 2; // 次の下面の頂点
			indices.push_back(i1);
			indices.push_back(i0);
			indices.push_back(bottomCenterIndex);
		}
		
		// メッシュの設定
		auto device = Graphics::GetDevice();
		auto mesh = std::make_shared<Mesh>();
		mesh->vertexStride = sizeof(Vertex);
		mesh->indexFormat = DXGI_FORMAT_R32_UINT;

		// バッファの作成
		CreateComBuffers(device, vertices.data(), vertices.size() * sizeof(Vertex), indices.data(), indices.size() * sizeof(uint32_t), mesh);

		// サブメッシュの設定
		mesh->subMeshes.push_back({ static_cast<uint32_t>(indices.size()), 0, -1 });

		// バウンディングボックスの計算
		mesh->localBounds.Encapsulate({ -radius, -height / 2, -radius });
		mesh->localBounds.Encapsulate({ radius, height / 2, radius });
		return mesh;
	}

	// カプセルメッシュの作成
	std::shared_ptr<Mesh> CreateCapsule(float radius, float height, uint32_t radialSegments, uint32_t heightSegments)
	{
		// カプセルはシリンダーと半球を組み合わせた形状です。ここでは、シリンダー部分を中心にして、上下に半球を追加する形で頂点とインデックスを生成します。
		// 頂点データの作成
		struct Vertex
		{
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT3 normal;
		};
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		uint32_t cylinderVertexCount = (radialSegments + 1) * 2; // シリンダー部分の頂点数
		uint32_t hemisphereVertexCount = (heightSegments + 1) * (radialSegments + 1) * 2; // 上半球と下半球の頂点数
		uint32_t vertexCount = cylinderVertexCount + hemisphereVertexCount;
		uint32_t indexCount = radialSegments * 12 + heightSegments * radialSegments * 12; // シリンダー部分のインデックス数 + 上半球と下半球のインデックス数
		vertices.reserve(vertexCount);
		indices.reserve(indexCount);

		// シリンダー部分の頂点生成
		for (uint32_t i = 0; i <= radialSegments; i++)
		{
			float theta = i * 2 * XM_PI / radialSegments;
			float cosTheta = cosf(theta);
			float sinTheta = sinf(theta);
			vertices.push_back({ {radius * cosTheta, -height / 2, radius * sinTheta}, {cosTheta, 0, sinTheta} });
			vertices.push_back({ {radius * cosTheta, height / 2, radius * sinTheta}, {cosTheta, 0, sinTheta} });
		}
		// シリンダー部分のインデックス生成
		for (uint32_t i = 0; i < radialSegments; i++)
		{
			uint32_t i0 = i * 2;
			uint32_t i1 = i0 + 1;
			uint32_t i2 = ((i + 1) % (radialSegments + 1)) * 2;
			uint32_t i3 = i2 + 1;
			indices.push_back(i0);
			indices.push_back(i2);
			indices.push_back(i1);
			indices.push_back(i1);
			indices.push_back(i2);
			indices.push_back(i3);
		}
		// 上半球の頂点生成
		for (uint32_t lat = 0; lat <= heightSegments; lat++)
		{
			float phi = lat * XM_PI / (2 * heightSegments);
			float sinPhi = sinf(phi);
			float cosPhi = cosf(phi);
			for (uint32_t lon = 0; lon <= radialSegments; lon++)
			{
				float theta = lon * 2 * XM_PI / radialSegments;
				float cosTheta = cosf(theta);
				float sinTheta = sinf(theta);
				float x = cosTheta * sinPhi;
				float y = cosPhi;
				float z = sinTheta * sinPhi;
				vertices.push_back({ {radius * x, height / 2 + radius * y, radius * z}, {x, y, z} });
			}
		}
		// 下半球の頂点生成
		for (uint32_t lat = 0; lat <= heightSegments; lat++)
		{
			float phi = lat * XM_PI / (2 * heightSegments);
			float sinPhi = sinf(phi);
			float cosPhi = cosf(phi);
			for (uint32_t lon = 0; lon <= radialSegments; lon++)
			{
				float theta = lon * 2 * XM_PI / radialSegments;
				float cosTheta = cosf(theta);
				float sinTheta = sinf(theta);
				float x = cosTheta * sinPhi;
				float y = -cosPhi;
				float z = sinTheta * sinPhi;
				vertices.push_back({ {radius * x, -height / 2 + radius * y, radius * z}, {x, y, z} });
			}
		}
		// 上半球のインデックス生成
		for (uint32_t lat = 0; lat < heightSegments; lat++)
		{
			for (uint32_t lon = 0; lon < radialSegments; lon++)
			{
				uint32_t i0 = cylinderVertexCount + lat * (radialSegments + 1) + lon;
				uint32_t i1 = i0 + 1;
				uint32_t i2 = i0 + (radialSegments + 1);
				uint32_t i3 = i2 + 1;
				if (lat != 0) {
					indices.push_back(i0);
					indices.push_back(i2);
					indices.push_back(i1);
				}
				indices.push_back(i1);
				indices.push_back(i2);
				indices.push_back(i3);
			}
		}
		// 下半球のインデックス生成
		uint32_t topHemisphereVertexCount = (heightSegments + 1) * (radialSegments + 1); // 上半球のみの頂点数
		for (uint32_t lat = 0; lat < heightSegments; lat++)
		{
			for (uint32_t lon = 0; lon < radialSegments; lon++)
			{
				uint32_t i0 = cylinderVertexCount + topHemisphereVertexCount + lat * (radialSegments + 1) + lon;
				uint32_t i1 = i0 + 1;
				uint32_t i2 = i0 + (radialSegments + 1);
				uint32_t i3 = i2 + 1;
				if (lat != 0) {
					indices.push_back(i0);
					indices.push_back(i2);
					indices.push_back(i1);
				}
				indices.push_back(i1);
				indices.push_back(i2);
				indices.push_back(i3);
			}
		}
		
		// メッシュの設定
		auto device = Graphics::GetDevice();
		auto mesh = std::make_shared<Mesh>();
		mesh->vertexStride = sizeof(Vertex);
		mesh->indexFormat = DXGI_FORMAT_R32_UINT;

		// バッファの作成
		CreateComBuffers(device, vertices.data(), vertices.size() * sizeof(Vertex), indices.data(), indices.size() * sizeof(uint32_t), mesh);

		// サブメッシュの設定
		mesh->subMeshes.push_back({ static_cast<uint32_t>(indices.size()), 0, -1 });

		// バウンディングボックスの計算
		mesh->localBounds.Encapsulate({ -radius, -height / 2, -radius });
		mesh->localBounds.Encapsulate({ radius, height / 2, radius });
		return mesh;
	}

}