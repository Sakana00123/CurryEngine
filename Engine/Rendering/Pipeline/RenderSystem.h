#pragma once
#include "Engine/Rendering/Pipeline/RenderPipeline.h"

class Time;

/** @brief RenderSystem を表すクラスです。 */
class RenderSystem
{
public:
	/**
	 * @brief 初期化します。
	 */
	void Initialize(Time* time);
	/**
	 * @brief 描画処理を行います。
	 */
	void Render();

	// エディタGUIの描画処理。エディタモードでのみ呼び出され、ImGuiを使用してエディタ固有のUIを描画します。
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawEditorGUI();

	// ウィンドウサイズの変更イベント処理。ウィンドウサイズの変更などで描画ターゲットのサイズが変わったときに呼び出されます。登録された描画パスのリサイズが必要なレンダーターゲットをすべてリサイズします。
	/**
	 * @brief SizeChanged イベントを処理します。
	 */
	void OnSizeChanged(ID3D11Device* device, uint32_t width, uint32_t height);

	/**
	 * @brief 終了処理を行います。
	 */
	void Finalize();

private:
	Time* time{};

	// TODO: これ以上増えるようなら、配列やマップで管理するように変更する

	std::unique_ptr<RenderPipeline> sceneRenderPipeline;
	std::unique_ptr<RenderPipeline> gameRenderPipeline;
	std::unique_ptr<RenderPipeline> previewRenderPipeline;
	std::unique_ptr<RenderPipeline> effectPreviewRenderPipeline;
	std::unique_ptr<RenderPipeline> animationPreviewRenderPipeline;
};
