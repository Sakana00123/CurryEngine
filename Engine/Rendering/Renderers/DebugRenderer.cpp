#include "pch.h"
#include "DebugRenderer.h"
#include "Engine/Rendering/Pipeline/RenderContext.h"
//#include "Engine/Core/Math/Matrix4x4.h"
#include "Engine/Resources/Shader.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Core/EnginePaths.h"

void DebugRenderer::Initialize()
{
	// デバッグ描画の初期化処理を実装
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	auto device = Graphics::GetDevice();
	std::string dir = EnginePaths::ShadersDataDir;
	CreateVertexShaderFromCSO(device, (dir + "DebugRendererVS.cso").c_str(), vertexShader.ReleaseAndGetAddressOf(),
		inputLayout.ReleaseAndGetAddressOf(), inputElementDesc, _countof(inputElementDesc));
	CreatePixelShaderFromCSO(device, (dir + "DebugRendererPS.cso").c_str(), pixelShader.ReleaseAndGetAddressOf());

	// 頂点バッファの作成
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(Vertex) * VertexCapacity;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, vertexBuffer.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	// 定数バッファの作成
	bufferDesc.ByteWidth = sizeof(ConstantBufferData);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void DebugRenderer::Finalize()
{
	// デバッグ描画の終了処理を実装
}


void DebugRenderer::DrawAll(RenderContext* rtx, D3D11_PRIMITIVE_TOPOLOGY topology)
{
	// デバッグ描画の実行処理を実装
	auto dc = rtx->immediateContext;
	auto& view = rtx->view;
	auto& projection = rtx->projection;

	// シェーダー設定
	dc->VSSetShader(vertexShader.Get(), nullptr, 0);
	dc->PSSetShader(pixelShader.Get(), nullptr, 0);
	dc->IASetInputLayout(inputLayout.Get());

	// 定数バッファ設定
	dc->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

	// ビュープロジェクション行列作成
	DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&view);
	DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&projection);
	DirectX::XMMATRIX VP = V * P;

	// 定数バッファ更新
	ConstantBufferData data;
	DirectX::XMStoreFloat4x4(&data.viewProjection, VP);
	dc->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);

	// 頂点バッファ設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	dc->IASetPrimitiveTopology(topology);
	dc->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
	dc->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

	// 描画
	UINT totalVertexCount = static_cast<UINT>(vertices.size());
	UINT start = 0;
	UINT count = (totalVertexCount < VertexCapacity) ? totalVertexCount : VertexCapacity;

	while (start < totalVertexCount)
	{
		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		HRESULT hr = dc->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
		_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

		memcpy(mappedSubresource.pData, &vertices[start], sizeof(Vertex) * count);

		dc->Unmap(vertexBuffer.Get(), 0);

		dc->Draw(count, 0);

		start += count;
		if ((start + count) > totalVertexCount)
		{
			count = totalVertexCount - start;
		}
	}

	vertices.clear();

}

void DebugRenderer::AddVertex(const Vector3& position, const Color& color)
{
	// 頂点を追加する処理を実装
	if (vertices.size() < VertexCapacity)
	{
		vertices.push_back({ position, color });
	}
}


void DebugRenderer::DrawLine(const Vector3& start, const Vector3& end, const Color& color)
{
	// 線分を描画する処理を実装
	AddVertex(start, color);
	AddVertex(end, color);
}


void DebugRenderer::DrawGrid(const Vector3& center, float size, int divisions, const Color& color)
{
	// グリッドを描画する処理を実装
	float halfSize = size * 0.5f;
	float step = size / divisions;
	for (int i = 0; i <= divisions; ++i)
	{
		float offset = -halfSize + i * step;
		// X軸方向の線
		DrawLine({ center.x + offset, center.y, center.z - halfSize }, { center.x + offset, center.y, center.z + halfSize }, color);
		// Z軸方向の線
		DrawLine({ center.x - halfSize, center.y, center.z + offset }, { center.x + halfSize, center.y, center.z + offset }, color);
	}
}