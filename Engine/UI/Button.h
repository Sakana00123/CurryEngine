#pragma once
#include "Selectable.h"
#include <any>

/**
 * @file Button.h
 * @brief クリック操作やSubmit操作に反応してイベントを発火するUIボタンコンポーネント。
 * @details
 * - `Selectable` を継承し、選択状態の管理や描画プロパティ編集に対応します。
 * - `IPointerClickHandler`, `ISubmitHandler` を実装し、マウス/タッチクリックや決定入力でイベントを発火します。
 * - インスペクタ（ImGui）から GameObject と機能（現在は `SetActive`）を紐づけて、クリック時に動的に処理を実行できます。
 * - C++20 でのコンパイルを想定しています。
 *
 * @note スレッドセーフではありません。メインスレッドから利用してください。
 */
class Button : public Selectable, public IPointerClickHandler, public ISubmitHandler, public IUpdateSelectedHandler
{
	C_REFLECT(Button)
private:
    /**
     * @brief クリック時に呼び出されるコールバック群。
     * @details `AddOnClickEvent` で登録した任意のコールバックが格納されます。登録順に実行されます。
     * @warning バインド対象オブジェクトのライフタイムは呼び出し側で保証してください（解放後に呼ぶと未定義動作）。
     */
    std::vector<std::function<void()>> onClickFunctions;

	/**
	* @brief クリックを有効にするかを返す関数オブジェクト。
	* @details クリック時にこの関数が設定されている場合、呼び出して `true` を返した場合のみクリック処理を実行します。
	* 無効な場合は常にクリック処理を実行します。
	* @note 例えば、ダイアログ表示中や特定の状態でクリックを無効にしたい場合に利用できます。
	*/
    std::function<bool()> clickFlagFunc = nullptr;

	/**
	* @brief カスタムクリック関数オブジェクト。
	* @details 毎フレーム呼び出され、`true` を返した場合にクリック処理を実行します。
	* @note 例えば、ゲームの状態に応じてクリックを動的に制御したい場合に利用できます。
	* クリック時だけでなく、毎フレームの状態チェックに使用されます。
	* クリック時にのみ評価される `clickFlagFunc` とは異なり、こちらは常に評価されます。
    */
	std::function<bool()> customClickFunc = nullptr;

    /**
     * @brief 選択時に呼び出されるコールバック群。
     * @details `AddOnSelectEvent` で登録した任意のコールバックが格納されます。登録順に実行されます。
     * @warning バインド対象オブジェクトのライフタイムは呼び出し側で保証してください（解放後に呼ぶと未定義動作）。
	 */
	std::function<void()> onSelectFunction = nullptr;

    /**
     * @brief 非選択時に呼び出されるコールバック群。
     * @details `AddOnDeselectEvent` で登録した任意のコールバックが格納されます。登録順に実行されます。
     * @warning バインド対象オブジェクトのライフタイムは呼び出し側で保証してください（解放後に呼ぶと未定義動作）。
	 */
	std::function<void()> onDeselectFunction = nullptr;

    /**
     * @brief 選択状態更新時に呼び出されるコールバック群。
     * @details 選択状態が変化した際に登録されたコールバックが実行されます。
	 * @warning バインド対象オブジェクトのライフタイムは呼び出し側で保証してください（解放後に呼ぶと未定義動作）。
	 */
	std::function<void()> onUpdateSelectFunction = nullptr;

    /**
     * @brief Button 内部で用いるイベント情報。
     * @details ImGui のインスペクタから設定され、クリック時に `pFunc` が実行されます。
     */
    struct EventInfo {
		/** @brief 対象となるObjectのID。インスペクタで設定されます。 */
		ObjectId objReference = ObjectId::Invalid();
		std::string className;                             ///< 対象となるクラス名。
		std::string funcName;                              ///< 実行する関数名。
		std::pair<std::string, std::any> value; ///< 型と値のペアの配列。関数引数などに使用します。

        bool IsValid() const {
			if (objReference == ObjectId::Invalid() && className.empty() && funcName.empty())
				return false;
			return true;
		}
    };

    /** @brief インスペクタで設定したイベント情報の配列。 */
    std::vector<EventInfo> eventInfo;
public:
    /** @brief 既定コンストラクタ。 */
    Button() = default;
    /** @brief 仮想デストラクタ。 */
    ~Button() override = default;

    /**
     * @brief 初期化処理。
     * @details 所属 `gameObject` から `Image` コンポーネントを取得して `image` に保持します。
     * @note `Image` コンポーネントが存在しない場合、`image` は `nullptr` になります。
     */
    void Initialize() override;

    /**
     * @brief 毎フレーム更新処理。
     * @param deltaTime 前フレームからの経過時間（秒）。
     * @details
	 * - `customClickFunc` が設定されている場合、呼び出して `true` を返した場合にクリック処理を実行します。
	 */
	void Update(float deltaTime) override;

    /**
     * @brief ポインタによるクリック入力ハンドラ。
     * @param eventData クリックイベントデータ（未使用）。
     * @details クリック時に `OnClick()` を呼び出します。
     */
    void OnPointerClick(PointerEventData* eventData) override {
        OnClick();
    }
    /**
     * @brief Submit（決定）入力ハンドラ。
     * @param eventData Submitイベントデータ（未使用）。
     * @details Submit時に `OnClick()` を呼び出します。
     */
    void OnSubmit(BaseEventData* eventData) override {
        OnClick();
    }

