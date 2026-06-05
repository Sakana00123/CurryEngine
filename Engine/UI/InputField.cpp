#include "pch.h"
#include "InputField.h"
#include "Text.h"
#include "Engine/Scenes/Scene.h"

REGISTER_COMPONENT_WITH_ATTRIBUTES(InputField, "UI", ComponentAttributes::DisallowMultiple, {})

InputField::InputField() 
{
	//カーソルライン生成
	auto device = Graphics::GetDevice();
	cursorLine = std::make_unique<Sprite>(device);
}

void InputField::Initialize()
{
}

void InputField::Update(float elapsedTime)
{
	if (isFocus) {
		blinkTimer -= elapsedTime;
		if (blinkTimer < 0) {
			blinkTimer = blinkInterval;
			cursorVisible = !cursorVisible;
		}
	}
}
void InputField::Begin(RenderContext* rtx)
{
	/*if (Text * textComponent = GetTextComponent()) {
		textComponent->GetRectTransform()->pivot = GetRectTransform()->pivot;
	}*/
}

void InputField::End(RenderContext* rtx)
{
	if (cursorVisible) {
		if (Text* textComponent = GetTextComponent())
		{
			float x, y;
			textComponent->GetCursorPos(cursorPos, x, y);
			//cursorLine->rect->SetPosition({ x,y,0 });
			//cursorLine->rect->size = { 1.f, static_cast<float>(textComponent->lineHeight) };
			float lineHeight = static_cast<float>(textComponent->lineHeight) * (textComponent->fontSize / textComponent->fntSize);
			cursorLine->Render(rtx->immediateContext, x, y, 2.0f, lineHeight, cursorLineColor);
		}
	}
}

void InputField::DrawProperty()
{
#ifdef USE_IMGUI
	Selectable::DrawProperty();
	if (Text* textComponent = GetTextComponent()) {
		ImGui::Text("Text:%ls", textComponent->text.c_str());
	}
	int limit = static_cast<int>(characterLimit);
	if (ImGui::InputInt("characterLimit", &limit)) {
		characterLimit = static_cast<size_t>(std::clamp(limit, 0, 256));
		if (Text* textComponent = GetTextComponent()) {
			textComponent->SetCharacterLimit(characterLimit);
		}
	}

	bool allowMultiLine = (inputFlags & static_cast<int>(InputFlag::MultiLine)) != 0;
	if (ImGui::Checkbox("Multi Line", &allowMultiLine)) {
		inputFlags ^= static_cast<int>(InputFlag::MultiLine);
	}
	bool commitOnEnter = (inputFlags & static_cast<int>(InputFlag::CommitOnEnter)) != 0;
	if (ImGui::Checkbox("Commit On Enter", &commitOnEnter)) {
		inputFlags ^= static_cast<int>(InputFlag::CommitOnEnter);
	}


#endif // USE_IMGUI
}

void InputField::OnUpdateSelected(BaseEventData* eventData)
{
	Text* textComponent = GetTextComponent();
	if (!textComponent)
		return;

	std::wstring inputStr = InputSystem::inputString;

	if (!inputStr.empty()) {

		if (characterLimit > textComponent->GetText().size() || characterLimit == 0)
		{
			//std::wstring wstr = StringToWstring(inputStr);
			WPARAM wParam = InputSystem::inputChar;

			if (wParam >= 32) {

				std::wstring tempStr = textComponent->GetText();
				size_t prevLength = tempStr.length();
				if (cursorPos > tempStr.length()) {
					cursorPos = tempStr.length();
				}
				tempStr.insert(cursorPos, inputStr);
				size_t insertedLength = tempStr.length() - prevLength;
				textComponent->SetText(tempStr);//確定文字をバッファに追加
				CursorUpdate(static_cast<int>(insertedLength));
			}
			else if (wParam == VK_RETURN) {//Enter
				if ((inputFlags & static_cast<int>(InputFlag::MultiLine)) != 0)
				{
					textComponent->InsertText(cursorPos, L"\n");
					CursorUpdate(1);
				}
				else if ((inputFlags & static_cast<int>(InputFlag::CommitOnEnter)) != 0)
				{
					// 確定処理（必要に応じてイベント発行など）
					// ここでは単純にフォーカスを外す例
					isFocus = false;
					CursorUpdate();
				}
			}
		}
	}

	std::string keyStr = InputSystem::keyString;

	if (!keyStr.empty()) {

		WPARAM wParam = InputSystem::inputKeyDown;

		if (wParam == VK_BACK && !textComponent->GetText().empty() && cursorPos > 0) {//BackSpace
			textComponent->EraseText(cursorPos - 1, 1);
			CursorUpdate(-1);
		}
		else if (wParam == VK_DELETE && cursorPos < textComponent->GetText().size()) {//Delete
			textComponent->EraseText(cursorPos, 1);
			CursorUpdate();
		}
		else if (wParam == VK_RIGHT && cursorPos < textComponent->GetText().size()) {
			CursorUpdate(1);
		}
		else if (wParam == VK_LEFT && cursorPos > 0) {
			CursorUpdate(-1);
		}
	}
}

void InputField::OnPointerDown(PointerEventData* eventData)
{
	Selectable::OnPointerDown(eventData);
	// クリック位置にカーソルを移動
	Text* textComponent = GetTextComponent();
	if (!textComponent)
		return;

	// 矩形内にいるかチェック
	if (!GetRectTransform()->Contains(eventData->position))
		return;

	isFocus = true;
	cursorPos = textComponent->GetCursorIndexFromPoint(
		eventData->position.x,
		eventData->position.y
	);
	CursorUpdate();
}

void InputField::OnSelect(BaseEventData* eventData)
{
	isFocus = true;
	CursorUpdate();
}
void InputField::OnDeselect(BaseEventData* eventData)
{
	isFocus = false;
	CursorUpdate();
}

void InputField::CursorUpdate(int move)
{
	cursorPos += move;
	cursorVisible = isFocus;
	blinkTimer = blinkInterval;
}

Text* InputField::GetTextComponent() const
{
	if (textComponentRef != ObjectId::Invalid()) {
		return GetScene()->FindComponentById<Text>(textComponentRef);
	}
	return nullptr;
}