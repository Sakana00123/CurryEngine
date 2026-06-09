#include "pch.h"
#include "Image.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Scenes/SceneManager.h"
#include "Canvas.h"
#include "Engine/Resources/ResourceManager.h"
//#include "CanvasScaler.h"

REGISTER_COMPONENT_WITH_ATTRIBUTES(Image, "UI", ComponentAttributes::RequiredComponent , { "RectTransform" });

Image::Image()
{
	ID3D11Device* device = Graphics::GetDevice();

	HRESULT hr{ S_OK };
	//頂点情報のセット
	Vertex vertices[]
	{
		{ { -1,  1, 0, 1 }, { 1, 1, 1, 1 } },
		{ {  1,  1, 0, 1 }, { 1, 0, 0, 1 } },
		{ { -1, -1, 0, 1 }, { 0, 1, 0, 1 } },
		{ {  1, -1, 0, 1 }, { 0, 0, 1, 1 } },
	};
	//頂点バッファオブジェクトの生成
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = vertices;
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;
	hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBuffer.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	//画像ファイルのロードとシェーダーリソースビューオブジェクトの生成
	/*hr = SetSource(device, filePath, shaderResourceView.ReleaseAndGetAddressOf(), &texture2dDesc,false);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));*/

	/*std::shared_ptr<Texture> texture = std::make_shared<Texture>();
	texture->MakeDummy(device);
	material.SetTexture("color_map", texture);*/

	//material.
	SetSource(nullptr);

	//material.SetTexture(0, ResourceManager::Load<Texture>(""));
	/*hr = SetSource(device, nullptr, maskTexture.ReleaseAndGetAddressOf(), NULL, false);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));*/
	std::shared_ptr<AssetTexture> maskTexture = std::make_shared<AssetTexture>();
	maskTexture->MakeDummy(device);
	maskMaterial.SetTexture("maskTexture", maskTexture);

	std::shared_ptr<Shader> vs = ResourceManager::GetOrLoadShader<VertexShader>("sprite_vs");
	std::shared_ptr<Shader> ps = ResourceManager::GetOrLoadShader<PixelShader>("sprite_ps");
	material.SetShader(device, vs);
	material.SetShader(device, ps);

	//マスク用ピクセルシェーダー
	std::shared_ptr<Shader> maskPS = ResourceManager::GetOrLoadShader<PixelShader>("SpriteMaskPS");
	maskMaterial.SetShader(device, vs);
	maskMaterial.SetShader(device, maskPS);
}

void Image::SetSource(const wchar_t* source, bool reload)
{
	std::shared_ptr<AssetTexture> texture;
	if (source) {
		std::wstring path = source;
		texture = reload ? ResourceManager::Load<AssetTexture>(std::string(path.begin(), path.end()))
			: ResourceManager::GetOrLoad<AssetTexture>(std::string(path.begin(), path.end()));
	}
	if (!texture) {
		texture = std::make_shared<AssetTexture>();
		texture->MakeDummy(Graphics::GetDevice());
	}
	material.SetTexture("color_map", texture);

	// テクスチャ変更フラグが立っていたら初期化
	if (material.IsTextureChanged())
	{
		Initialize();
		// フラグをクリア
		material.ClearTextureChangedFlag();
	}
}

AssetTexture* Image::GetTexture()
{
	auto texture = material.GetTexture<Texture>("color_map").lock();
	return texture ? dynamic_cast<AssetTexture*>(texture.get()) : nullptr;
}

void Image::Initialize()
{
	//sx = 0, sy = 0, sw = static_cast<float>(texture2dDesc.Width > 0 ? texture2dDesc.Width : 100), sh = static_cast<float>(texture2dDesc.Height > 0 ? texture2dDesc.Height : 100);
	if (material.GetTexture("color_map").expired()) return;
	const auto& texture2d = material.GetTexture<Texture2D>("color_map");
	const D3D11_TEXTURE2D_DESC& texture2dDesc = !texture2d.expired() ? texture2d.lock()->GetDesc() : D3D11_TEXTURE2D_DESC{};
	sx = 0, sy = 0, sw = static_cast<float>(texture2dDesc.Width > 0 ? texture2dDesc.Width : 100), sh = static_cast<float>(texture2dDesc.Height > 0 ? texture2dDesc.Height : 100);
#ifdef _DEBUG
	//if (SceneManager::state == SceneManager::State::Editing)
	//{
	//	rect->size.x = sw, rect->size.y = sh;
	//}
#endif // _DEBUG

}

