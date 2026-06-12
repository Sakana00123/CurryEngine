#include "pch.h"
#include "MeshCollider.h"
#include "Engine/Rendering/Renderers/GltfModelRenderer.h"
#include "Rigidbody.h"

REGISTER_COMPONENT(MeshCollider, "Physics")

void MeshCollider::Initialize()
{
	// デバッグプリミティブの準備など、必要な初期化処理をここに実装します。
	Collider::Initialize();
}

void MeshCollider::Register()
{
	// ここでMeshColliderを持つゲームオブジェクトにMeshColliderを追加し、物理エンジンに登録する処理を実装
	GltfModelRenderer* modelRenderer = GetOwner()->GetComponent<GltfModelRenderer>();

	if (Rigidbody* rigidbody = GetOwner()->GetComponent<Rigidbody>())
	{
		if (!convex && !rigidbody->IsKinematic())
		{
			rigidbody->SetKinematic(true);
			Console::LogError("MeshCollider with non-convex mesh must be used with a kinematic Rigidbody. The Rigidbody has been set to kinematic automatically.");
		}
	}

	if (modelRenderer)
	{
		MeshColliderData data;
		if (modelRenderer->batchMeshes.size() > 0)
		{
			for (const auto& batchMesh : modelRenderer->batchMeshes)
			{
				size_t vertexOffset = data.vertices.size(); // 現在の頂点数をオフセットとして保存
				data.vertices.reserve(vertexOffset + batchMesh.cachedVertices.size());
				for (const auto& vertex : batchMesh.cachedVertices)
				{
					// BatchMeshの頂点データも必要に応じて、この時点で配置済みであることを前提とします
					data.vertices.push_back({ vertex.position.x, vertex.position.y, vertex.position.z });
				}
				for (const auto& index : batchMesh.cachedIndices)
				{
					data.indices.push_back(static_cast<int>(index + vertexOffset)); // インデックスデータは頂点オフセットを加算して追加
					
					_ASSERT_EXPR(
						index + vertexOffset <= INT_MAX,
						L"index overflow"
					);
				}
			}
			
		}
		else
		{
			// Meshを直接使うのではなく、Nodeが持つtransform行列を考慮して展開する
			for (const auto& node : modelRenderer->nodes)
			{
				if (node.mesh >= 0 && node.mesh < modelRenderer->meshes.size())
				{
					const auto& mesh = modelRenderer->meshes[node.mesh];
					DirectX::XMMATRIX globalTransform = DirectX::XMLoadFloat4x4(&node.globalTransform);

					for (const auto& primitive : mesh.primitives)
					{
						size_t vertexOffset = data.vertices.size(); // 現在の頂点数をオフセットとして保存
						data.vertices.reserve(vertexOffset + primitive.cachedVertices.size());
						for (const auto& vertex : primitive.cachedVertices)
						{
							// 頂点座標にノードの変換行列を適用してワールドスケール/ローカル配置を反映する
							DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&vertex.position);
							pos = DirectX::XMVector3TransformCoord(pos, globalTransform);
							DirectX::XMFLOAT3 transformedPos;
							DirectX::XMStoreFloat3(&transformedPos, pos);
							
							data.vertices.push_back({ transformedPos.x, transformedPos.y, transformedPos.z });
						}
						// インデックスデータのフォーマットに合わせて byte 配列から読み取る
						if (primitive.indexBufferView.format == DXGI_FORMAT_R32_UINT)
						{
							const uint32_t* indices = reinterpret_cast<const uint32_t*>(primitive.cachedIndices.data());
							size_t numIndices = primitive.cachedIndices.size() / sizeof(uint32_t);
							data.indices.reserve(data.indices.size() + numIndices);
							for (size_t i = 0; i < numIndices; ++i)
							{
								data.indices.push_back(static_cast<int>(indices[i] + vertexOffset));
							}
						}
						else if (primitive.indexBufferView.format == DXGI_FORMAT_R16_UINT)
						{
							const uint16_t* indices = reinterpret_cast<const uint16_t*>(primitive.cachedIndices.data());
							size_t numIndices = primitive.cachedIndices.size() / sizeof(uint16_t);
							data.indices.reserve(data.indices.size() + numIndices);
							for (size_t i = 0; i < numIndices; ++i)
							{
								data.indices.push_back(static_cast<int>(indices[i] + vertexOffset));
							}
						}
						else if (primitive.indexBufferView.format == DXGI_FORMAT_R8_UINT)
						{
							const uint8_t* indices = reinterpret_cast<const uint8_t*>(primitive.cachedIndices.data());
							size_t numIndices = primitive.cachedIndices.size() / sizeof(uint8_t);
							data.indices.reserve(data.indices.size() + numIndices);
							for (size_t i = 0; i < numIndices; ++i)
							{
								data.indices.push_back(static_cast<int>(indices[i] + vertexOffset));
							}
						}
					}
				}
			}
		}
		data.materialHandle = m_materialHandle; // マテリアルハンドルを設定
		data.isTrigger = isTrigger; // トリガーかどうかのフラグを設定
		data.contactOffset = contactOffset; // 接触オフセットを設定
		data.collider = this; // コライダへのポインタを設定


		if (convex)
		{
			// 物理エンジンに凸メッシュとしてコライダーを登録
			if (!Physics::AddConvexMeshShape(GetTransform(), data, m_shapeHandle))
			{
				// 追加に失敗した場合のエラーハンドリング
				Console::LogError("Failed to add MeshCollider shape to physics engine.");
			}
		}
		else
		{
			// 物理エンジンに三角形メッシュとしてコライダーを登録
			if (!Physics::AddTriangleMeshShape(GetTransform(), data, m_shapeHandle))
			{
				// 追加に失敗した場合のエラーハンドリング
				Console::LogError("Failed to add MeshCollider shape to physics engine.");
			}
		}
	}
	else
	{
		Console::LogWarning("MeshCollider requires a GltfModelRenderer component to obtain the mesh asset path.");
		return;
	}
}

void MeshCollider::SyncWithPhysics()
{
	// 物理エンジンにローカルポーズを更新
	// MeshColliderは通常、位置や回転の変更があまりないため、必要に応じて実装します。

}

void MeshCollider::Render(RenderContext* rtx)
{
#ifdef _DEBUG
	// デバッグ描画の実装をここに追加します。
#endif
}

#ifdef USE_IMGUI
//void MeshCollider::DrawProperty(const PropertyDrawContext& context)
//{
//	IMGUI_PROPERTY_BEGIN();
//
//	Collider::DrawProperty(context);
//
//	bool edited = false;
//	IMGUI_PROPERTY_BOOL("Convex", convex, edited);
//	if (edited)
//	{
//		SetNeedSync(); // convex プロパティが変更されたことを通知して、物理エンジンに同期する必要があることを示します。
//	}
//
//	IMGUI_PROPERTY_END();
//
//}
#endif // USE_IMGUI

json MeshCollider::Serialize() const
{
	json j = Collider::Serialize();
	return j;
}

void MeshCollider::Deserialize(const json& j)
{
	Collider::Deserialize(j);
}

bool MeshCollider::IsConvex() const
{
	return convex;
}

void MeshCollider::SetConvex(bool isConvex)
{
	if (convex != isConvex)
	{
		convex = isConvex;
		SetNeedSync(); // convex プロパティが変更されたことを通知して、物理エンジンに同期する必要があることを示します。
	}
}