#pragma once
#include "UIComponent.h"
#include "Engine/Events/EventSystem.h"

/**
 * @file
 * @brief UI 用グラフィックのレイキャストを行うコンポーネント。
 * @details 有効化時に `EventSystem` へ登録し、無効化時に登録解除します。
 *          受け取ったポインタイベントに基づき、ヒットした UI 要素の情報を
 *          `RaycastResult` として結果リストに追記します。
 */

class PointerEventData;
struct RaycastResult;

/**
 * @brief UI のヒットテストを担当するレイキャスター。
 * @details `UIComponent` を継承し、`EventSystem` からの問い合わせに応じて
 *          画面上の UI 当たり判定を行います。
 */
class GraphicRaycaster : public UIComponent
{
	C_REFLECT(GraphicRaycaster)
public:
	/** @brief 既定コンストラクタ。*/
	GraphicRaycaster() = default;
	/** @brief デストラクタ。*/
	virtual ~GraphicRaycaster() override = default;

	/**
	 * @brief コンポーネント有効化時の処理。
	 * @details このレイキャスターを `EventSystem` に登録します。
	 */
	void OnEnable() override;

	/**
	 * @brief コンポーネント無効化時の処理。
	 * @details このレイキャスターを `EventSystem` から登録解除します。
	 */
	void OnDisable() override;

	/**
	 * @brief UI 要素に対するレイキャストを実行します。
	 * @param eventData ポインタ入力（位置など）を保持するイベントデータ。
	 * @param resultAppendList ヒット結果を追記するリスト。既存要素は保持されます。
	 * @details ヒットした UI 要素ごとに `RaycastResult` を生成し、`resultAppendList` に追加します。
	 *          並び順やフィルタリングは実装依存です。
	 */
	void Raycast(std::shared_ptr<PointerEventData> eventData, std::vector<RaycastResult>& resultAppendList);
};