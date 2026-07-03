#pragma once

#include "Engine/Rendering/Pipeline/Pass/RenderPass.h"
struct RenderContext;
class Scene;

/**************************************************
 * @file
 * @brief 描画パイプラインの基底クラス。
 * @details 描画パイプラインの共通インターフェースを定義し、各種描画パス（ジオメトリパス、ライトパス、ポストエフェクトパスなど）を管理します。
 **************************************************/
class RenderPipeline
{
public:
	/** @brief 既定コンストラクタ。*/
	RenderPipeline() = default;
	/** @brief デストラクタ。*/
	virtual ~RenderPipeline() = default;

	/** @brief 描画パイプラインの描画パス登録処理。派生クラスで描画パスの登録処理を実装します。*/
	virtual void SetupRenderPasses() = 0;

	/** @brief 描画パイプラインの初期化処理。描画パスの登録と初期化を行います。*/
	virtual void Initialize();
	/** @brief 描画パイプラインの終了処理。リソースの解放などを行います。*/
	virtual void Finalize();
	/**
	 * @brief 描画パイプラインの実行処理。
	 * @param rtx 描画に必要なコンテキスト情報。
	 * @param scene 描画対象のシーン。
	 */
	void Execute(RenderContext* rtx, Scene* scene);

	/** @brief 描画パイプラインのプロパティ描画処理。*/
	void DrawProperty();

	/**
	 * @brief 描画パイプラインのサイズ変更イベント処理。
	 * @param device Direct3D 11 デバイス。
	 * @param width 新しい幅。
	 * @param height 新しい高さ。
	 * @details ウィンドウサイズの変更などで描画ターゲットのサイズが変わったときに呼び出されます。登録された描画パスのリサイズが必要なレンダーターゲットをすべてリサイズします。
	 */
	void OnSizeChanged(ID3D11Device* device, uint32_t width, uint32_t height);

protected:
	/**
	 * @brief 描画パスを追加します。
	 * @param pass 追加する描画パスのユニークポインタ。
	 * @details 描画パイプラインは複数の描画パスを順番に実行します。描画パスは `RenderPass` クラスを継承して実装されるべきです。
	 */
	void AddRenderPass(std::unique_ptr<RenderPass> pass);

private:
	// 描画パスのリスト。描画パイプラインは複数の描画パスを順番に実行します。
	std::vector<std::unique_ptr<RenderPass>> m_renderPasses;
};

// ---------------------------------- 各描画パイプラインの定義 ----------------------------------

// シーンビューの描画パイプラインクラス
class SceneRenderPipeline : public RenderPipeline
{
public:
	/** @brief 描画パイプラインの描画パス登録処理。シーンビューに必要な描画パスを登録します。*/
	void SetupRenderPasses() override;
};

// ゲームビューの描画パイプラインクラス
class GameRenderPipeline : public RenderPipeline
{
public:
	/** @brief 描画パイプラインの描画パス登録処理。ゲームビューに必要な描画パスを登録します。*/
	void SetupRenderPasses() override;
};

// プレビュー表示の描画パイプラインクラス
class PreviewRenderPipeline : public RenderPipeline
{
public:
	/** @brief 描画パイプラインの描画パス登録処理。プレビュー表示に必要な描画パスを登録します。*/
	void SetupRenderPasses() override;
};

// エフェクトプレビュー表示の描画パイプラインクラス
class EffectPreviewRenderPipeline : public RenderPipeline
{
public:
	/** @brief 描画パイプラインの描画パス登録処理。エフェクトプレビュー表示に必要な描画パスを登録します。*/
	void SetupRenderPasses() override;
};