    /**
     * @brief 選択イベントハンドラ。
     * @param eventData 選択イベントデータ（未使用）。
     * @details 選択時に登録されたコールバックを実行します。
	 */
    void OnSelect(BaseEventData* eventData) override
    {
        Selectable::OnSelect(eventData);
        if (onSelectFunction) {
            onSelectFunction();
		}
    }

    /**
     * @brief 非選択イベントハンドラ。
     * @param eventData 非選択イベントデータ（未使用）。
	 * @details 非選択時に登録されたコールバックを実行します。
	 */
	void OnDeselect(BaseEventData* eventData) override
    {
        Selectable::OnDeselect(eventData);
        if (onDeselectFunction) {
            onDeselectFunction();
        }
    }

    /**
     * @brief 選択状態更新イベントハンドラ。
     * @param eventData 選択状態更新イベントデータ（未使用）。
	 * @details 選択状態が更新された際に登録されたコールバックを実行します。
	 */
    void OnUpdateSelected(BaseEventData* eventData) override
    {
        if (onUpdateSelectFunction) {
            onUpdateSelectFunction();
        }
	}

#ifdef USE_IMGUI
    /**
     * @brief インスペクタ（ImGui）用のプロパティ描画。
     * @details
     * - `USE_IMGUI` が有効な場合のみ有効。
     * - クリック時のイベント（`eventInfo`）をGUI上で編集できます。
     * - GameObject のドラッグ&ドロップ、および機能選択（`SetActive`）とフラグ設定に対応。
     */
    void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI


    /**
     * @brief `EventInfo` に基づいて実行関数 `pFunc` を更新します。
     * @param info 対象となるイベント情報。`funcId` と `flag`、`pObj` を参照します。
     * @details
     * - `funcId == 1` の場合、`pObj->SetActive(flag)` を呼ぶラムダを設定します。
     * - それ以外は `pFunc = nullptr` になります。
     * @warning `pObj == nullptr` の場合に実行するとクラッシュの可能性があります。設定時・実行時の NULL チェックを行ってください。
     */
    void UpdateInfo(EventInfo& info);

    /**
     * @brief メンバ関数をクリックイベントに登録します。
     * @tparam T メソッドを持つクラス型。
     * @param func 登録するメンバ関数ポインタ（引数なし・戻り値なし）。
     * @param instance 呼び出し対象インスタンス。
     * @note 登録順に実行されます。インスタンスのライフタイムは呼び出し側で保証してください。
     * @code
     * button->AddOnClickEvent(&MyClass::OnClicked, myInstance);
     * @endcode
     */
    template<class T>
    void AddOnClickEvent(void (T::* func)(void), T* instance) {
        onClickFunctions.emplace_back(std::bind(func, instance));
    }

    /**
     * @brief 任意の関数/ラムダをクリックイベントに登録します。
     * @param func 実行したい処理を格納した `std::function<void()>`。
     * @note キャプチャを伴うラムダも登録可能です。登録順に実行されます。
     */
    void AddOnClickEvent(std::function<void()> func) {
        onClickFunctions.emplace_back(func);
    }

	/**
	* @brief クリック時に呼ばれる関数でクリックを有効/無効にする関数を設定します。
	* @param func クリックを有効にするかを返す関数。`true` を返すとクリックが有効になります。
	*/
    void SetClickFlagFunction(const std::function<bool()>& func) {
        clickFlagFunc = func;
	}

	/**
	* @brief カスタムクリック関数を設定します。
	* @param func カスタムでクリック処理を実行するかの関数。`true` を返すとクリック処理が実行されます。
	* @note こちらの関数は毎フレーム呼び出され、クリック時にのみ評価される `clickFlagFunc` とは異なります。
	*/
    void SetCustomClickFunction(const std::function<bool()>& func) {
        customClickFunc = func;
	}

    /**
     * @brief 選択イベント関数を登録します。
	 * @param func 登録する関数オブジェクト（引数なし・戻り値なし）。
	 */
    void SetOnSelectEvent(const std::function<void()>& func) {
        onSelectFunction = func;
	}

    /**
	 * @brief 非選択イベント関数を登録します。
	 * @param func 登録する関数オブジェクト（引数なし・戻り値なし）。
	 */
    void SetOnDeselectEvent(const std::function<void()>& func) {
		onDeselectFunction = func;
	}

    /**
	 * @brief 選択状態更新イベント関数を登録します。
	 * @param func 登録する関数オブジェクト（引数なし・戻り値なし）。
     */
    void SetOnUpdateSelectEvent(const std::function<void()>& func) {
        onUpdateSelectFunction = func;
	}

    /**
     * @brief オブジェクトの状態を JSON 形式でシリアライズします。
     * @return シリアライズされた JSON オブジェクト。
     * @details `eventInfo` の内容も含めてシリアライズします。
	 */
	json Serialize() const override;

    /**
     * @brief JSON 形式のデータからオブジェクトの状態を復元します。
     * @param j 復元元の JSON オブジェクト。
     * @details `eventInfo` の内容も含めて復元します。
	 */
	void Deserialize(const json& j) override;

private:
    /**
     * @brief クリック時に呼ばれる内部処理。
     * @details
     * - まず `onClickFunctions` に登録されたコールバックを順に実行します。
     * - 続いて `eventInfo` に設定された `pFunc` を順に実行します（`nullptr` はスキップ）。
     */
    void OnClick();

};