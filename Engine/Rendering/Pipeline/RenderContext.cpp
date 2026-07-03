#include "pch.h"
#include "RenderContext.h"
#include "Graphics.h"

RenderContext::RenderContext(ID3D11DeviceContext* context, FullScreenQuad* fullScreenQuad, std::unordered_map<std::string, void*> sharedResources)
	: immediateContext(context), m_context(context), fullScreenQuad(fullScreenQuad), sharedResources(std::move(sharedResources))
{
	// 初期状態ではデフォルトのレンダーターゲットを設定
	SetDefaultRenderTarget();
}


void RenderContext::SetSharedResource(const std::string& key, void* resource)
{
	sharedResources[key] = resource;
}

void* RenderContext::GetSharedResource(const std::string& key) const
{
	auto it = sharedResources.find(key);
	if (it != sharedResources.end())
	{
		return it->second;
	}
	return nullptr; // キーが見つからない場合は nullptr を返す
}

void RenderContext::SetRenderTarget(const RenderTexture& target)
{
	// シェーダーリソースビューをすべて解除
	UnbindSRVs();

	// レンダーターゲットを設定
	ID3D11RenderTargetView* rtv = target.GetRenderTargetView();
	ID3D11DepthStencilView* dsv = target.GetDepthStencilView();
	m_context->OMSetRenderTargets(1, &rtv, dsv);
	m_currentRenderTarget = &target;

	// ビューポートも更新
	D3D11_VIEWPORT viewport = target.GetViewport();
	m_context->RSSetViewports(1, &viewport);

}

void RenderContext::SetDefaultRenderTarget()
{
	// シェーダーリソースビューをすべて解除
	UnbindSRVs();
	// デフォルトのレンダーターゲットを設定
	ID3D11RenderTargetView* defaultRTV = Graphics::GetDefaultRenderTargetView();
	ID3D11DepthStencilView* defaultDSV = Graphics::GetDefaultDepthStencilView();
	D3D11_VIEWPORT defaultViewport = Graphics::GetDefaultViewport();
	m_context->OMSetRenderTargets(1, &defaultRTV, defaultDSV);
	m_currentRenderTarget = nullptr; // デフォルトのレンダーターゲットは特定の RenderTarget オブジェクトに対応しないため nullptr を設定
	m_context->RSSetViewports(1, &defaultViewport);
}

void RenderContext::ClearCurrentRenderTarget(const Color& color) const
{
	// シェーダーリソースビューをすべて解除
	if (m_currentRenderTarget)
	{
		// 現在のレンダーターゲットをクリア
		m_currentRenderTarget->Clear(m_context, color);
	}
	else
	{
		// デフォルトのレンダーターゲットをクリア
		ID3D11RenderTargetView* defaultRTV = Graphics::GetDefaultRenderTargetView();
		ID3D11DepthStencilView* defaultDSV = Graphics::GetDefaultDepthStencilView();
		m_context->ClearRenderTargetView(defaultRTV, &color.r);
		m_context->ClearDepthStencilView(defaultDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
}

void RenderContext::DrawFullScreenQuad(ID3D11ShaderResourceView** shaderResourceViews, uint32_t startSlot, uint32_t numViews, ID3D11PixelShader* replacedPixelShader)
{
	// シェーダーリソースビューを設定
	fullScreenQuad->Draw(m_context, shaderResourceViews, startSlot, numViews, replacedPixelShader);
}

void RenderContext::DrawFullScreenQuad(Material* material)
{
	// シェーダーリソースビューを設定
	fullScreenQuad->Render(this, material);
}

void RenderContext::UnbindSRVs() const
{
	// シェーダーリソースビューをすべて解除
	ID3D11ShaderResourceView* nullSRV[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = { nullptr };
	m_context->PSSetShaderResources(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE, nullSRV);
	m_context->VSSetShaderResources(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE, nullSRV);
}