void Image::Draw(RenderContext* rtx) 
{
	RectTransform* rectTransform = GetRectTransform();
	if (!rectTransform || material.GetTexture("color_map").expired()) return;
	ID3D11DeviceContext* immediateContext = rtx->immediateContext;
	//D3D11_VIEWPORT viewport{};
	//UINT numViewports{ 1 };
	//immediateContext->RSGetViewports(&numViewports, &viewport);

	float refW = 1920.0f, refH = 1080.0f;
	/*if (Canvas* canvas = GetCanvas())
	{
		if (CanvasScaler* scaler = canvas->GetOwner()->GetComponent<CanvasScaler>())
		{
			refW = scaler->referenceWidth;
			refH = scaler->referenceHeight;
		}
	}*/

	//引数から矩形の各頂点の位置（スクリーン座標系）を計算する
	// left-top
	float x0{ rectTransform->TopLeft().x };
	float y0{ rectTransform->TopLeft().y };
	// right-top
	float x1{ rectTransform->TopRight().x };
	float y1{ rectTransform->TopRight().y };
	// left-bottom
	float x2{ rectTransform->BottomLeft().x };
	float y2{ rectTransform->BottomLeft().y };
	// right-bottom
	float x3{ rectTransform->BottomRight().x };
	float y3{ rectTransform->BottomRight().y };

	//切り取り位置
	float u = this->uv.x * sw;
	float v = this->uv.y * sh;
	//left-top
	float tx0{ sx + u };
	float ty0{ sy + v };
	//right-top
	float tx1{ sx + sw + u };
	float ty1{ sy + v };
	//left-bottom
	float tx2{ sx + u };
	float ty2{ sy + sh + v };
	//right-bottom
	float tx3{ sx + sw + u };
	float ty3{ sy + sh + v };

	//スクリーン座標系からNDCへの座標変換を行う
	x0 = 2.0f * x0 / refW - 1.0f;
	y0 = 1.0f - 2.0f * y0 / refH;
	x1 = 2.0f * x1 / refW - 1.0f;
	y1 = 1.0f - 2.0f * y1 / refH;
	x2 = 2.0f * x2 / refW - 1.0f;
	y2 = 1.0f - 2.0f * y2 / refH;
	x3 = 2.0f * x3 / refW - 1.0f;
	y3 = 1.0f - 2.0f * y3 / refH;

	//計算結果で頂点バッファオブジェクトを更新する
	HRESULT hr{ S_OK };
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	hr = immediateContext->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	Vertex* vertices{ reinterpret_cast<Vertex*>(mapped_subresource.pData) };
	if (vertices != nullptr)
	{
		vertices[0].position = { x0, y0, 0, 1 };
		vertices[1].position = { x1, y1, 0, 1 };
		vertices[2].position = { x2, y2, 0, 1 };
		vertices[3].position = { x3, y3, 0, 1 };
		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = color;

		const auto& texture2d = material.GetTexture<Texture2D>("color_map");
		if (!texture2d.expired())
		{
			const D3D11_TEXTURE2D_DESC& texture2dDesc = texture2d.lock()->GetDesc();
			vertices[0].texcoord = { tx0 / texture2dDesc.Width,ty0 / texture2dDesc.Height };
			vertices[1].texcoord = { tx1 / texture2dDesc.Width,ty1 / texture2dDesc.Height };
			vertices[2].texcoord = { tx2 / texture2dDesc.Width,ty2 / texture2dDesc.Height };
			vertices[3].texcoord = { tx3 / texture2dDesc.Width,ty3 / texture2dDesc.Height };
		}
	}
	immediateContext->Unmap(vertexBuffer.Get(), 0);

	//頂点バッファのバインド
	UINT stride{ sizeof(Vertex) };
	UINT offset{ 0 };
	immediateContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

	//プリミティブタイプおよびデータの順序に関する情報のバインド
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//入力レイアウトオブジェクトのバインド
	//immediateContext->IASetInputLayout(inputLayout.Get());

	//シェーダーリソースのバインド
	//immediateContext->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());

	//シェーダーのバインド
	//immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
	//immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);
	material.Apply(rtx);

	//マスク
	if (enableMask)
	{
		/*immediateContext->PSSetShader(maskPixelShader.Get(), nullptr, 0);
		immediateContext->PSSetShaderResources(1, 1, maskTexture.GetAddressOf());*/
		maskMaterial.Apply(rtx);
	}

	//プリミティブの描画
	immediateContext->Draw(4, 0);
}

void Image::DrawProperty()
{
#ifdef USE_IMGUI
	
	ImGui::ColorEdit4("Color", &color.r);

	ImGui::DragFloat2("UV", &uv.x, 0.1f);

	ImGui::Separator();

	ImGui::Checkbox("EnableMask", &enableMask);

	// Base
	Graphic::DrawProperty();

	// Material
	material.DrawProperty();

	//マテリアルのテクスチャが変更されていたら初期化し直す
	if (material.IsTextureChanged())
	{
		Initialize();
		// フラグをクリア
		material.ClearTextureChangedFlag();
	}


	if (enableMask)
	{
		maskMaterial.DrawProperty();
	}

#endif // USE_IMGUI
}

json Image::Serialize() const
{
	json j = Graphic::Serialize();
	j["sx"] = sx;
	j["sy"] = sy;
	j["sw"] = sw;
	j["sh"] = sh;
	j["uv"] = { uv.x, uv.y };
	j["enableMask"] = enableMask;

#if 1
	// マテリアル
	j["material"] = material.Serialize();
	// マスクマテリアル
	j["maskMaterial"] = maskMaterial.Serialize();
#endif // 0

	return j;
}

void Image::Deserialize(const json& j)
{
	sx = j.value("sx", 0.0f);
	sy = j.value("sy", 0.0f);
	sw = j.value("sw", 100.0f);
	sh = j.value("sh", 100.0f);
	if (j.contains("uv") && j["uv"].is_array() && j["uv"].size() == 2) {
		uv.x = j["uv"][0].get<float>();
		uv.y = j["uv"][1].get<float>();
	}
	enableMask = j.value("enableMask", false);
#if 1
	// マテリアル
	if (j.contains("material")) {
		material.Deserialize(j["material"]);
	}
	// マスクマテリアル
	if (j.contains("maskMaterial")) {
		maskMaterial.Deserialize(j["maskMaterial"]);
	}
#endif // 0

}