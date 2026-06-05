#include "pch.h"
#include "ShadowMapPass.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include <profiler.h>
#include "Engine/Rendering/Renderers/GltfModelRenderer.h"

CONST LONG SHADOWMAP_WIDTH{ 2048 };
CONST LONG SHADOWMAP_HEIGHT{ 2048 };

void ShadowMapPass::Initialize()
{
	// シャドウマップ用のリソースの初期化
	cascadedShadowMaps = std::make_unique<CascadedShadowMaps>(Graphics::GetDevice(), SHADOWMAP_WIDTH * 4, SHADOWMAP_HEIGHT * 4);
}

void ShadowMapPass::Execute(RenderContext* rtx, Scene* scene)
{
	auto immediateContext = rtx->immediateContext;
	auto renderState = rtx->renderState;

    //シーン全体のキャプチャ終了
    //auto frameBuffer = static_cast<FrameBuffer*>(rtx->GetSharedResource("FrameBuffer"));
    //frameBuffer->Deactivate(immediateContext);

    
	// シャドウマップのレンダーターゲットをアクティブ化
    cascadedShadowMaps->Clear(immediateContext);
    cascadedShadowMaps->Activate(immediateContext, rtx->view, rtx->projection, rtx->lightDirection, criticalDepthValue, 3/*cbSlot*/);
    
	renderState->BindDepthStencilState(immediateContext, DepthStencilState::TestAndWrite, 0);
    renderState->BindRasterizerState(immediateContext, RasterizerState::SolidCullNone);
    renderState->BindBlendState(immediateContext, BlendState::Opaque);


    // フレームバッファのシェーダーリソースビューをRenderContextに共有リソースとして設定
    //rtx->SetSharedResource("OpaquePass_ColorMap", frameBuffer->shader_resource_views[0].Get());
    //rtx->SetSharedResource("OpaquePass_DepthMap", frameBuffer->shader_resource_views[1].Get());
	
	// ここまでPreRenderPassで描画した内容をシャドウマップ適用パスで使用するため、PreRenderPassのレンダーテクスチャのカラーバッファと深度バッファを共有リソースとしてRenderContextに設定
	auto preRenderTexture = static_cast<RenderTexture*>(rtx->GetSharedResource("PreRenderTexture"));
    rtx->SetSharedResource("OpaquePass_ColorTexture", preRenderTexture->GetColorTexture());
    rtx->SetSharedResource("OpaquePass_DepthTexture", preRenderTexture->GetDepthTexture());


    //シャドウマップ生成描画
    {
        ProfileScopedSection_2(0, "CastShadows", ImGuiControl::Profiler::Red);
        for (const std::shared_ptr<GameObject>& object : scene->objectManager->GetAll())
        {
            if (GltfModelRenderer* renderer = object->GetComponent<GltfModelRenderer>())
            {
                if (renderer->IsEnabled() && renderer->IsEnableShadow())
                {
                    renderer->CastShadow(rtx);
                }
                if (renderer->IsEnableOnlyShadow())
                {
                    renderer->CastShadow(rtx);
                }
            }
        }
    }
    //cascadedShadowMaps->Deactivate(immediateContext);

	// シャドウマップ生成後、デフォルトのレンダーターゲットに切り替える
    rtx->SetDefaultRenderTarget();

	// シャドウマップのシェーダーリソースビューをRenderContextに共有リソースとして設定
	rtx->SetSharedResource("ShadowMapPass_DepthTexture", cascadedShadowMaps->GetDepthTexture());

}