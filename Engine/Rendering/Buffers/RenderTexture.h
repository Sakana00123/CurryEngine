#pragma once
#include <d3d11_1.h>
#include <wrl.h>
#include "Engine/Core/Color.h"
#include "Engine/Resources/Texture.h"

struct RenderTexture : public Texture
{
public:
	RenderTexture() = default;
	~RenderTexture() = default;

	// レンダーターゲットの作成
	void Create(ID3D11Device* device, UINT width, UINT height, bool withDepthStencil = true);
	// リソースの解放(すべてのComPtrをリセット)
	void Release();

	// リサイズ
	void Resize(ID3D11Device* device, UINT width, UINT height);

	// 幅を取得
	uint32_t GetWidth() const;
	// 高さを取得
	uint32_t GetHeight() const;

	// シェーダーリソースビューを取得（セマンティクスに応じた SRV を返すことができる。DefaultとDepthに対応）
	ID3D11ShaderResourceView* GetSRV(TextureSemantic semantic = TextureSemantic::Default) const override;
	// シェーダーリソースビューのアドレスを取得（API 呼び出し用。セマンティクスに応じた SRV アドレスを返すことができる。DefaultとDepthに対応）
	ID3D11ShaderResourceView** GetSRVAddress(TextureSemantic semantic = TextureSemantic::Default) override;

	// テクスチャの次元を取得
	TextureDimension GetDimension() const override { return TextureDimension::Texture2D; }

	// シェーダーリソースビューを取得
	ID3D11ShaderResourceView* GetColorBuffer() const;

	// 深度バッファのシェーダーリソースビューを取得
	ID3D11ShaderResourceView* GetDepthBuffer() const;

	// 色テクスチャを取得
	RawTexture2D* GetColorTexture() const;

	// 深度テクスチャを取得
	RawTexture2D* GetDepthTexture() const;


	// レンダーターゲットビューを取得
	ID3D11RenderTargetView* GetRenderTargetView() const;

	// 深度ステンシルビューを取得
	ID3D11DepthStencilView* GetDepthStencilView() const;
	// ビューポートを取得
	D3D11_VIEWPORT GetViewport() const;

	// クリア
	void Clear(ID3D11DeviceContext* context, const Color& color) const;
private:
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;
	std::unique_ptr<RawTexture2D> m_colorTexture;
	std::unique_ptr<RawTexture2D> m_depthTexture;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_colorBuffer;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_depthBuffer;
	D3D11_VIEWPORT m_viewport{};
};
