#include "pch.h"
#include "FinalPass.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include <Engine\Core\EnginePaths.h>

void FinalPass::Initialize()
{
	// FinalPassの初期化処理
	auto device = Graphics::GetDevice();
	HRESULT hr{ S_OK };
	// 最終合成用のピクセルシェーダーを作成
	std::string dir = EnginePaths::ShadersDataDir;
	hr = CreatePixelShaderFromCSO(device, (dir + "FinalPassPS.cso").c_str(), finalPassPs.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void FinalPass::Execute(RenderContext* rtx, Scene* scene)
{
	TracyD3D11Zone(Graphics::GetTracyD3D11Ctx(), "FinalPass::Execute");
	auto immediateContext = rtx->immediateContext;
	auto renderState = rtx->renderState;
	// 最終描画のレンダーターゲットをアクティブ化
	rtx->SetDefaultRenderTarget();

	// シャドウマップを合成したレンダーターゲットを取得
	auto renderTarget = static_cast<RenderTexture*>(rtx->GetSharedResource("PostProcessPass_RenderTexture"));


#ifndef _DEBUG
	// 最終描画(Releaseビルド用の全画面描画コード。Debugビルドでは、ImGuiで表示するため、全画面描画は行わない)
	// シャドウマップを合成したレンダーターゲットをシェーダーリソースビューとして使用して最終描画
	ID3D11ShaderResourceView* shaderResourceViews[] = {
		renderTarget->GetColorBuffer()
	};
	Graphics::fullScreenQuad->Draw(immediateContext, shaderResourceViews, 0, _countof(shaderResourceViews), finalPassPs.Get());
#else
	// Debugビルドでは、ImGuiで表示するため、全画面描画は行わない

	rtx->SetSharedResource("FinalPass_RenderTexture", renderTarget);


#endif // !_DEBUG

}
