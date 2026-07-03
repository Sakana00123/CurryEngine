#pragma once
#include "UIComponent.h"
#include "Text.h"
#include "Archive/sprite.h"
#include <memory>
#include "Selectable.h"
#include "Engine/Utils/stdUtiles.h"

/**
 * @file
 * @brief テキスト入力用の UI コンポーネント。
 * @details フォーカス、カーソル（キャレット）点滅、キー入力処理を行い、
 *          `Text` コンポーネントへ編集結果を反映します。`Selectable` のナビゲーションや
 *          `EventSystem` と連携し、選択状態での継続更新（`IUpdateSelectedHandler`）にも対応します。
 */

/**
 * @brief 入力動作のフラグ。
 */
enum InputFlag { 
	MultiLine,       //!< 複数行入力を許可
	CommitOnEnter    //!< Enter キーで確定（単一行モードなど）
};

/**
 * @brief テキスト入力フィールド。
 * @details `Selectable` を継承し、選択・フォーカスの状態管理を行います。
 *          `IUpdateSelectedHandler` を実装し、選択中に継続的に更新処理（キー入力監視など）を行います。
 */
class InputField : public Selectable, public IUpdateSelectedHandler
{
	C_REFLECT(InputField)
public:
	/**
	 * @brief コンストラクタ。
	 */
	InputField();
	/** @brief デストラクタ。*/
	virtual ~InputField() override = default;

	/**
	 * @brief 初期化処理。
	 * @details テキストコンポーネントの準備やカーソル描画用リソースの初期化を行います。
	 */
	void Initialize() override;

	/**
	 * @brief 毎フレーム更新。
	 * @param elapsedTime 経過時間（秒）。
	 * @details キャレットの点滅やフォーカス状態の更新、必要に応じた内部状態更新を行います。
	 */
	void Update(float elapsedTime) override;

	/**
	 * @brief 描画開始時の処理。
	 * @param rtx 描画コンテキスト。
	 */
	void Begin(RenderContext* rtx) override;

	/**
	 * @brief 描画終了時の処理。
	 * @param rtx 描画コンテキスト。
	 */
	void End(RenderContext* rtx) override;

#ifdef USE_IMGUI
	/**
	 * @brief インスペクタ用のプロパティ描画。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

protected:
	/**
	 * @brief 選択中の継続更新（キー入力処理など）。
	 * @param eventData ベースイベントデータ。
	 */
	void OnUpdateSelected(BaseEventData* eventData) override;

	/**
	 * @brief 最大文字数を設定します（0 は無制限）。
	 * @param characterLimit 最大文字数。
	 */
	void SetCharacterLimit(int characterLimit) { this->characterLimit = characterLimit; }
	/**
	 * @brief 最大文字数を取得します。
	 * @return 現在設定されている最大文字数。
	 */
	int GetCharacterLimit() const { return characterLimit; }
protected:

	/**
	 * @brief ポインタダウン（クリック）時のコールバック。
	 * @param eventData ベースイベントデータ。
	 */
	void OnPointerDown(PointerEventData* eventData) override;

	/**
	 * @brief 選択時のコールバック。
	 * @param eventData ベースイベントデータ。
	 */
	void OnSelect(BaseEventData* eventData) override;

	/**
	 * @brief 選択解除時のコールバック。
	 * @param eventData ベースイベントデータ。
	 */
	void OnDeselect(BaseEventData* eventData) override;
private:

	/**
	 * @brief カーソル位置を更新します。
	 * @param move 位置の相対移動量（負数で左、正数で右）。
	 */
	void CursorUpdate(int move = 0);
public:
	
	/**
	 * @brief テキストコンポーネントを取得します。
	 * @return 関連付けられた `Text` コンポーネントへのポインタ。存在しない場合は `nullptr` を返します。
	 */
	Text* GetTextComponent() const;


	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
	ObjectId textComponentRef; // インスペクタでの参照用ID

	C_PROPERTY(CurryEngine::PropertyAttributes::HideInInspector)
	int inputFlags = 0; // 入力動作のフラグ（InputFlag のビットマスク）

	C_PROPERTY()
	Color cursorLineColor; // カーソルの色

	/** @brief 入力可能な最大文字数（0 は無制限）。*/
	C_PROPERTY()
	int characterLimit = 0;//最大文字数（0は制限なし）

private:
	/** @brief 現在のカーソル位置（文字インデックス）。*/
	int cursorPos = 0;
	//Image* cursorLine = nullptr;
	/** @brief カーソル（キャレット）描画用スプライト。*/
	std::unique_ptr<Sprite> cursorLine;
	/** @brief フォーカス中か。*/
	bool isFocus = false;
	/** @brief 点滅タイマー（秒）。*/
	float blinkTimer = 0.f;
	/** @brief 点滅間隔（秒）。*/
	const float blinkInterval = 0.5f;
	/** @brief 現在キャレットを表示するか。*/
	bool cursorVisible = false;
};