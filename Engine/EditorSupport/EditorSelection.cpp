#include "pch.h"
#include "EditorSelection.h"


void EditorSelection::Select(const std::shared_ptr<GameObject>& object, bool additive)
{
	if (!additive) {
		m_selected.clear();
	}
	
	auto it = std::find(m_selected.begin(), m_selected.end(), object);
	if (it != m_selected.end()) {
		if (additive) {
			// すでに選択されているオブジェクトが再度選択された場合、additive が true (Ctrlクリック) なら選択を解除する
			m_selected.erase(it);
			return;
		}
	}
	// オブジェクトが選択されていない場合、選択に追加する
	m_selected.push_back(object);
}

void EditorSelection::SelectRange(const std::shared_ptr<GameObject>& object, const std::vector<std::shared_ptr<GameObject>>& flatList, bool additive)
{
	// もし選択が空なら、単純に選択する
	if (m_selected.empty()) {
		Select(object);
		return;
	}
	
	auto pivot = m_selected.back(); // 選択の基点となるオブジェクト
	auto itPivot = std::find(flatList.begin(), flatList.end(), pivot);
	auto itTarget = std::find(flatList.begin(), flatList.end(), object);
	
	if (itPivot == flatList.end() || itTarget == flatList.end()) {
		// どちらかがリストに存在しない場合は、単純に選択する
		Select(object);
		return;
	}

	if (itPivot > itTarget) {
		std::swap(itPivot, itTarget); // 常に itPivot < itTarget になるようにする
	}

	// 範囲選択の結果を additive モードで追加するか、単純に置き換えるか
	if (!additive) {
		m_selected.clear();
	}
	for (auto& it = itPivot; it <= itTarget; ++it) {
		// すでに選択されているオブジェクトは追加しない
		if (std::find(m_selected.begin(), m_selected.end(), *it) == m_selected.end()) {
			m_selected.push_back(*it);
		}
	}
}

void EditorSelection::Deselect(const std::shared_ptr<GameObject>& object)
{
	m_selected.erase(std::remove(m_selected.begin(), m_selected.end(), object), m_selected.end());
}

void EditorSelection::Clear()
{
	m_selected.clear();
}

bool EditorSelection::IsSelected(const std::shared_ptr<GameObject>& object) const
{
	return std::find(m_selected.begin(), m_selected.end(), object) != m_selected.end();
}

bool EditorSelection::IsSelected(const GameObject* object) const
{
	return std::any_of(m_selected.begin(), m_selected.end(),
		[object](const std::shared_ptr<GameObject>& selected) {
			return selected.get() == object;
		});
}

bool EditorSelection::IsEmpty() const
{
	return m_selected.empty();
}

int EditorSelection::Count() const
{
	return static_cast<int>(m_selected.size());
}

const std::vector<std::shared_ptr<GameObject>>& EditorSelection::GetAll() const
{
	return m_selected;
}

std::shared_ptr<GameObject> EditorSelection::GetPrimary() const
{
	if (m_selected.empty()) {
		return nullptr;
	}
	return m_selected.back();
}