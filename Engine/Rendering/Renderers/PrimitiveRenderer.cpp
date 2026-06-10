#include "pch.h"
#include "PrimitiveRenderer.h"
#include "Engine/Resources/Shader.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Resources/ResourceManager.h"

REGISTER_COMPONENT(PrimitiveRenderer, "Rendering")

PrimitiveRenderer::PrimitiveRenderer()
{
	auto device = Graphics::GetDevice();
#ifndef USE_MATERIAL
	HRESULT hr{ S_OK };

	D3D11_INPUT_ELEMENT_DESC input_element_desc[]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		  D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		  D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	CreateVertexShaderFromCSO(device, "./Data/Shaders/geometric_primitive_vs.cso", vertexShader.GetAddressOf(),
		inputLayout.GetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
	CreatePixelShaderFromCSO(device, "./Data/Shaders/geometric_primitive_ps.cso", pixelShader.GetAddressOf());

	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(Constants);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = device->CreateBuffer(&buffer_desc, nullptr, constantBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
#else

	// マテリアルを作成
	material = std::make_shared<Material>();
	//auto shader = std::make_shared<Shader>();
	
	// シェーダを読み込み、マテリアルに設定
	std::shared_ptr<Shader> vs = ResourceManager::GetShader<VertexShader>("geometric_primitive_vs");
	std::shared_ptr<Shader> ps = ResourceManager::GetShader<PixelShader>("geometric_primitive_ps");
	material->SetShader(device, vs);
	material->SetShader(device, ps);

	// デフォルトの白色マテリアルを設定
	material->SetValue("materialColor", Color::White);

#endif // !USE_MATERIAL
}

void PrimitiveRenderer::SetShape(const Shape& shape)
{
	auto device = Graphics::GetDevice();
	switch (shape)
	{
	case Shape::Cube:
		CreateCube(device);
		break;
	case Shape::Cylinder:
		CreateCylinder(device);
		break;
	case Shape::Sphere:
		CreateSphere(device);
		break;
	default:
		break;
	}
}

void PrimitiveRenderer::CreateCube(ID3D11Device* device)
{
	Vertex vertices[24]{};
	// サイズが1.0の正立方体データを作成する（重心を原点にする）。正立方体のコントロールポイント数は８個、
	// １つのコントロールポイントの位置には法線の向きが違う頂点が３個あるので頂点情報の総数は８x３＝２４個、
	// 頂点情報配列（vertices）にすべての頂点の位置・法線情報を格納する。

	//Front(左上0,右上1,左下2,右下3)
	{
		vertices[0].position = { -0.5f, 0.5f, -0.5f };
		vertices[1].position = { 0.5f, 0.5f, -0.5f };
		vertices[2].position = { -0.5f, -0.5f, -0.5f };
		vertices[3].position = { 0.5f, -0.5f, -0.5f };

		vertices[0].normal = vertices[1].normal = vertices[2].normal = vertices[3].normal = { 0,0,-1 };
	}
	//Right(左上4,右上5,左下6,右下7)
	{
		vertices[4].position = { 0.5f, 0.5f, -0.5f };
		vertices[5].position = { 0.5f, 0.5f, 0.5f };
		vertices[6].position = { 0.5f, -0.5f, -0.5f };
		vertices[7].position = { 0.5f, -0.5f, 0.5f };

		vertices[4].normal = vertices[5].normal = vertices[6].normal = vertices[7].normal = { 1,0,0 };
	}
	//Back
	{
		vertices[8].position = { 0.5f, 0.5f, 0.5f };
		vertices[9].position = { -0.5f, 0.5f, 0.5f };
		vertices[10].position = { 0.5f, -0.5f, 0.5f };
		vertices[11].position = { -0.5f, -0.5f, 0.5f };

		vertices[8].normal = vertices[9].normal = vertices[10].normal = vertices[11].normal = { 0,0,1 };
	}
	//Left
	{
		vertices[12].position = { -0.5f, 0.5f, 0.5f };
		vertices[13].position = { -0.5f, 0.5f, -0.5f };
		vertices[14].position = { -0.5f, -0.5f, 0.5f };
		vertices[15].position = { -0.5f, -0.5f, -0.5f };

		vertices[12].normal = vertices[13].normal = vertices[14].normal = vertices[15].normal = { -1,0,0 };
	}
	//Up
	{
		vertices[16].position = { -0.5f, 0.5f, 0.5f };
		vertices[17].position = { 0.5f, 0.5f, 0.5f };
		vertices[18].position = { -0.5f, 0.5f, -0.5f };
		vertices[19].position = { 0.5f, 0.5f, -0.5f };

		vertices[16].normal = vertices[17].normal = vertices[18].normal = vertices[19].normal = { 0,1,0 };
	}
	//Down
	{
		vertices[20].position = { -0.5f, -0.5f, -0.5f };
		vertices[21].position = { 0.5f, -0.5f, -0.5f };
		vertices[22].position = { -0.5f, -0.5f, 0.5f };
		vertices[23].position = { 0.5f, -0.5f, 0.5f };

		vertices[20].normal = vertices[21].normal = vertices[22].normal = vertices[23].normal = { 0,-1,0 };
	}

	uint32_t indices[36]{};
	// 正立方体は６面持ち、１つの面は２つの３角形ポリゴンで構成されるので、３角形ポリゴンの総数は６x２＝１２個、
	// 正立方体を描画するために１２回の３角形ポリゴン描画が必要、よって参照される頂点情報は１２x３＝３６回、
	// ３角形ポリゴンが参照する頂点情報のインデックス（頂点番号）を描画順に配列（indices）に格納する。
	// 時計回りが表面になるように格納すること。

	//Front
	indices[0] = 2;
	indices[1] = 0;
	indices[2] = 1;

	indices[3] = 2;
	indices[4] = 1;
	indices[5] = 3;
	//Right
	indices[6] = 6;
	indices[7] = 4;
	indices[8] = 5;

	indices[9] = 6;
	indices[10] = 5;
	indices[11] = 7;

	//Back
	indices[12] = 10;
	indices[13] = 8;
	indices[14] = 9;

	indices[15] = 10;
	indices[16] = 9;
	indices[17] = 11;

	//Left
	indices[18] = 14;
	indices[19] = 12;
	indices[20] = 13;

	indices[21] = 14;
	indices[22] = 13;
	indices[23] = 15;

	//Up
	indices[24] = 18;
	indices[25] = 16;
	indices[26] = 17;

	indices[27] = 18;
	indices[28] = 17;
	indices[29] = 19;

	//Down
	indices[30] = 22;
	indices[31] = 20;
	indices[32] = 21;

	indices[33] = 22;
	indices[34] = 21;
	indices[35] = 23;

	//バッファ生成
	CreateComBuffers(device, vertices, 24, indices, 36);

	shape = Shape::Cube;
}

void PrimitiveRenderer::CreateCylinder(ID3D11Device* device, int segmentCount)
{
	// 頂点データ、インデックスデータの生成(半径1、高さ1の単位円柱を生成する。)

	std::vector<Vertex> vertices/*((segmentCount * 6) + 2)*/;
	std::vector<uint32_t> indices/*(segmentCount * 12)*/;
	// 円柱の側面は、円周を segmentCount 等分して、各セグメントを２つの三角形で構成する。
	float angleStep = DirectX::XM_2PI / segmentCount;
	//上下面
	for (int t = 0; t < 2; t++) {
		DirectX::XMFLOAT3 normal{ 0,1,0 };
		normal.y = (t == 0) ? 1.f : -1.f;
		float y = normal.y * 0.5f;
		vertices.push_back({ { 0,y,0 }, normal });//[0]
		vertices.push_back({ { cosf(0.f),y,sinf(0.f) }, normal });
		for (int i = 1; i <= segmentCount; i++) {
			float theta = i * angleStep;
			float x = cosf(theta);
			float z = sinf(theta);
			if (i < segmentCount)
				vertices.push_back({ { x,y,z }, normal });
			if (t == 0)
			{
				indices.push_back(i);
				indices.push_back(0);
				indices.push_back((i < segmentCount) ? (i + 1) : 1);
			}
			else
			{
				indices.push_back(t * segmentCount + t);
				indices.push_back(t * segmentCount + i + t);
				indices.push_back((i < segmentCount) ? (t * segmentCount + i + 2) : t * segmentCount + 2);
			}
		}
	}
	//側面
	int beginIndex = 2 * segmentCount + 4;
	for (int i = 0; i < segmentCount * 2; i += 2) {
		float theta0 = i * angleStep;
		float theta1 = (i + 1) * angleStep;
		float x0 = cosf(theta0);
		float x1 = cosf(theta1);
		float z0 = sinf(theta0);
		float z1 = sinf(theta1);
		DirectX::XMFLOAT3 normal{ cosf(theta0 + theta1), 0, sinf(theta0 + theta1) };
		vertices.push_back({ {x0,0.5f, z0}, normal });
		vertices.push_back({ {x0,-0.5f, z0}, normal });
		vertices.push_back({ {x1,0.5f, z1}, normal });
		vertices.push_back({ {x1,-0.5f, z1}, normal });

		indices.push_back(beginIndex + i + 1);
		indices.push_back(beginIndex + i);
		indices.push_back(beginIndex + i + 2);

		indices.push_back(beginIndex + i + 1);
		indices.push_back(beginIndex + i + 2);
		indices.push_back(beginIndex + i + 3);
	}
	CreateComBuffers(device, vertices.data(), vertices.size(), indices.data(), indices.size());

	shape = Shape::Cylinder;
}

void PrimitiveRenderer::CreateSphere(ID3D11Device* device, int stackCount, int sliceCount)
{
    //頂点データ、インデックスデータの生成(半径1の単位球を生成する)
    std::vector<Vertex> vertices;
    vertices.reserve(static_cast<size_t>(stackCount + 1) * static_cast<size_t>(sliceCount + 1));

    for (int stack = 0; stack <= stackCount; ++stack) {
        float phi = XM_PI * stack / stackCount; // 緯度

        for (int slice = 0; slice <= sliceCount; ++slice) {
            float theta = XM_2PI * slice / sliceCount; // 経度
            // 極座標から直交座標に変換
            XMFLOAT3 position = {
                sinf(phi) * cosf(theta), // x
                cosf(phi),                // y
                sinf(phi) * sinf(theta)  // z
            };
            // 明示的に正規化して法線を求める（安全策）
            DirectX::XMVECTOR vpos = DirectX::XMLoadFloat3(&position);
            DirectX::XMVECTOR vnorm = DirectX::XMVector3Normalize(vpos);
            XMFLOAT3 normal;
            DirectX::XMStoreFloat3(&normal, vnorm);

            vertices.push_back({ position, normal });
        }
    }

    std::vector<uint32_t> indices;
    indices.reserve(static_cast<size_t>(stackCount) * static_cast<size_t>(sliceCount) * 6);

    for (int stack = 0; stack < stackCount; ++stack) {
        for (int slice = 0; slice < sliceCount; ++slice) {
            int a = stack * (sliceCount + 1) + slice;
            int b = a + sliceCount + 1;

            // トライアングルの生成
            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(a + 1);

            indices.push_back(a + 1);
            indices.push_back(b);
            indices.push_back(b + 1);
        }
    }

    CreateComBuffers(device, vertices.data(), vertices.size(), indices.data(), indices.size());

    shape = Shape::Sphere;
}

void PrimitiveRenderer::CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertexCount,
	uint32_t* indices, size_t indexCount)
{
	HRESULT hr{ S_OK };

	// 頂点バッファ、インデックスバッファの生成
	D3D11_BUFFER_DESC buffer_desc{};
	D3D11_SUBRESOURCE_DATA subresource_data{};
	buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertexCount);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	subresource_data.pSysMem = vertices;
	subresource_data.SysMemPitch = 0;
	subresource_data.SysMemSlicePitch = 0;
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertexBuffer.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	buffer_desc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indexCount);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	subresource_data.pSysMem = indices;
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, indexBuffer.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void PrimitiveRenderer::Render(RenderContext* rtx)
{
	// バッファがない場合は描画しない
	if (!vertexBuffer || !indexBuffer)
	{
		return;
	}

	ID3D11DeviceContext* immediateContext = rtx->immediateContext;

	// 頂点バッファ、インデックスバッファのセット
	uint32_t stride{ sizeof(Vertex) };
	uint32_t offset{ 0 };
	immediateContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	immediateContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

#ifdef USE_MATERIAL

	// マテリアルのセット
	//material->SetVector("materialColor", color);
	material->SetValue("world", gameObject->transform->GetWorld());
	material->Apply(rtx);

#else

	// シェーダー、インプットレイアウトのセット
	immediateContext->IASetInputLayout(inputLayout.Get());

	immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
	immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);

	Constants data{ gameObject->transform->GetWorld(), color};
	immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
	immediateContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

