#include "pch.h"
#include "PFX_OutLinePass.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Resources/ResourceManager.h"


void PFX_OutLinePass::Initialize()
{
	// レンダーターゲットの初期化
	auto device = Graphics::GetDevice();
	m_renderTexture.Create(device, 1920, 1080);
	RegisterResizableRenderTexture(&m_renderTexture); // リサイズが必要なレンダーターゲットとして登録(※これを呼び出さないと、ウィンドウサイズ変更時にリサイズされないので注意)

	// ポストプロセスマテリアルの初期化
	m_material = std::make_shared<Material>();
	m_material->SetShader(device, ResourceManager::LoadShader<PixelShader>("PFX_OutLinePS")); // ここで使用するシェーダーを指定
}

void PFX_OutLinePass::Finalize()
{
	// レンダーターゲットの終了化処理
	m_renderTexture.Release();
}

void PFX_OutLinePass::Execute(RenderContext* rtx, Scene* scene)
{
	auto renderTexture = static_cast<RenderTexture*>(rtx->GetSharedResource("ShadowApplyPass_RenderTexture"));
	if (renderTexture)
	{
		// レンダーターゲットをセット
		rtx->SetRenderTarget(m_renderTexture);
		rtx->ClearCurrentRenderTarget(Color::Black);

		// この部分で、renderTextureを使用してポストプロセスエフェクトを適用する処理を実装します。
		RawTexture2D* colorTexture = renderTexture->GetColorTexture();
		std::shared_ptr<RawTexture2D> sharedColorTexture = std::make_shared<RawTexture2D>(*colorTexture);
		// 複数テクスチャ渡したいときは、↑の処理を各テクスチャに対して行う。
		auto renderTexture = static_cast<RenderTexture*>(rtx->GetSharedResource("PFX_OutLinePass_RenderTexture"));
		// 第一引数ににhlsl側の名前入れる。(一応配列禁止)
		m_material->SetTexture("textureMap", sharedColorTexture);
		// この部分で、renderTextureを使用してポストプロセスエフェクトを適用する処理を実装します。
		auto depthTexture = static_cast<RawTexture2D*>(rtx->GetSharedResource("OpaquePass_DepthTexture"));
		std::shared_ptr<RawTexture2D> sharedDepthTexture = std::make_shared<RawTexture2D>(*depthTexture);
		m_material->SetTexture("depthMap", sharedDepthTexture);
		// 複数テクスチャ渡したいときは、↑の処理を各テクスチャに対して行う。

		// TODO_MARUMO: 第一引数ににhlsl側の名前入れる。(一応配列禁止)

		//int testValue = 42; // 例として整数値を設定
		//m_material->SetValue<int>("testValue", testValue); // こんな感じで変数も渡せる

		// ポストプロセスマテリアルを使用して全画面クワッドを描画
		rtx->DrawFullScreenQuad(m_material.get());

		// SharedResourceにポストプロセス後のテクスチャを保存して、次のパスで使用できるようにする
		rtx->SetSharedResource("PFX_OutLinePass_RenderTexture", &m_renderTexture);
	}
}

void PFX_OutLinePass::DrawProperty()
{
#ifdef USE_IMGUI
	// ポストプロセスパスのプロパティ描画処理
	ImGui::Text("PFX_OutLinePass Properties");
	// ここにImGuiを使ったプロパティ描画コードを追加します。 

	ImGui::Image(m_renderTexture.GetColorBuffer(), ImVec2(256, 144)); // 描画結果を表示


#endif // USE_IMGUI

}