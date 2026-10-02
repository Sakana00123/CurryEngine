#pragma once
#include <d3d11_1.h>
#include <wrl.h>
#include "Engine/Core/Color.h"
#include "Engine/Resources/Texture.h"

/** @brief RenderTexture を表す構造体です。 */
struct RenderTexture : public Texture
{
public:
	/**
	 * @brief RenderTexture を構築します。
	 */
	RenderTexture() = default;
	/**
	 * @brief RenderTexture を破棄します。
	 */
	~RenderTexture() = default;

	// レンダーターゲットの作成
	/**
	 * @brief 新しい要素を生成します。
	 */
	void Create(ID3D11Device* device, UINT width, UINT height, bool withDepthStencil = true);
	// リソースの解放(すべてのComPtrをリセット)
	/**
	 * @brief Release の処理を行います。
	 */
	void Release();

	// リサイズ
	/**
	 * @brief Resize の処理を行います。
	 */
	void Resize(ID3D11Device* device, UINT width, UINT height);

	// 幅を取得
	/**
	 * @brief GetWidth に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	uint32_t GetWidth() const;
	// 高さを取得
	/**
	 * @brief GetHeight に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	uint32_t GetHeight() const;

	// シェーダーリソースビューを取得（セマンティクスに応じた SRV を返すことができる。DefaultとDepthに対応）
	/**
	 * @brief GetSRV に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	ID3D11ShaderResourceView* GetSRV(TextureSemantic semantic = TextureSemantic::Default) const override;
	// シェーダーリソースビューのアドレスを取得（API 呼び出し用。セマンティクスに応じた SRV アドレスを返すことができる。DefaultとDepthに対応）
	/**
	 * @brief GetSRVAddress に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	ID3D11ShaderResourceView** GetSRVAddress(TextureSemantic semantic = TextureSemantic::Default) override;

	// テクスチャの次元を取得
	TextureDimension GetDimension() const override { return TextureDimension::Texture2D; }

	// シェーダーリソースビューを取得
	/**
	 * @brief GetColorBuffer に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	ID3D11ShaderResourceView* GetColorBuffer() const;

	// 深度バッファのシェーダーリソースビューを取得
	/**
	 * @brief GetDepthBuffer に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	ID3D11ShaderResourceView* GetDepthBuffer() const;

	// 色テクスチャを取得
	/**
	 * @brief GetColorTexture に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	RawTexture2D* GetColorTexture() const;

	// 深度テクスチャを取得
	/**
	 * @brief GetDepthTexture に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	RawTexture2D* GetDepthTexture() const;


	// レンダーターゲットビューを取得
	/**
	 * @brief GetRenderTargetView に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	ID3D11RenderTargetView* GetRenderTargetView() const;

	// 深度ステンシルビューを取得
	/**
	 * @brief GetDepthStencilView に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	ID3D11DepthStencilView* GetDepthStencilView() const;
	// ビューポートを取得
	/**
	 * @brief GetViewport に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	D3D11_VIEWPORT GetViewport() const;

	// クリア
	/**
	 * @brief 保持している内容を消去します。
	 */
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
