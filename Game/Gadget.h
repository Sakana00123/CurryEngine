#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "GadgetItemData.h"
class RectTransform;
class Text;

// Gadget クラスは、ゲーム内の特定のエリアやオブジェクトにアタッチされるコンポーネントで、プレイヤーやボールなどが接触したときに特定の効果を発生させるための基底クラスです。例えば、重力を変化させるエリアや、速度を減少させるエリアなど、様々なタイプのガジェットがこのクラスを継承して実装されます。
class Gadget : public Component
{
	C_REFLECT(Gadget)

public:
	enum class GadgetType
	{
		AllyGadget, // 味方ガジェット
		ObtrusiveGadget // 妨害ガジェット
	};

public:
	Gadget() = default;
	~Gadget() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Finalize() override;
	void LateUpdate(float deltaTime) override;

	virtual void OnAttachment(); // ガジェットがオブジェクトにアタッチされたときのイベント

	// ピンに近づけたときにプレビュー表示を開始するためのイベント。必要に応じてオーバーライドして実装します。
	virtual void OnPreviewEnter() {}

	// ピンから離れたときにプレビュー表示を終了するためのイベント。必要に応じてオーバーライドして実装します。
	virtual void OnPreviewExit() {}


	// ガジェットが有効化されたときのイベント
	virtual void OnActivate() {}
	// ガジェットが無効化されたときのイベント
	virtual void OnDeactivate() {}

	// ガジェットがアクティブ化できるかどうかを判断するための関数。必要に応じてオーバーライドして条件を指定します。
	virtual bool CanActivate() const;

	// ガジェットが特定のアクションを実行するためのイベント（例: プレイヤーがガジェットを使用したとき）
	virtual void OnAction() {}

	// ガジェットがラウンドの終了時に特定の処理を行うためのイベント。必要に応じてオーバーライドして実装します。
	virtual void OnRoundEnd() {}

	// ガジェットが壊れたときのイベント。耐久値が0になったときなどに呼び出すことができます。
	virtual void OnBreak();

	virtual void ClearBallSet() {} // ガジェットが管理しているボールのセットをクリアするための関数。必要に応じてオーバーライドして実装します。

	// アクションを実行するための関数。
	void PerformAction();

	// ガジェットの耐久値を減らす関数。耐久値が0になったときに OnBreak() を呼び出す。
	void DecreaseDurability();

	virtual void SetDisabled(bool disabled)
	{
		isDisabled = disabled;

		// 無効化された場合、管理しているボールなどの情報をリセットする
		if (isDisabled)
		{
			ClearBallSet();
		}

	}
	virtual bool IsDisabled() const { return isDisabled; }


public:

	// ガジェットをアクティブ化するための関数。CanActivate() を呼び出して条件をチェックし、条件が満たされていれば OnActivate() を呼び出します。
	void Activate();

	// ガジェットを非アクティブ化するための関数。単純に OnDeactivate() を呼び出します。
	void Deactivate();

	// ガジェットの耐久値を取得する関数
	int GetDurability() const { return durability; }

	// ガジェットの耐久値を設定する関数
	void SetDurability(int newDurability);

	//ガジェットのタイプをセットする関数
	void SetGadgetType(GadgetType type) { gadgetType = type; }

	//ガジェットのタイプを取得する関数
	GadgetType GetGadgetType() const { return gadgetType; }

	// ガジェットのアイテムデータをセットする関数
	void SetGadgetItemData(const GadgetItemData& data) { gadgetItemData = data; }

	// ガジェットのアイテムデータを取得する関数
	GadgetItemData GetGadgetItemData() const { return gadgetItemData; }

private:
	C_PROPERTY(CurryEngine::PropertyAttributes::ReadOnly, CurryEngine::PropertyAttributes::NonSerialized)
	int durability = 10; // ガジェットの耐久値。これが0になるとガジェットが壊れるなどの処理を行うことができます。


	bool gadgetActive = false; // ガジェットの現在のアクティブ状態を管理するフラグ

	bool isDisabled = false; // ガジェットが無効化されているかどうかを管理するフラグ。これが true の場合、ガジェットはアクティブ化できません。

	RectTransform* durabilityRectTransform = nullptr; // ガジェットの耐久値を表示するための RectTransform コンポーネントへのポインタ。必要に応じてシーン内でアサインして使用します。
	Text* durabilityText = nullptr; // ガジェットの耐久値を表示するための Text コンポーネントへのポインタ。必要に応じてシーン内でアサインして使用します。

	GadgetType gadgetType = GadgetType::AllyGadget; // ガジェットのタイプを管理する変数

	GadgetItemData gadgetItemData; // ガジェットのアイテムデータを管理する変数。必要に応じてシーン内でアサインして使用します。
};