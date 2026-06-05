#pragma once
#include "Engine/Rendering/Pipeline/RenderContext.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Rendering/Material.h"

class RenderPass
{
public:
	virtual ~RenderPass() = default;

	// レンダリングパスの初期化処理（必要に応じてオーバーライド）
	virtual void Initialize() {}

	// レンダリングパスの終了化処理（必要に応じてオーバーライド）
	virtual void Finalize() {}

	// 各レンダリングパスで実装されるべき純粋仮想関数
	virtual void Execute(RenderContext* rtx, Scene* scene) = 0;

	// レンダリングパスのプロパティ描画処理（必要に応じてオーバーライド）
	virtual void DrawProperty() {}

	//// パスの入力リソースを定義するための関数（必要に応じてオーバーライド）
	//virtual std::vector<std::string> GetInputs() const { return {}; }
	//// パスの出力リソースを定義するための関数（必要に応じてオーバーライド）
	//virtual std::vector<std::string> GetOutputs() const { return {}; }

	// レンダーターゲットのリサイズイベントを処理するための関数
	void OnSizeChanged(ID3D11Device* device, uint32_t width, uint32_t height);

	// デバッグ用のパス名を設定するための関数
	void SetRenderPassName(const char* name) { renderPassName = name; }

	// デバッグ用のパス名を取得するための関数
	const char* GetRenderPassName() const { return renderPassName; }

protected:

	// レンダーターゲットのリサイズが必要な場合に、リストに追加するための関数
	void RegisterResizableRenderTexture(RenderTexture* rt);

private:
	friend class RenderPipeline;
	std::vector<RenderTexture*> resizableRenderTargets; // リサイズが必要なレンダーターゲットのリスト
	const char* renderPassName = "Unnamed Pass"; // デバッグ用のパス名
};