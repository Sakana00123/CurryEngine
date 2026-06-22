#pragma once
#include "Engine/Rendering/Pipeline/RenderPipeline.h"

class Time;

class RenderSystem
{
public:
	void Initialize(Time* time);
	void Render();

	// エディタGUIの描画処理。エディタモードでのみ呼び出され、ImGuiを使用してエディタ固有のUIを描画します。
	void DrawEditorGUI();

	// ウィンドウサイズの変更イベント処理。ウィンドウサイズの変更などで描画ターゲットのサイズが変わったときに呼び出されます。登録された描画パスのリサイズが必要なレンダーターゲットをすべてリサイズします。
	void OnSizeChanged(ID3D11Device* device, uint32_t width, uint32_t height);

	void Finalize();

private:
	Time* time{};
	std::unique_ptr<RenderPipeline> sceneRenderPipeline;
	std::unique_ptr<RenderPipeline> gameRenderPipeline;
	std::unique_ptr<RenderPipeline> previewRenderPipeline;
};