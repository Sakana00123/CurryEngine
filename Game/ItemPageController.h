#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

class ItemPageController : public Component
{
	C_REFLECT(ItemPageController)
public:
	ItemPageController() = default;
	~ItemPageController() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void NextPage(); // 次のページに移動する関数

	void PreviousPage(); // 前のページに移動する関数

	void GoToPage(int pageIndex); // 指定したページに移動する関数

	void UpdatePageDisplay(); // ページの表示を更新する関数

	bool CanGoToNextPage() const; // 次のページに移動できるかどうかをチェックする関数

	bool CanGoToPreviousPage() const; // 前のページに移動できるかどうかをチェックする関数

private:

	int pageIndex = 0; // ページのインデックス（0から始まる）
	
	C_PROPERTY()
	int itemsPerPage = 15; // 1ページあたりのアイテム数

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
	ObjectId pageIndexTextRef; // ページインデックスを表示する Text コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Button"))
	ObjectId nextPageButtonRef; // 次のページボタンへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Button"))
	ObjectId previousPageButtonRef; // 前のページボタンへの参照
};