#endif // USE_MATERIAL

	// 描画
	D3D11_BUFFER_DESC buffer_desc{};
	indexBuffer->GetDesc(&buffer_desc);
	immediateContext->DrawIndexed(buffer_desc.ByteWidth / sizeof(uint32_t), 0, 0);
}

#ifdef USE_IMGUI
void PrimitiveRenderer::DrawProperty(const PropertyDrawContext& context)
{

	if (ImGui::BeginCombo("Shape", 
		shape == Shape::Cube ? "Cube" : shape == Shape::Cylinder ? "Cylinder" : "Sphere"))
	{
		if (ImGui::Selectable("Cube", shape == Shape::Cube))
		{
			SetShape(Shape::Cube);
		}
		if (ImGui::Selectable("Cylinder", shape == Shape::Cylinder))
		{
			SetShape(Shape::Cylinder);
		}
		if (ImGui::Selectable("Sphere", shape == Shape::Sphere))
		{
			SetShape(Shape::Sphere);
		}
		ImGui::EndCombo();
	}

#ifndef USE_MATERIAL
	ImGui::ColorEdit4("Color", &color.r);
#else
	Renderer::DrawProperty(context);
#endif // !USE_MATERIAL
}
#endif // USE_IMGUI

Math::BoundingBox PrimitiveRenderer::CalculateAABB() const
{
	// ワールド変換を考慮したAABBを計算して返す
	Math::BoundingBox aabb;
	
	XMFLOAT3 scale = gameObject->transform->GetWorldScale();

	// スケールを考慮したAABBを計算（立方体のサイズが1.0なので、-0.5～0.5をスケールで拡大縮小）
	aabb = Math::BoundingBox({ -0.5f * scale.x, -0.5f * scale.y, -0.5f * scale.z }, { 0.5f * scale.x, 0.5f * scale.y, 0.5f * scale.z });

	return aabb;
}

json PrimitiveRenderer::Serialize() const
{
	json j = Renderer::Serialize();
	j["shape"] = static_cast<int>(shape);
	Color color = this->color;
	material->GetValue("materialColor", color);
	j["color"] = { color.r, color.g, color.b, color.a };

	return j;
}
void PrimitiveRenderer::Deserialize(const json& j)
{
	Renderer::Deserialize(j);
	if (j.contains("shape"))
	{
		shape = static_cast<Shape>(j["shape"].get<int>());
		SetShape(shape);
	}
	if (j.contains("color"))
	{
		json colorArray = j["color"];
		color.r = colorArray[0].get<float>();
		color.g = colorArray[1].get<float>();
		color.b = colorArray[2].get<float>();
		color.a = colorArray[3].get<float>();
	}
	material->SetValue("materialColor", color);

}