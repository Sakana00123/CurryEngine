#include "pch.h"
#include "MeshRenderer.h"
#include "Engine/Resources/ProceduralMesh.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

REGISTER_COMPONENT(MeshRenderer, "Renderer")

void MeshRenderer::Initialize()
{
	// デフォルトでキューブを設定
	SetPrimitiveMesh(primitiveType);

	// デフォルトマテリアルの作成
	if (!material)
	{
		auto device = Graphics::GetDevice();
		material = std::make_shared<Material>();
		std::shared_ptr<Shader> vs = ResourceManager::Load<VertexShader>("./Data/Shaders/geometric_primitive_vs.cso");
		std::shared_ptr<Shader> ps = ResourceManager::Load<PixelShader>("./Data/Shaders/geometric_primitive_ps.cso");
		material->SetShader(device, vs);
		material->SetShader(device, ps);
		// シーン定数バッファをバインドしないように設定（このマテリアルはシーン定数バッファを使用しないため）
		material->SetNotBindCBuffer({ "SCENE_CONSTANT_BUFFER" });

		// デフォルトの白色テクスチャを作成して設定
		material->SetValue("materialColor", Color::White);
	}
}

void MeshRenderer::Render(RenderContext* rtx)
{
	if (!mesh) return;

	auto immediateContext = rtx->immediateContext;

	// メッシュの頂点バッファとインデックスバッファをセット
	uint32_t vertexStride{ mesh->vertexStride };
	uint32_t vertexOffset{ 0 };
	immediateContext->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(), &vertexStride, &vertexOffset);
	immediateContext->IASetIndexBuffer(mesh->indexBuffer.Get(), mesh->indexFormat, 0);

	// プリミティブトポロジーを設定（ここでは三角形リストを想定）
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// マテリアルが設定されていれば適用
	if (material)
	{
		material->SetValue("world", GetTransform()->GetWorld());
		material->Apply(rtx);
	}

	// サブメッシュごとに描画
	for (const auto& subMesh : mesh->subMeshes)
	{
		// インデックスを使用して描画
		immediateContext->DrawIndexed(subMesh.indexCount, subMesh.indexOffset, 0);
	}
}

Math::BoundingBox MeshRenderer::CalculateAABB() const
{
	if (!mesh) return Math::BoundingBox();
	// メッシュのローカル空間のバウンディングボックスをワールド空間に変換して返す
	Math::BoundingBox localBounds = mesh->localBounds;
	Math::BoundingBox worldBounds;
	Vector3 worldMin, worldMax;
	XMFLOAT4X4 world = GetTransform()->GetWorld();
	XMMATRIX worldMatrix = XMLoadFloat4x4(&world);
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&worldMin), XMVector3TransformCoord(XMLoadFloat3(&localBounds.min), worldMatrix));
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&worldMax), XMVector3TransformCoord(XMLoadFloat3(&localBounds.max), worldMatrix));
	worldBounds.Encapsulate(worldMin);
	worldBounds.Encapsulate(worldMax);
	return worldBounds;
}

#ifdef USE_IMGUI
void MeshRenderer::DrawProperty(const PropertyDrawContext& context)
{
	Renderer::DrawProperty(context);

	// プリミティブタイプの選択
	const char* primitiveTypes[] = { "Cube", "Sphere", "Plane", "Capsule", "Cylinder" };
	if (ImGui::Combo("Primitive Type", &primitiveType, primitiveTypes, IM_ARRAYSIZE(primitiveTypes)))
	{
		SetPrimitiveMesh(primitiveType);
	}


	// メッシュのプロパティを表示
	if (mesh)
	{
		ImGui::Text("SubMeshCount: %d", mesh->subMeshes.size());
	}
	else
	{
		ImGui::Text("Mesh: None");
	}

}
#endif // USE_IMGUI

json MeshRenderer::Serialize() const
{
	json j = Renderer::Serialize();
	// メッシュのパスを保存
	//j["meshPath"] = mesh ? mesh->GetPath() : "";
	j["primitiveType"] = primitiveType;
	return j;
}

void MeshRenderer::Deserialize(const json& j)
{
	Renderer::Deserialize(j);
	// メッシュのパスからメッシュをロード
	//if (j.contains("meshPath") && j["meshPath"].is_string())
	//{
	//	std::string meshPath = j["meshPath"];
	//	if (!meshPath.empty())
	//	{
	//		mesh = std::make_shared<Mesh>();
	//		mesh->LoadFromFile(meshPath);
	//	}
	//}
	if (j.contains("primitiveType") && j["primitiveType"].is_number_integer())
	{
		SetPrimitiveMesh(j["primitiveType"]);
	}
}

void MeshRenderer::SetPrimitiveMesh(int type)
{
	primitiveType = type;
	switch (type)
	{
	case 0: // キューブ
		mesh = ProceduralMesh::CreateCube(1, 1, 1);
		break;
	case 1: // 球
		mesh = ProceduralMesh::CreateSphere(1);
		break;
	case 2: // 平面
		mesh = ProceduralMesh::CreatePlane(1, 1);
		break;
	case 3: // カプセル
		mesh = ProceduralMesh::CreateCapsule(1, 2);
		break;
	case 4: // 円柱
		mesh = ProceduralMesh::CreateCylinder(1, 2);
		break;
	default:
		mesh = nullptr;
		break;
	}
}