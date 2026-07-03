#include "pch.h"
#include "RenderTexture.h"
#include "Engine/Core/Misc.h"

void RenderTexture::Create(ID3D11Device* device, UINT width, UINT height, bool withDepthStencil)
{
	// 引数の検査
	_ASSERT_EXPR(device != nullptr, L"ID3D11Device is null.");
	_ASSERT_EXPR(width > 0 && height > 0, L"Invalid render target size.");

	// ビューポートの設定
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	m_viewport.Width = static_cast<FLOAT>(width);
	m_viewport.Height = static_cast<FLOAT>(height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	// レンダーターゲット用のテクスチャとビューの作成
	HRESULT hr{ S_OK };
	Microsoft::WRL::ComPtr<ID3D11Texture2D> renderTexture;
	// テクスチャの作成
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	hr = device->CreateTexture2D(&textureDesc, nullptr, renderTexture.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	// レンダーターゲットビューの作成
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	hr = device->CreateRenderTargetView(renderTexture.Get(), &rtvDesc, m_rtv.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	// シェーダーリソースビューの作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> colorBuffer;
	hr = device->CreateShaderResourceView(renderTexture.Get(), &srvDesc, colorBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
	// RawTexture2D の作成
	m_colorTexture = std::make_unique<RawTexture2D>(colorBuffer.Get(), textureDesc);

	// 深度ステンシルバッファの作成
	if (withDepthStencil)
	{
		// 深度ステンシルビューの作成
		D3D11_TEXTURE2D_DESC depthDesc{};
		depthDesc.Width = width;
		depthDesc.Height = height;
		depthDesc.MipLevels = 1;
		depthDesc.ArraySize = 1;
		depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		depthDesc.SampleDesc.Count = 1;
		depthDesc.SampleDesc.Quality = 0;
		depthDesc.Usage = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		depthDesc.CPUAccessFlags = 0;
		depthDesc.MiscFlags = 0;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture;
		hr = device->CreateTexture2D(&depthDesc, nullptr, depthTexture.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 注意: 深度ステンシルビューのフォーマットは、テクスチャのフォーマットと互換性が必要
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = 0;
		hr = device->CreateDepthStencilView(depthTexture.Get(), &dsvDesc, m_dsv.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
		D3D11_SHADER_RESOURCE_VIEW_DESC depthSrvDesc{};
		depthSrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		depthSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		depthSrvDesc.Texture2D.MipLevels = 1;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> depthBuffer;
		hr = device->CreateShaderResourceView(depthTexture.Get(), &depthSrvDesc, depthBuffer.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
		// RawTexture2D の作成
		m_depthTexture = std::make_unique<RawTexture2D>(depthBuffer.Get(), depthDesc);
	}
}

void RenderTexture::Release()
{
	m_rtv.Reset();
	m_dsv.Reset();
	m_colorTexture.reset();
	m_depthTexture.reset();
}

void RenderTexture::Resize(ID3D11Device* device, UINT width, UINT height)
{
	bool hasDepthStencil = m_dsv != nullptr;
	Release();
	Create(device, width, height, hasDepthStencil);
}

uint32_t RenderTexture::GetWidth() const
{
	return static_cast<uint32_t>(m_viewport.Width);
}

uint32_t RenderTexture::GetHeight() const
{
	return static_cast<uint32_t>(m_viewport.Height);
}

ID3D11RenderTargetView* RenderTexture::GetRenderTargetView() const
{
	return m_rtv.Get();
}

ID3D11DepthStencilView* RenderTexture::GetDepthStencilView() const
{
	return m_dsv.Get();
}

ID3D11ShaderResourceView* RenderTexture::GetSRV(TextureSemantic semantic) const
{
	switch (semantic)
	{
	case TextureSemantic::Default:
		return GetColorBuffer();
	case TextureSemantic::Depth:
		return GetDepthBuffer();
	default:
		return nullptr;
	};
}

ID3D11ShaderResourceView** RenderTexture::GetSRVAddress(TextureSemantic semantic)
{
	switch (semantic)
	{
	case TextureSemantic::Default:
		return m_colorTexture->GetSRVAddress();
	case TextureSemantic::Depth:
		return m_depthTexture->GetSRVAddress();
	default:
		return nullptr;
	};
}

ID3D11ShaderResourceView* RenderTexture::GetColorBuffer() const
{
	return m_colorTexture ? m_colorTexture->GetSRV() : nullptr;
}

ID3D11ShaderResourceView* RenderTexture::GetDepthBuffer() const
{
	return m_depthTexture ? m_depthTexture->GetSRV() : nullptr;
}

RawTexture2D* RenderTexture::GetColorTexture() const
{
	return m_colorTexture ? m_colorTexture.get() : nullptr;
}

RawTexture2D* RenderTexture::GetDepthTexture() const
{
	return m_depthTexture ? m_depthTexture.get() : nullptr;
}

D3D11_VIEWPORT RenderTexture::GetViewport() const
{
	return m_viewport;
}

void RenderTexture::Clear(ID3D11DeviceContext* context, const Color& color) const
{
	context->ClearRenderTargetView(m_rtv.Get(), &color.r);
	if (m_dsv)
	{
		context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
}