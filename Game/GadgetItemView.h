#pragma once
#include "Engine/UI/UIComponent.h"
#include "Engine/Core/Transform.h"
#include "GadgetItemData.h"

class GadgetItemView : public UIComponent
{
	C_REFLECT(GadgetItemView)
public:
	GadgetItemView() = default;
	~GadgetItemView() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;


	// ツールチップを表示する関数
	void ShowTooltip(Transform* attachPoint, const GadgetItemData& data);

	// ツールチップを非表示にする関数
	void HideTooltip();

private:

	C_PROPERTY( CurryEngine::PropertyAttributes::ObjectReference("GameObject") )
		ObjectId tooltipReference; // 例: アイテムのツールチップを表示する GameObject への参照

	//C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Image"))
	//	ObjectId backgroundImageReference; // 例: アイテムの背景画像への参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId nameTextReference; // 例: アイテム名を表示する Text コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId descriptionTextReference; // 例: アイテム説明を表示する Text コンポーネントへの参照

};