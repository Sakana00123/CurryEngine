#pragma once
#include "UIComponent.h"

/**
 * @file
 * @brief UI のルートとなるキャンバスを表すコンポーネント。
 * @details 画面解像度に追従して `RectTransform` のサイズとアンカーを自動調整し、
 *          登録された `Graphic` コンポーネントのリスト管理（登録/遅延削除）を行います。
 *          `ImGui` が有効な場合はインスペクタ用の簡易情報も表示します。
 */

class Graphic;

/**
 * @brief UI 要素（`Graphic`）をまとめるキャンバス。
 * @details `UIComponent` を継承し、ゲーム画面全体にフィットするように毎フレーム自身の
 *          `RectTransform` を更新します。`Graphic` の登録解除はフレーム間で競合を避けるため
 *          遅延削除キュー（`erases`）を用いて行われます。
 */
class Canvas : public UIComponent
{
	C_REFLECT(Canvas)
	/**
	 * @brief 管理対象の `Graphic` 一覧。
	 * @note 要素の寿命管理（delete 等）は行いません。所有権は保持しません。
	 */
	std::vector<Graphic*> graphics;

	/**
	 * @brief 次フレームの更新時に削除する `Graphic` の一時リスト（遅延削除用）。
	 */
	std::vector<Graphic*> erases;
public:
	/** @brief 既定コンストラクタ。*/
	Canvas() = default;
	/** @brief デストラクタ。*/
	~Canvas() override = default;

	/**
	 * @brief 初期化処理。
	 * @details 現在の画面サイズを取得して、キャンバスの `RectTransform::size` を設定します。
	 */
	void Initialize() override;

	/**
	 * @brief 毎フレーム更新。
	 * @param elapsedTime 経過時間（秒）。
	 * @details 以下の処理を行います。
	 * - 遅延削除リスト `erases` に入っている `Graphic` を `graphics` から除去。
	 * - 画面サイズに合わせて `RectTransform` のサイズ/アンカー/位置を固定（中央寄せ）。
	 */
	void Update(float elapsedTime) override;

	/**
	 * @brief 管理している `Graphic` の一覧を取得します。
	 * @return `Graphic*` の配列（コピー）。
	 * @note 返り値はコピーです。サイズが大きい場合はコストに注意してください。
	 */
	std::vector<Graphic*> GetGraphics() const;

	/**
	 * @brief `Graphic` をキャンバスに登録します。
	 * @param graphic 登録する `Graphic` へのポインタ（非所有）。
	 * @note 重複登録の検出は行いません。必要に応じて呼び出し側で制御してください。
	 */
	void RegisterGraphic(Graphic* graphic) {
		graphics.emplace_back(graphic);
	}

	/**
	 * @brief `Graphic` の登録を解除します（遅延削除）。
	 * @param graphic 解除対象の `Graphic` へのポインタ。
	 * @details 即時削除は行わず、次回 `Update` 時に除去します。反復中のコンテナ変更による
	 *          不整合やクラッシュを避けるための設計です。
	 */
	void UnregisterGraphic(Graphic* graphic) {
		erases.emplace_back(graphic);
	}

#ifdef USE_IMGUI
	/**
	 * @brief インスペクタ（デバッグ UI）にプロパティを描画します。
	 * @details `USE_IMGUI` 定義時のみ、管理している `Graphic` 数を表示します。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

};