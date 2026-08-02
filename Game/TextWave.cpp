#include "pch.h"
#include "TextWave.h"
#include "Engine/UI/Text.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(TextWave, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(TextWave, "UserScripts", ComponentAttributes::None, {})


void TextWave::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	waveTime = 0.0f;
}

void TextWave::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
	// すべてのテキストをウェーブさせる



	waveTime += deltaTime; // Increment wave time


	Text* textComp = GetOwner()->GetComponent<Text>();

	if (textComp)
	{
		for (size_t i = 0; i < textComp->GetText().size(); ++i)
		{
			Text::CharModifier mod;

			mod.posOffset.y = sinf(-waveTime * 5.0f + i * 0.5f) * 2.0f; // Wave effect

			textComp->SetCharModifier(i, mod);
		}

	}


}