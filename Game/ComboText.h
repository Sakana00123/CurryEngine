#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

class ComboText : public Component
{
	C_REFLECT(ComboText)
public:
	ComboText() = default;
	~ComboText() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;


	void AddComboCount(int count);

	void ResetComboCount();


	void UpdateComboText();

	int GetMaxComboCount() const { return maxComboCount; }

private:

	int comboCount = 0; // コンボ数を保持する変数

	int maxComboCount = 0; // 最大コンボ数を保持する変数

	// オブジェクト参照プロパティを定義する場合は、C_PROPERTY() マクロの引数に ObjectReference 属性を指定します。引数には参照先の型名を文字列で指定します。
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId textObjectReference; // Text オブジェクトへの参照を保持するプロパティ

};