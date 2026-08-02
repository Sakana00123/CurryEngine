#include "pch.h"
#include "ItemPageController.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"
#include <Engine/UI/Text.h>
#include <Engine/UI/Button.h>
#include "Engine/Audio/Audio.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(ItemPageController, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(ItemPageController, "UserScripts", ComponentAttributes::None, {})


void ItemPageController::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	if (Button* nextButton = GetScene()->FindComponentById<Button>(nextPageButtonRef))
	{
		nextButton->AddOnClickEvent([this]() { NextPage(); });
	}
	if (Button* prevButton = GetScene()->FindComponentById<Button>(previousPageButtonRef))
	{
		prevButton->AddOnClickEvent([this]() { PreviousPage(); });
	}
	UpdatePageDisplay(); // 初期表示の更新
}

void ItemPageController::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void ItemPageController::NextPage()
{
	// 次のページに移動する処理をここに実装します。
	pageIndex++;
	UpdatePageDisplay();
	Audio::PlayOneShot(L"./Assets/Sounds/SE/ButtonTrue.wav", 0.5f);
}

void ItemPageController::PreviousPage()
{
	// 前のページに移動する処理をここに実装します。
	if (pageIndex > 0)
	{
		pageIndex--;
		UpdatePageDisplay();
		Audio::PlayOneShot(L"./Assets/Sounds/SE/ButtonTrue.wav", 0.5f);
	}
}

void ItemPageController::GoToPage(int pageIndex)
{
	// 指定したページに移動する処理をここに実装します。
	this->pageIndex = pageIndex;
	UpdatePageDisplay();
}

void ItemPageController::UpdatePageDisplay()
{
	// ページの表示を更新する処理をここに実装します。
	// 例えば、現在の pageIndex に基づいてアイテムのリストを更新するなどの処理が考えられます。
	std::vector<GameObject*> children = GetOwner()->GetChildren();
	int startIndex = pageIndex * itemsPerPage;
	int endIndex = (std::min)(startIndex + itemsPerPage, static_cast<int>(children.size()));

	for (size_t i = 0; i < children.size(); ++i)
	{
		if (i >= startIndex && i < endIndex)
		{
			children[i]->SetActive(true); // 表示するアイテム
		}
		else
		{
			children[i]->SetActive(false); // 非表示にするアイテム
		}
	}

	if (auto textComponent = GetScene()->FindComponentById<Text>(pageIndexTextRef))
	{
		int totalPages = (children.size() + itemsPerPage - 1) / itemsPerPage; // 総ページ数を計算
		totalPages = (std::max)(totalPages, 1); // 総ページ数が0になるのを防ぐ
		std::wstring pageText = std::to_wstring(pageIndex + 1) + L"/" + std::to_wstring(totalPages); // 現在のページ番号と総ページ数を表示
		textComponent->SetText(pageText); // ページ番号は1から始まる表示にする
	}
	Button* previousPageButton = GetScene()->FindComponentById<Button>(previousPageButtonRef);
	Button* nextPageButton = GetScene()->FindComponentById<Button>(nextPageButtonRef);
	if (previousPageButton && nextPageButton)
	{
		previousPageButton->GetOwner()->SetActive(CanGoToPreviousPage()); // 前のページが存在する場合のみ前のページボタンを表示
		nextPageButton->GetOwner()->SetActive(CanGoToNextPage()); // 次のページが存在する場合のみ次のページボタンを表示
	}
}

bool ItemPageController::CanGoToNextPage() const
{
	// 次のページに移動できるかどうかをチェックする処理をここに実装します。
	std::vector<GameObject*> children = GetOwner()->GetChildren();
	int totalPages = (children.size() + itemsPerPage - 1) / itemsPerPage; // 総ページ数を計算
	return pageIndex < totalPages - 1; // 次のページが存在するかどうかを返す
}

bool ItemPageController::CanGoToPreviousPage() const
{
	// 前のページに移動できるかどうかをチェックする処理をここに実装します。
	return pageIndex > 0; // 前のページが存在するかどうかを返す
}