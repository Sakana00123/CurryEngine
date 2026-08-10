#include "pch.h"
#include "ShadowApplyPass.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Resources/ResourceManager.h"

void ShadowApplyPass::Initialize()
{
	// シャドウマップ適用パスの初期化やリソースの準備を行う
	auto device = Graphics::GetDevice();
	//HRESULT hr{ S_OK };
	//hr = CreatePixelShaderFromCSO(device, "./Data/Shaders/CascadedShadowPS.cso", cascadedShadowPs.GetAddressOf());
	//_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	m_cascadedShadowMaterial = std::make_shared<Material>();
	m_cascadedShadowMaterial->SetShader(device, ResourceManager::GetShader<PixelShader>("CascadedShadowPS"));
	std::vector<std::string> notBindCBufferNames = { "SCENE_CONSTANT_BUFFER", "LIGHT_CONSTANT_BUFFER", /*"PARAMETRIC_CONSTANT_BUFFER", */"CASCADED_CONSTANTS"};
	m_cascadedShadowMaterial->SetNotBindCBuffer(notBindCBufferNames);
	// 初期値設定
	m_cascadedShadowMaterial->SetValue("colorizeCascadedLayer", false);
	m_cascadedShadowMaterial->SetValue("shadowDepthBias", 0.000021f);

	m_shadowRenderTexture.Create(device, 1920, 1080);
	RegisterResizableRenderTexture(&m_shadowRenderTexture); // リサイズが必要なレンダーターゲットとして登録(※これを呼び出さないと、ウィンドウサイズ変更時にリサイズされないので注意)
}

void ShadowApplyPass::Finalize()
{
	// シャドウマップ適用パスのリソースの解放やクリーンアップを行う
	//cascadedShadowPs.Reset();
	m_shadowRenderTexture.Release();
}

void ShadowApplyPass::Execute(RenderContext* rtx, Scene* scene)
{
	ZoneScopedN("ShadowApplyPass::Execute");

	// シャドウマップ適用パスの実行コードを記述する
	auto immediateContext = rtx->immediateContext;
	auto renderState = rtx->renderState;

	// シャドウマップを合成するためのレンダーターゲットをアクティブ化
	rtx->SetRenderTarget(m_shadowRenderTexture);
	rtx->ClearCurrentRenderTarget(Color::Black);

	// シャドウマップを合成するためのレンダーターゲットを共有リソースとしてRenderContextに設定
	rtx->SetSharedResource("ShadowApplyPass_RenderTexture", &m_shadowRenderTexture);
	
	
#if 0

	ID3D11ShaderResourceView* shaderResourceViews[] = {
		static_cast<ID3D11ShaderResourceView*>(rtx->GetSharedResource("OpaquePass_ColorMap")),
		static_cast<ID3D11ShaderResourceView*>(rtx->GetSharedResource("OpaquePass_DepthMap")),
		static_cast<ID3D11ShaderResourceView*>(rtx->GetSharedResource("ShadowMapPass_DepthMap")),
	};

	// シャドウを合成したものを描画
	rtx->DrawFullScreenQuad(shaderResourceViews, 0, 3, cascadedShadowPs.Get());
#else
	RawTexture2D* pColorMapTexture = static_cast<RawTexture2D*>(rtx->GetSharedResource("OpaquePass_ColorTexture"));
	RawTexture2D* pDepthMapTexture = static_cast<RawTexture2D*>(rtx->GetSharedResource("OpaquePass_DepthTexture"));
	RawTexture2D* pShadowMapTexture = static_cast<RawTexture2D*>(rtx->GetSharedResource("ShadowMapPass_DepthTexture"));
	std::shared_ptr<RawTexture2D> colorMapTexture = std::make_shared<RawTexture2D>(pColorMapTexture->GetSRV(), pColorMapTexture->GetDesc());
	std::shared_ptr<RawTexture2D> depthMapTexture = std::make_shared<RawTexture2D>(pDepthMapTexture->GetSRV(), pDepthMapTexture->GetDesc());
	std::shared_ptr<RawTexture2D> shadowMapTexture = std::make_shared<RawTexture2D>(pShadowMapTexture->GetSRV(), pShadowMapTexture->GetDesc());
	m_cascadedShadowMaterial->SetTexture("colorMap", colorMapTexture);
	m_cascadedShadowMaterial->SetTexture("depthMap", depthMapTexture);
	m_cascadedShadowMaterial->SetTexture("cascadedShadowMaps", shadowMapTexture);
	rtx->DrawFullScreenQuad(m_cascadedShadowMaterial.get());
#endif // 0

}

//void ShadowApplyPass::Resize(UINT width, UINT height)
//{
//	// シャドウマップ適用パスのリサイズ処理を記述する
//	m_shadowRenderTexture.Resize(Graphics::GetDevice(), width, height);
//}

void ShadowApplyPass::DrawProperty()
{
#ifdef USE_IMGUI
	// シャドウマップ適用パスのプロパティ描画コードを記述する
	ImGui::SeparatorText("ShadowApplyPass Properties");

	m_cascadedShadowMaterial->DrawProperty();


#endif // USE_IMGUI

}
