#include "pch.h"
#include "ComboText.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include <Engine\UI\Text.h>

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(ComboText, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(ComboText, "UserScripts", ComponentAttributes::None, {})


void ComboText::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	ResetComboCount(); // コンボカウントを初期化

	maxComboCount = 0; // 最大コンボカウントもリセット

}

void ComboText::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void ComboText::AddComboCount(int count)
{
	// コンボカウントを増加させる処理をここに実装します。
	comboCount += count; // 引数で指定された数だけコンボカウントを増加
	UpdateComboText(); // コンボテキストを更新
	if (comboCount > maxComboCount)
	{
		maxComboCount = comboCount; // 最大コンボカウントを更新
	}
}

void ComboText::ResetComboCount()
{
	// コンボカウントをリセットする処理をここに実装します。
	comboCount = 0; // コンボカウントをリセット
	UpdateComboText(); // コンボテキストを更新
}

void ComboText::UpdateComboText()
{
	// コンボテキストを更新する処理をここに実装します。
	// 例えば、textObjectReference を使って Text オブジェクトを取得し、そのテキストを更新するなどの処理が考えられます。
	if (Text* textComponent = GetScene()->FindComponentById<Text>(textObjectReference))
	{
		std::wstring comboText = comboCount > 0 ? std::to_wstring(comboCount) + L"コンボ" : L""; // コンボカウントが0より大きい場合は表示、そうでない場合は空文字列
		textComponent->SetText(comboText); // Text コンポーネントのテキストを更新
	}
}