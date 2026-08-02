#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Gadget.h"

class Smoke : public Gadget
{
	C_REFLECT(Smoke)
public:
	Smoke() = default;
	~Smoke() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void OnActivate() override; // ガジェットがアクションを実行したときの処理
	void OnDeactivate() override; // ガジェットがアクションを実行したときの処理

	void OnPreviewEnter() override; // ピンに近づけたときのプレビュー表示開始イベント（必要に応じて実装）
	void OnPreviewExit() override; // ピンから離れたときのプレビュー表示終了イベント（必要に応じて実装）

	void OnAttachment() override; // ガジェットがオブジェクトにアタッチされたときの処理
	void OnRoundEnd() override; // ラウンド終了時の処理

	void OnBreak() override; // ガジェットが壊れたときの処理

	bool IsSmokeActive() const { return smokeActive; } // スモークが現在アクティブかどうかを返す関数

private:
	bool smokeActive = false; // スモークが現在アクティブかどうかを示すフラグ

	// ここにコンポーネントのメンバ変数を定義します。必要に応じて C_PROPERTY() マクロを使用してシリアライズ可能なプロパティを定義できます。
	//C_PROPERTY()
	//int exampleProperty = 0; // 例: シリアライズされるプロパティ

	// 属性を指定する場合は、C_PROPERTY() マクロの引数に属性フラグを指定します。ComponentAttributes 名前空間の enum と対応しています。
	//C_PROPERTY(CurryEngine::PropertyAttributes::ReadOnly)
	//float readOnlyProperty = 0.0f; // 例: 読み取り専用プロパティ

	// オブジェクト参照プロパティを定義する場合は、C_PROPERTY() マクロの引数に ObjectReference 属性を指定します。引数には参照先の型名を文字列で指定します。
	//C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Transform"))
	//ObjectId transformReference; // 例: Transform コンポーネントへの参照


};