#include "pch.h"
#include "MeshRenderer.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Core/Transform.h"
#include "Engine/Resources/AssetDatabase.h"

REGISTER_COMPONENT(MeshRenderer, "Renderer")

void MeshRenderer::Initialize()
{
	Renderer::Initialize();
}

void MeshRenderer::Render(RenderContext* rtx)
{
	if (!modelRenderer) return;

	// メッシュの描画
	Transform* transform = GetTransform();
	XMFLOAT4X4 world = transform->GetWorld();
	XMMATRIX worldMatrix = XMLoadFloat4x4(&world);
	modelRenderer->Draw(rtx, worldMatrix);
}

Math::BoundingBox MeshRenderer::CalculateAABB() const
{
	if (!modelRenderer) return Math::BoundingBox();

	// モデルアセットのノード情報から、ローカル空間のAABBを計算する
	AssetModel* asset = modelRenderer->m_asset.get();
	Math::BoundingBox localBounds;

	//nodeのglobalTransformから、ローカル空間のバウンディングボックスを作成する
	for (auto& node : asset->nodes)
	{
		Vector3 point;
		XMVECTOR S, R, T;
		XMMatrixDecompose(&S, &R, &T, XMLoadFloat4x4(&node.globalTransform));
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&point), T);
		localBounds.Encapsulate(point);

		// メッシュのバウンディングボックスを取得する
		for (auto& meshIndex : node.meshIndices)
		{
			if (meshIndex < 0) continue;
			const AssetModel::MeshData& mesh = asset->meshes[meshIndex];
			if (mesh.isSkinned)
			{
				for (const auto& vertex : mesh.skinnedVertices)
				{
					localBounds.Encapsulate(vertex.position);
				}
			}
			else
			{
				for (const auto& vertex : mesh.staticVertices)
				{
					localBounds.Encapsulate(vertex.position);
				}
			}
		}
	}
	// ローカル空間のAABBをワールド空間に変換する
	Math::BoundingBox worldBounds;
	Vector3 worldMin, worldMax;
	XMFLOAT4X4 world = GetTransform()->GetWorld();
	XMMATRIX worldMatrix = XMLoadFloat4x4(&world);
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&worldMin), XMVector3TransformCoord(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&localBounds.min)), worldMatrix));
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&worldMax), XMVector3TransformCoord(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&localBounds.max)), worldMatrix));
	worldBounds.Encapsulate(worldMin);
	worldBounds.Encapsulate(worldMax);

	return worldBounds;
}

#ifdef USE_IMGUI
void MeshRenderer::DrawProperty(const PropertyDrawContext& context)
{
	Renderer::DrawProperty(context);


}
#endif // USE_IMGUI

json MeshRenderer::Serialize() const
{
	json j = Renderer::Serialize();
	// メッシュのパスを保存
	if (!meshAssetPath.empty())
	{
		//std::u8string u8MeshAssetPath(meshAssetPath.begin(), meshAssetPath.end());
		//j["meshAssetPath"] = u8MeshAssetPath;
		j["meshAssetPath"] = meshAssetPath;
	}
	
	return j;
}

void MeshRenderer::Deserialize(const json& j)
{
	Renderer::Deserialize(j);
	// メッシュのパスからメッシュをロード
	if (j.contains("meshAssetPath"))
	{
		//std::u8string u8MeshAssetPath = j["meshAssetPath"].get<std::u8string>();
		//meshAssetPath = std::string(u8MeshAssetPath.begin(), u8MeshAssetPath.end());
		meshAssetPath = j["meshAssetPath"].get<std::string>();
		if (!meshAssetPath.empty())
		{
			if (CurryEngine::Resources::AssetMeta* meta = CurryEngine::Resources::AssetDatabase::GetOrImport(meshAssetPath))
			{
				modelRenderer = std::make_shared<ModelRenderer>();
				std::shared_ptr<AssetModel> assetModel = CurryEngine::Resources::AssetDatabase::LoadAsset<AssetModel>(meta->id);
				modelRenderer->SetModelAsset(assetModel);
			}
		}
	}
}
