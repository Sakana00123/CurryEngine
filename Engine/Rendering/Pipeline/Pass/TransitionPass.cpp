#include "pch.h"
#include "TransitionPass.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Resources/ResourceManager.h"


void TransitionPass::Initialize()
{
	// レンダーターゲットの初期化
	auto device = Graphics::GetDevice();
	m_renderTexture.Create(device, 1920, 1080);

	// ポストプロセスマテリアルの初期化
	m_material = std::make_shared<Material>();
	m_material->SetShader(device, ResourceManager::Load<PixelShader>("./Shader/TransitionPS.hlsl")); // ここで使用するシェーダーを指定
}

void TransitionPass::Finalize()
{
	// レンダーターゲットの終了化処理
	m_renderTexture.Release();
}

void TransitionPass::Execute(RenderContext* rtx, Scene* scene)
{
	auto renderTexture = static_cast<RenderTexture*>(rtx->GetSharedResource("PostProcessPass_RenderTexture"));
	if (renderTexture)
	{
		// レンダーターゲットをセット
		rtx->SetRenderTarget(m_renderTexture);
		rtx->ClearCurrentRenderTarget(Color::Black);

		// この部分で、renderTextureを使用してポストプロセスエフェクトを適用する処理を実装します。
		RawTexture2D* colorTexture = renderTexture->GetColorTexture();
		std::shared_ptr<RawTexture2D> sharedColorTexture = std::make_shared<RawTexture2D>(*colorTexture);
		// 複数テクスチャ渡したいときは、↑の処理を各テクスチャに対して行う。

		// 第一引数ににhlsl側の名前入れる。(一応配列禁止)
		m_material->SetTexture("textureMap", sharedColorTexture);

		// SharedResourceから値を受け取ることもできる。例えば、遷移の進行度やフェードイン/アウトの切り替えなど。
		if (void* progressPtr = rtx->GetSharedResource("SceneTransitionFadeValue"))
			transitionProgress = *static_cast<float*>(progressPtr);
		if (void* fadeInPtr = rtx->GetSharedResource("SceneTransitionIsFading"))
			isFadeIn = static_cast<int>(*static_cast<bool*>(fadeInPtr));


		//int testValue = 42; // 例として整数値を設定
		//m_material->SetValue<int>("testValue", testValue); // こんな感じで変数も渡せる
		m_material->SetValue<float>("transitionProgress", transitionProgress);
		m_material->SetValue<int>("isFadeIn", isFadeIn);
		// ポストプロセスマテリアルを使用して全画面クワッドを描画
		rtx->DrawFullScreenQuad(m_material.get());

		// SharedResourceにポストプロセス後のテクスチャを保存して、次のパスで使用できるようにする
		rtx->SetSharedResource("PostProcessPass_RenderTexture", &m_renderTexture);
	}
}

void TransitionPass::DrawProperty()
{
#ifdef USE_IMGUI
	// ポストプロセスパスのプロパティ描画処理
	ImGui::Text("TransitionPass Properties");
	// ここにImGuiを使ったプロパティ描画コードを追加します。 

	ImGui::Image(m_renderTexture.GetColorBuffer(), ImVec2(256, 144)); // 描画結果を表示

	ImGui::SliderFloat("Transition Progress", &transitionProgress, 0.0f, 1.0f); // 遷移の進行度を調整するスライダー
	ImGui::Checkbox("Fade In", &reinterpret_cast<bool&>(isFadeIn)); // フェードイン/アウトを切り替えるチェックボックス


#endif // USE_IMGUI

}