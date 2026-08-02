#include "pch.h"
#include "PopupText.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include <Engine/UI/Text.h>

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(PopupText, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(PopupText, "UserScripts", ComponentAttributes::None, {})


void PopupText::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}

void PopupText::Update(float deltaTime)
{
	if (easingHandler.GetSequenceCount() == 0 || easingHandler.IsCompleted())
	{
		return; // イージングが完了している場合は更新処理をスキップ
	}
	// 毎フレームの更新処理をここに実装します。
	float t = 0.0f; // 例: 経過時間を追跡する変数
	easingHandler.Update(t, deltaTime);

	// ポップアップテキストの位置を更新
	Vector2 newPosition = Vector2::Lerp(originalPosition, targetPosition, t);
	GetOwner()->GetComponent<RectTransform>()->SetAnchoredPosition(newPosition);

	// ポップアップテキストの透明度を更新（例: フェードアウトさせる場合）
	if (valueText)
	{
		Color currentColor = valueText->GetColor();
		currentColor.a = 1.0f - t; // 経過時間に応じて透明度を減少させる
		valueText->SetColor(currentColor);
	}
}

void PopupText::ShowPopup(const std::wstring& text, const Color& color, float fontSize, float duration, Transform* targetTransform)
{
	// ポップアップテキストを表示する処理をここに実装します。
	// 例えば、textObjectReference を使って Text オブジェクトを取得し、そのテキストや色、フォントサイズを設定するなどの処理が考えられます。
	easingHandler.AddEasing(EaseType::InCubic, 0.f, 1.f, 1.0f); // 1秒かけて上に移動
	easingHandler.SetCompletedFunction([this]() {
		// イージングが完了したときの処理
		GetOwner()->Destroy(); // ポップアップテキストを破棄
		});
	originalPosition = GetOwner()->GetComponent<RectTransform>()->GetAnchoredPosition(); // 元の位置を保存
	targetPosition = originalPosition - Vector2(0, 50); // 目標位置を設定（例: 上に50ピクセル移動）

	valueText = GetOwner()->GetComponent<Text>();
	if (valueText)
	{
		
		valueText->SetText(text); // 増加量を表示
		valueText->SetColor(color); // 色を設定
		valueText->SetFontSize(fontSize); // フォントサイズを設定
		valueText->SetHorizontalOverflow(Text::HorizontalOverflow::Overflow); // はみ出しを許可
		valueText->SetAlignment(Text::Alignment::MiddleCenter); // 中央揃え

		// ボールのスクリーン座標を取得してテキストの位置を設定
		if (targetTransform)
		{
			valueText->GetRectTransform()->SetAnchoredPositionByTransform(targetTransform);
		}
	}
}