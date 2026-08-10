#include "pch.h"
#include "PFX_CrtPass.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Resources/ResourceManager.h"

void PfxCrtPass::Initialize()
{
	// レンダーターゲットの初期化
	auto device = Graphics::GetDevice();
	m_pfxCrtTexture.Create(device, 1920, 1080);
	RegisterResizableRenderTexture(&m_pfxCrtTexture); // リサイズが必要なレンダーターゲットとして登録(※これを呼び出さないと、ウィンドウサイズ変更時にリサイズされないので注意)

	// ポストプロセスマテリアルの初期化
	m_pfxCrtMaterial = std::make_shared<Material>();
	m_pfxCrtMaterial->SetShader(device, ResourceManager::LoadShader<PixelShader>("PFX_CrtPassPS")); // ここで使用するシェーダーを指定
	//std::vector<std::string> notBindCBufferNames = {
	//	"SCENE_CONSTANT_BUFFER",
	//};
	//m_pfxCrtMaterial->SetNotBindCBuffer(notBindCBufferNames); //NOTIFY: デフォルト設定を実装したので基本的にこの処理が不要になった。
}

void PfxCrtPass::Finalize()
{
	// レンダーターゲットの終了化処理
	m_pfxCrtTexture.Release();
}

void PfxCrtPass::Execute(RenderContext* rtx, Scene* scene)
{
	auto renderTexture = static_cast<RenderTexture*>(rtx->GetSharedResource("PFX_OutLinePass_RenderTexture"));
	if (renderTexture)
	{
		TracyD3D11Zone(Graphics::GetTracyD3D11Ctx(), "PfxCrtPass::Execute");
		// レンダーターゲットをセット
		rtx->SetRenderTarget(m_pfxCrtTexture);
		rtx->ClearCurrentRenderTarget(Color::Black);

		// この部分で、renderTextureを使用してポストプロセスエフェクトを適用する処理を実装します。
		RawTexture2D* colorTexture = renderTexture->GetColorTexture();
		auto depthTexture = static_cast<RawTexture2D*>(rtx->GetSharedResource("OpaquePass_DepthTexture"));
		std::shared_ptr<RawTexture2D> sharedColorTexture = std::make_shared<RawTexture2D>(*colorTexture);
		std::shared_ptr<RawTexture2D> sharedDepthTexture = std::make_shared<RawTexture2D>(*depthTexture);
		// 複数テクスチャ渡したいときは、↑の処理を各テクスチャに対して行う。

		// TODO_MARUMO: 第一引数ににhlsl側の名前入れる。(一応配列禁止)
		m_pfxCrtMaterial->SetTexture("textureMap", sharedColorTexture);
		m_pfxCrtMaterial->SetTexture("depthMap", sharedDepthTexture);

		//int testValue = 42; // 例として整数値を設定
		//m_postProcessMaterial->SetValue<int>("testValue", testValue); // こんな感じで変数も渡せる

		// ポストプロセスマテリアルを使用して全画面クワッドを描画
		rtx->DrawFullScreenQuad(m_pfxCrtMaterial.get());

		// SharedResourceにポストプロセス後のテクスチャを保存して、次のパスで使用できるようにする
		rtx->SetSharedResource("PostProcessPass_RenderTexture", &m_pfxCrtTexture);
	}
}

void PfxCrtPass::DrawProperty()
{
#ifdef USE_IMGUI
	// ポストプロセスパスのプロパティ描画処理
	ImGui::Text("PfxCrtProperties");
	// ここにImGuiを使ったプロパティ描画コードを追加します。

	ImGui::Image(m_pfxCrtTexture.GetColorBuffer(), ImVec2(256, 144)); // ポストプロセス後のテクスチャを表示


#endif // USE_IMGUI

}
