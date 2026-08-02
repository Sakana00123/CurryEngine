#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "PassiveSkillData.h"
#include "Engine/Events/EventHandlers.h"

class ItemTooltipController : public Component, public IPointerEnterHandler, public IPointerExitHandler
{
	C_REFLECT(ItemTooltipController)
public:
	ItemTooltipController() = default;
	~ItemTooltipController() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	// IPointerEnterHandler の実装
	void OnPointerEnter(PointerEventData* eventData) override;

	// IPointerExitHandler の実装
	void OnPointerExit(PointerEventData* eventData) override;

	// ツールチップを設定する関数(PassiveSkillComponent から呼び出されることを想定)
	void SetupTooltip(const ItemData& data);

	// ツールチップの表示/非表示を切り替える関数
	void SetTooltipActive(bool active);

private:

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId tooltipPanelReference; // ツールチップのパネルへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
	ObjectId nameTextReference; // パッシブスキルの名前を表示するテキストコンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
	ObjectId descriptionTextReference; // パッシブスキルの説明を表示するテキストコンポーネントへの参照

};