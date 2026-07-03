#include "pch.h"
#include "GraphicRaycaster.h"
#include "Canvas.h"
#include "Graphic.h"
#include "Engine/Events/PointerEventData.h"
#include "Engine/Events/RaycastResult.h"
#include "Engine/Events/EventSystem.h"

REGISTER_COMPONENT_WITH_ATTRIBUTES(GraphicRaycaster, "UI", ComponentAttributes::DisallowMultiple | ComponentAttributes::RequiredComponent, { "RectTransform" });

void GraphicRaycaster::OnEnable()
{
	if (GetOwner() == nullptr) return; // 所属 GameObject が無効な場合は処理しない
	EventSystem::RegisterGraphicRaycaster(gameObject->GetComponentShared<GraphicRaycaster>());
}

void GraphicRaycaster::OnDisable()
{
	if (GetOwner() == nullptr) return; // 所属 GameObject が無効な場合は処理しない
	// TODO: ここ通ってないかも（playMode/editMode切り替えで呼ばれてないので今後UIで問題が起きたら要確認）
	EventSystem::UnregisterGraphicRaycaster(gameObject->GetComponentShared<GraphicRaycaster>());
}


void GraphicRaycaster::Raycast(std::shared_ptr<PointerEventData> eventData, std::vector<RaycastResult>& resultAppendList) {
	if (Canvas* canvas = gameObject->GetComponent<Canvas>()) {

		for (Graphic* graphic : canvas->GetGraphics()) {
			if (graphic == nullptr) continue; // 無効な Graphic はスキップ
			if (graphic->IsEnabled() == false) continue; // 無効な Graphic はスキップ
			if (!graphic->Raycast(eventData->position)) continue;

			//ヒットしたら追加
			RaycastResult result;
			result.gameObject = graphic->gameObject;
			result.hitGraphicId = graphic->GetId();
			result.moduleId = this->GetId();
			result.screenPosition = eventData->position;

			resultAppendList.push_back(result);
		}

	}

}