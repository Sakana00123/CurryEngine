#include "pch.h"
#include "PostProcessPass.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Resources/ResourceManager.h"


void PostProcessPass::Initialize()
{
	// レンダーターゲットの初期化
	auto device = Graphics::GetDevice();
	m_postProcessTexture.Create(device, 1920, 1080);
	RegisterResizableRenderTexture(&m_postProcessTexture); // リサイズが必要なレンダーターゲットとして登録(※これを呼び出さないと、ウィンドウサイズ変更時にリサイズされないので注意)

	// ポストプロセスマテリアルの初期化
	m_postProcessMaterial = std::make_shared<Material>();
	m_postProcessMaterial->SetShader(device, ResourceManager::LoadShader<PixelShader>("FinalPassPS")); // ここで使用するシェーダーを指定
}

void PostProcessPass::Finalize()
{
	// レンダーターゲットの終了化処理
	m_postProcessTexture.Release();
}

void PostProcessPass::Execute(RenderContext* rtx, Scene* scene)
{
	auto renderTexture = static_cast<RenderTexture*>(rtx->GetSharedResource("ShadowApplyPass_RenderTexture"));
	if (renderTexture)
	{
		// レンダーターゲットをセット
		rtx->SetRenderTarget(m_postProcessTexture);
		rtx->ClearCurrentRenderTarget(Color::Black);

		// この部分で、renderTextureを使用してポストプロセスエフェクトを適用する処理を実装します。
		RawTexture2D* colorTexture = renderTexture->GetColorTexture();
		std::shared_ptr<RawTexture2D> sharedColorTexture = std::make_shared<RawTexture2D>(*colorTexture);
		// 複数テクスチャ渡したいときは、↑の処理を各テクスチャに対して行う。

		// TODO_MARUMO: 第一引数ににhlsl側の名前入れる。(一応配列禁止)
		m_postProcessMaterial->SetTexture("textureMap", sharedColorTexture);

		//int testValue = 42; // 例として整数値を設定
		//m_postProcessMaterial->SetValue<int>("testValue", testValue); // こんな感じで変数も渡せる

		// ポストプロセスマテリアルを使用して全画面クワッドを描画
		rtx->DrawFullScreenQuad(m_postProcessMaterial.get());

		// SharedResourceにポストプロセス後のテクスチャを保存して、次のパスで使用できるようにする
		rtx->SetSharedResource("PostProcessPass_RenderTexture", &m_postProcessTexture);
	}
}

void PostProcessPass::DrawProperty()
{
#ifdef USE_IMGUI
	// ポストプロセスパスのプロパティ描画処理
	ImGui::Text("PostProcessPass Properties");
	// ここにImGuiを使ったプロパティ描画コードを追加します。 
	
	ImGui::Image(m_postProcessTexture.GetColorBuffer(), ImVec2(256, 144)); // ポストプロセス後のテクスチャを表示


#endif // USE_IMGUI

}