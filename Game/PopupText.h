#pragma once
#include "Engine/UI/UIComponent.h"
#include "Engine/Easing/EasingHandler.h"
class Text;

class PopupText : public UIComponent
{
	C_REFLECT(PopupText)
public:
	PopupText() = default;
	~PopupText() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	// ポップアップテキストを表示するための関数。引数でテキスト内容、色、フォントサイズ、表示時間を指定します。
	void ShowPopup(const std::wstring& text, const Color& color, float fontSize, float duration, Transform* targetTransform = nullptr);

private:
	EasingHandler easingHandler; // ポップアップのアニメーションに使用するイージングハンドラー
	Text* valueText = nullptr; // ポップアップテキストを表示する Text コンポーネントへの参照
	Vector2 originalPosition; // ポップアップの元の位置を保存するための変数
	Vector2 targetPosition; // ポップアップの目標位置を保存するための変数
	
};