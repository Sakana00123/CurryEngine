#include "pch.h"
#include "sprite_batch.h"
#include "Engine/Core/Misc.h"
#include "Engine/Resources/Texture.h"
#include "Engine/Resources/Shader.h"
#include <memory>
#include "Engine/Core/EnginePaths.h"

SpriteBatch::SpriteBatch(ID3D11Device* device, const wchar_t* filename, size_t max_sprites)
	: max_vertices(max_sprites * 6)
{
	HRESULT hr{ S_OK };

	std::unique_ptr<Vertex[]> vertices{ std::make_unique<Vertex[]>(max_vertices) };

	//頂点バッファオブジェクトの生成
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * max_vertices);
	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA subresource_data{};
	subresource_data.pSysMem = vertices.get();
	subresource_data.SysMemPitch = 0;
	subresource_data.SysMemSlicePitch = 0;
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	//画像ファイルのロードとシェーダーリソースビューオブジェクトの生成
	hr = LoadTextureFromFile(device, filename, shader_resource_view.GetAddressOf(), &texture2d_desc);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	//入力レイアウトオブジェクトの生成
	D3D11_INPUT_ELEMENT_DESC input_element_desc[]
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
		D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
		D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	//頂点シェーダーオブジェクトの生成
	std::string dir = EnginePaths::ShadersDataDir;
	hr = CreateVertexShaderFromCSO(device, (dir + "sprite_vs.cso").c_str(), vertex_shader.GetAddressOf(), input_layout.GetAddressOf(), input_element_desc, _countof(input_element_desc));
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	//ピクセルシェーダーオブジェクトの生成
	hr = CreatePixelShaderFromCSO(device, (dir + "sprite_ps.cso").c_str(), pixel_shader.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void SpriteBatch::Begin(ID3D11DeviceContext* immediate_context)
{
	vertices.clear();
	immediate_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
	immediate_context->PSSetShader(pixel_shader.Get(), nullptr, 0);
	immediate_context->PSSetShaderResources(0, 1, shader_resource_view.GetAddressOf());
}

void SpriteBatch::End(ID3D11DeviceContext* immediate_context)
{
	HRESULT hr{ S_OK };
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	hr = immediate_context->Map(vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	size_t vertex_count = vertices.size();
	_ASSERT_EXPR(max_vertices >= vertex_count, "Buffer overflow");
	Vertex* data{ reinterpret_cast<Vertex*>(mapped_subresource.pData) };
	if (data != nullptr)
	{
		const Vertex* p = vertices.data();
		memcpy_s(data, max_vertices * sizeof(Vertex), p, vertex_count * sizeof(Vertex));
	}
	immediate_context->Unmap(vertex_buffer.Get(), 0);

	UINT stride{ sizeof(Vertex) };
	UINT offset{ 0 };
	immediate_context->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);
	immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	immediate_context->IASetInputLayout(input_layout.Get());

	immediate_context->Draw(static_cast<UINT>(vertex_count), 0);
}

inline void Rotate(float& x, float& y, float cx, float cy, float angle)
{
	x -= cx;
	y -= cy;

	float cos{ cosf(DirectX::XMConvertToRadians(angle)) };
	float sin{ sinf(DirectX::XMConvertToRadians(angle)) };
	float tx{ x }, ty{ y };
	x = cos * tx + -sin * ty;
	y = sin * tx + cos * ty;

	x += cx;
	y += cy;
}

void SpriteBatch::Render(ID3D11DeviceContext* immediate_context,
	float dx, float dy,
	float dw, float dh,
	DirectX::XMFLOAT4 color,
	float angle,
	float sx, float sy,
	float sw, float sh
)
{
	//スクリーンのサイズを取得する
	D3D11_VIEWPORT viewport{};
	UINT num_viewports{ 1 };
	immediate_context->RSGetViewports(&num_viewports, &viewport);

	//引数から矩形の各頂点の位置（スクリーン座標系）を計算する
	// left-top
	float x0{ dx };
	float y0{ dy };
	// right-top
	float x1{ dx + dw };
	float y1{ dy };
	// left-bottom
	float x2{ dx };
	float y2{ dy + dh };
	// right-bottom
	float x3{ dx + dw };
	float y3{ dy + dh };

	//切り取り位置
	//left-top
	float tx0{ sx };
	float ty0{ sy };
	//right-top
	float tx1{ sx + sw };
	float ty1{ sy };
	//left-bottom
	float tx2{ sx };
	float ty2{ sy + sh };
	//right-bottom
	float tx3{ sx + sw };
	float ty3{ sy + sh };

	//回転の中心を矩形の中心点にした場合
	float cx = dx + dw * 0.5f;
	float cy = dy + dh * 0.5f;

	//回転処理
	Rotate(x0, y0, cx, cy, angle);
	Rotate(x1, y1, cx, cy, angle);
	Rotate(x2, y2, cx, cy, angle);
	Rotate(x3, y3, cx, cy, angle);

	//スクリーン座標系からNDCへの座標変換を行う
	x0 = 2.0f * x0 / viewport.Width - 1.0f;
	y0 = 1.0f - 2.0f * y0 / viewport.Height;
	x1 = 2.0f * x1 / viewport.Width - 1.0f;
	y1 = 1.0f - 2.0f * y1 / viewport.Height;
	x2 = 2.0f * x2 / viewport.Width - 1.0f;
	y2 = 1.0f - 2.0f * y2 / viewport.Height;
	x3 = 2.0f * x3 / viewport.Width - 1.0f;
	y3 = 1.0f - 2.0f * y3 / viewport.Height;

	float u0{ sx / texture2d_desc.Width };
	float v0{ sy / texture2d_desc.Height };
	float u1{ (sx + sw) / texture2d_desc.Width };
	float v1{ (sy + sh) / texture2d_desc.Height };

	vertices.push_back({ { x0, y0,0 }, color, { u0, v0 } });
	vertices.push_back({ { x1, y1,0 }, color, { u1, v0 } });
	vertices.push_back({ { x2, y2,0 }, color, { u0, v1 } });
	vertices.push_back({ { x2, y2,0 }, color, { u0, v1 } });
	vertices.push_back({ { x1, y1,0 }, color, { u1, v0 } });
	vertices.push_back({ { x3, y3,0 }, color, { u1, v1 } });
}

SpriteBatch::~SpriteBatch()
{

}