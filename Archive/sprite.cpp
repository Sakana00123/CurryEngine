#include "pch.h"
#include "sprite.h"
#include "Engine/Core/Misc.h"
#include "Engine/Resources/Texture.h"
#include "Engine/Resources/Shader.h"
#include "Engine/Core/EnginePaths.h"

Sprite::Sprite(ID3D11Device* device)
{
    HRESULT hr{ S_OK };

    //頂点情報のセット
    //Vertex vertices[]
    //{
    //	{ { -1,  1, 0 }, { 1, 1, 1, 1 } },
    //	{ {  1,  1, 0 }, { 1, 0, 0, 1 } },
    //	{ { -1, -1, 0 }, { 0, 1, 0, 1 } },
    //	{ {  1, -1, 0 }, { 0, 0, 1, 1 } },
    //};
    Vertex vertices[]
    {
        { { -1,  1, 0 }, { 1, 1, 1, 1 } },
        { {  1,  1, 0 }, { 1, 1, 1, 1 } },
        { { -1, -1, 0 }, { 1, 1, 1, 1 } },
        { {  1, -1, 0 }, { 1, 1, 1, 1 } },
    };

    //頂点バッファオブジェクトの生成
    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.ByteWidth = sizeof(vertices);
    buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    buffer_desc.MiscFlags = 0;
    buffer_desc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA subresource_data{};
    subresource_data.pSysMem = vertices;
    subresource_data.SysMemPitch = 0;
    subresource_data.SysMemSlicePitch = 0;
    hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer.ReleaseAndGetAddressOf());
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
    hr = CreatePixelShaderFromCSO(device, (dir + "debug_ps.cso").c_str(), pixel_shader.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

Sprite::Sprite(ID3D11Device* device, const wchar_t* filename)
{
    HRESULT hr{ S_OK };

    //頂点情報のセット
    Vertex vertices[]
    {
        { { -1.0f,  1.0f, 0 }, { 1, 1, 1, 1 }, { 0, 0 } },
        { {  1.0f,  1.0f, 0 }, { 1, 1, 1, 1 }, { 1, 0 } },
        { { -1.0f, -1.0f, 0 }, { 1, 1, 1, 1 }, { 0, 1 } },
        { {  1.0f, -1.0f, 0 }, { 1, 1, 1, 1 }, { 1, 1 } },
    };

    //頂点バッファオブジェクトの生成
    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.ByteWidth = sizeof(vertices);
    buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    buffer_desc.MiscFlags = 0;
    buffer_desc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA subresource_data{};
    subresource_data.pSysMem = vertices;
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

void Sprite::Render(ID3D11DeviceContext* immediate_context,
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

    //計算結果で頂点バッファオブジェクトを更新する
    HRESULT hr{ S_OK };
    D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
    hr = immediate_context->Map(vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

    Vertex* vertices{ reinterpret_cast<Vertex*>(mapped_subresource.pData) };
    if (vertices != nullptr)
    {
        vertices[0].position = { x0, y0, 0 };
        vertices[1].position = { x1, y1, 0 };
        vertices[2].position = { x2, y2, 0 };
        vertices[3].position = { x3, y3, 0 };
        vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = { color.x,color.y,color.z,color.w };

        vertices[0].texcoord = { tx0 / texture2d_desc.Width,ty0 / texture2d_desc.Height };
        vertices[1].texcoord = { tx1 / texture2d_desc.Width,ty1 / texture2d_desc.Height };
        vertices[2].texcoord = { tx2 / texture2d_desc.Width,ty2 / texture2d_desc.Height };
        vertices[3].texcoord = { tx3 / texture2d_desc.Width,ty3 / texture2d_desc.Height };
    }
    immediate_context->Unmap(vertex_buffer.Get(), 0);

    if (shader_resource_view.Get() != nullptr) {
        //シェーダーリソースのバインド
        immediate_context->PSSetShaderResources(0, 1, shader_resource_view.GetAddressOf());
    }

    //頂点バッファのバインド
    UINT stride{ sizeof(Vertex) };
    UINT offset{ 0 };
    immediate_context->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);

    //プリミティブタイプおよびデータの順序に関する情報のバインド
    immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    //入力レイアウトオブジェクトのバインド
    immediate_context->IASetInputLayout(input_layout.Get());

    //シェーダーのバインド
    immediate_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
    immediate_context->PSSetShader(pixel_shader.Get(), nullptr, 0);

    //プリミティブの描画
    immediate_context->Draw(4, 0);
}

void Sprite::Render(ID3D11DeviceContext* immediate_context,
    float dx, float dy,
    float dw, float dh,
    DirectX::XMFLOAT4 color,
    float angle,
    float sx, float sy,
    float sw, float sh,
    float scaleX, float scaleY,
    float pivotX, float pivotY)
{
    // スクリーンのサイズを取得
    D3D11_VIEWPORT viewport{};
    UINT num_viewports{ 1 };
    immediate_context->RSGetViewports(&num_viewports, &viewport);

    // 無効なスケーリングなら描画しない
    if (scaleX == 0.0f || scaleY == 0.0f) return;

    // sw, sh が負なら、テクスチャ全体を使う（FLT_MINの代替）
    if (sw < 0.0f && sh < 0.0f)
    {
        sw = static_cast<float>(texture2d_desc.Width);
        sh = static_cast<float>(texture2d_desc.Height);
    }

    // ピボット考慮した頂点座標
    float x0 = dx - pivotX * dw * scaleX;
    float y0 = dy - pivotY * dh * scaleY;
    float x1 = x0 + dw * scaleX;
    float y1 = y0;
    float x2 = x0;
    float y2 = y0 + dh * scaleY;
    float x3 = x0 + dw * scaleX;
    float y3 = y0 + dh * scaleY;

    // UV座標の計算（ピクセル → テクスチャ座標系）
    float texW = static_cast<float>(texture2d_desc.Width);
    float texH = static_cast<float>(texture2d_desc.Height);

    float tx0 = sx;
    float ty0 = sy;
    float tx1 = sx + sw;
    float ty1 = sy;
    float tx2 = sx;
    float ty2 = sy + sh;
    float tx3 = sx + sw;
    float ty3 = sy + sh;

    // 回転中心は描画位置(dx, dy)
    float cx = dx;
    float cy = dy;

    // 回転処理（Rotate関数がラジアン想定とする）
    Rotate(x0, y0, cx, cy, angle);
    Rotate(x1, y1, cx, cy, angle);
    Rotate(x2, y2, cx, cy, angle);
    Rotate(x3, y3, cx, cy, angle);

    // スクリーン座標系 → NDC座標系
    auto toNDC = [&](float& x, float& y) {
        x = 2.0f * x / viewport.Width - 1.0f;
        y = 1.0f - 2.0f * y / viewport.Height;
        };
    toNDC(x0, y0); toNDC(x1, y1); toNDC(x2, y2); toNDC(x3, y3);

    // 頂点バッファ更新
    D3D11_MAPPED_SUBRESOURCE mapped{};
    HRESULT hr = immediate_context->Map(vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (SUCCEEDED(hr))
    {
        Vertex* vertices = reinterpret_cast<Vertex*>(mapped.pData);
        if (vertices)
        {
            vertices[0].position = { x0, y0, 0 };
            vertices[1].position = { x1, y1, 0 };
            vertices[2].position = { x2, y2, 0 };
            vertices[3].position = { x3, y3, 0 };

            vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = { color.x, color.y, color.z, color.w };

            vertices[0].texcoord = { tx0 / texW, ty0 / texH };
            vertices[1].texcoord = { tx1 / texW, ty1 / texH };
            vertices[2].texcoord = { tx2 / texW, ty2 / texH };
            vertices[3].texcoord = { tx3 / texW, ty3 / texH };
        }
        immediate_context->Unmap(vertex_buffer.Get(), 0);
    }

    // テクスチャが無い場合は描画しない
    if (!shader_resource_view) return;

    // リソース・パイプライン設定
    immediate_context->PSSetShaderResources(0, 1, shader_resource_view.GetAddressOf());

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    immediate_context->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);
    immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    immediate_context->IASetInputLayout(input_layout.Get());

    immediate_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
    immediate_context->PSSetShader(pixel_shader.Get(), nullptr, 0);

    immediate_context->Draw(4, 0);
}


void Sprite::Render(ID3D11DeviceContext* immediate_context,
	float dx, float dy,
	float dw, float dh,
	DirectX::XMFLOAT2 pivot,
	DirectX::XMFLOAT4 color,
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
	float x0{ dx - dw * pivot.x };
	float y0{ dy - dh * pivot.y };
	// right-top
	float x1{ dx + dw * (1.f - pivot.x) };
	float y1{ dy - dh * pivot.y };
	// left-bottom
	float x2{ dx - dw * pivot.x };
	float y2{ dy + dh * (1.f - pivot.y) };
	// right-bottom
	float x3{ dx + dw * (1.f - pivot.x) };
	float y3{ dy + dh * (1.f - pivot.y) };

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

	//スクリーン座標系からNDCへの座標変換を行う
	x0 = 2.0f * x0 / viewport.Width - 1.0f;
	y0 = 1.0f - 2.0f * y0 / viewport.Height;
	x1 = 2.0f * x1 / viewport.Width - 1.0f;
	y1 = 1.0f - 2.0f * y1 / viewport.Height;
	x2 = 2.0f * x2 / viewport.Width - 1.0f;
	y2 = 1.0f - 2.0f * y2 / viewport.Height;
	x3 = 2.0f * x3 / viewport.Width - 1.0f;
	y3 = 1.0f - 2.0f * y3 / viewport.Height;

	//計算結果で頂点バッファオブジェクトを更新する
	HRESULT hr{ S_OK };
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	hr = immediate_context->Map(vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	Vertex* vertices{ reinterpret_cast<Vertex*>(mapped_subresource.pData) };
	if (vertices != nullptr)
	{
		vertices[0].position = { x0, y0, 0 };
		vertices[1].position = { x1, y1, 0 };
		vertices[2].position = { x2, y2, 0 };
		vertices[3].position = { x3, y3, 0 };
		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = { color.x,color.y,color.z,color.w };

		vertices[0].texcoord = { tx0 / texture2d_desc.Width,ty0 / texture2d_desc.Height };
		vertices[1].texcoord = { tx1 / texture2d_desc.Width,ty1 / texture2d_desc.Height };
		vertices[2].texcoord = { tx2 / texture2d_desc.Width,ty2 / texture2d_desc.Height };
		vertices[3].texcoord = { tx3 / texture2d_desc.Width,ty3 / texture2d_desc.Height };
	}
	immediate_context->Unmap(vertex_buffer.Get(), 0);

	if (shader_resource_view.Get() != nullptr) {
		//シェーダーリソースのバインド
		immediate_context->PSSetShaderResources(0, 1, shader_resource_view.GetAddressOf());
	}

	//頂点バッファのバインド
	UINT stride{ sizeof(Vertex) };
	UINT offset{ 0 };
	immediate_context->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);

	//プリミティブタイプおよびデータの順序に関する情報のバインド
	immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//入力レイアウトオブジェクトのバインド
	immediate_context->IASetInputLayout(input_layout.Get());

	//シェーダーのバインド
	immediate_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
	immediate_context->PSSetShader(pixel_shader.Get(), nullptr, 0);

	//プリミティブの描画
	immediate_context->Draw(4, 0);
}

void Sprite::TextOut(ID3D11DeviceContext* immediate_context, std::string s,
    float x, float y, float w, float h, DirectX::XMFLOAT4 color)
{
    float sw = static_cast<float>(texture2d_desc.Width / 16);
    float sh = static_cast<float>(texture2d_desc.Height / 16);
    float carriage = 0;
    for (const char c : s)
    {
        Render(immediate_context, x + carriage, y, w, h, color, 0, sw * (c & 0x0F), sh * (c >> 4), sw, sh);
        carriage += w;
    }
}

Sprite::~Sprite()
{

}