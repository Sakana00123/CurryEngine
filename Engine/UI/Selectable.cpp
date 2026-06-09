#include "pch.h"
#include "Selectable.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Rendering/Pipeline/RenderContext.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Events/EventSystem.h"

REGISTER_COMPONENT_WITH_ATTRIBUTES(Selectable, "UI", ComponentAttributes::HideInAddComponentMenu | ComponentAttributes::RequiredComponent, { "RectTransform" });

void Selectable::OnPointerDown(PointerEventData* eventData)
{
	isPressed = true;
	UpdateVisual();
}
void Selectable::OnPointerUp(PointerEventData* eventData)
{
	isPressed = false;
	UpdateVisual();
}
void Selectable::OnPointerEnter(PointerEventData* eventData)
{
	isHovered = true;
	UpdateVisual();
}
void Selectable::OnPointerExit(PointerEventData* eventData)
{
	isHovered = false;
	UpdateVisual();
}
void Selectable::OnSelect(BaseEventData* eventData)
{
	isSelected = true;
	UpdateVisual();
}
void Selectable::OnDeselect(BaseEventData* eventData)
{
	isSelected = false;
	UpdateVisual();
}
void Selectable::OnMove(AxisEventData* eventData)
{

	Selectable* newSelectable = nullptr;

	switch (eventData->moveDir)
	{
	case MoveDirection::Left:
		newSelectable = navigation.left;
		break;
	case MoveDirection::Right:
		newSelectable = navigation.right;
		break;
	case MoveDirection::Up:
		newSelectable = navigation.up;
		break;
	case MoveDirection::Down:
		newSelectable = navigation.down;
		break;
	default:
		break;
	}

	if (newSelectable != nullptr) {
		EventSystem::GetCurrent()->SetSelectedGameObject(newSelectable->gameObject);
	}
}


bool Selectable::IsHovering() const 
{ 
	return isHovered;
}

void Selectable::UpdateVisual() 
{
	// 色の優先度：Pressing > Hovering > Selected > Default
	Image* image = GetImage();
	if (!image) return;
	if (!interactable) image->color = disabledColor;
	else if (isPressed) image->color = pressingColor;
	else if (isHovered) image->color = hoveringColor;
	else if (isSelected) image->color = selectedColor;
	else image->color = defaultColor;
}

bool Selectable::IsInteractable() const
{
	return interactable;
}
void Selectable::SetInteractable(bool value)
{
	interactable = value;
	UpdateVisual();
}

Image* Selectable::GetImage() const
{
	if (imageReference.IsValid()) {
		if (Scene* scene = GetScene()) {
			if (ObjectManager* objectManager = scene->GetObjectManager()) {
				const auto& cacheMap = objectManager->GetComponentCacheMap();
				if (cacheMap.find(imageReference) == cacheMap.end()) {
					//_ASSERT_EXPR_A(false, "Referenced Image component not found in ObjectManager cache.");
					return nullptr;
				}
				if (auto componentPtr = cacheMap.at(imageReference).lock())
				{
					if (Image* img = dynamic_cast<Image*>(componentPtr.get())) {
						return img;
					}
					else {
						//_ASSERT_EXPR_A(false, "Referenced component is not an Image.");
						return nullptr;
					}
				}
			}
		}
	}
	return nullptr;
}


void Selectable::DrawProperty()
{
#ifdef USE_IMGUI
#if 0
	bool isInteractable = IsInteractable();
	if (ImGui::Checkbox("interactable", &isInteractable)) {
		SetInteractable(isInteractable);
	}

	//イベント
	{
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

		if (ImGui::TreeNodeEx("Navigation", ImGuiTreeNodeFlags_DefaultOpen)) {

			const char* directions[] = { "Up", "Down", "Left", "Right" };
			Selectable* navigations[] = { navigation.up, navigation.down, navigation.left, navigation.right };

			for (size_t i = 0; i < 4; i++) {
				ImGui::PushID(static_cast<int>(i));

				//ドロップ先
				ImGui::Text(directions[i]);
				//ImGui::SameLine();
				ImGui::Button(navigations[i] ? navigations[i]->gameObject->name.c_str() : "None(Selectable)");
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) {
						IM_ASSERT(payload->DataSize == sizeof(ObjectId*));
						ObjectId* pId = static_cast<ObjectId*>(payload->Data);

						if (GameObject* obj = ObjectManager::Find(*pId)) {
							navigations[i] = obj->GetComponent<Selectable>();
						}
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::PopID();
			}

			navigation.up = navigations[0];
			navigation.down = navigations[1];
			navigation.left = navigations[2];
			navigation.right = navigations[3];

			ImGui::TreePop();
		}
		ImGui::PopStyleColor(3);
	}

	ImGui::ColorEdit4("DefaultColor", &defaultColor.r);
	ImGui::ColorEdit4("SelectedColor", &selectedColor.r);
	ImGui::ColorEdit4("HoveringColor", &hoveringColor.r);
	ImGui::ColorEdit4("PressingColor", &pressingColor.r);
	ImGui::ColorEdit4("DisabledColor", &disabledColor.r);
#else

	bool isInteractable = IsInteractable(); // 現在のインタラクティブ状態を取得
	
	// 自動生成されたプロパティ描画を呼び出す
	Component::DrawProperty();

	// インタラクティブ状態の変更を検出して反映
	if (isInteractable != IsInteractable()) {
		SetInteractable(IsInteractable());
	}



#endif // 0

#endif // USE_IMGUI
}