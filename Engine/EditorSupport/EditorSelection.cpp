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

void EditorSelection::SelectTemp(const std::shared_ptr<GameObject>& object, bool additive)
{
	if (!additive) {
		m_isPreTempCommitInitClear = true; // 次の CommitTempSelection で既存の選択をクリアするフラグを立てる
	}

	auto it = std::find(m_selected.begin(), m_selected.end(), object);
	if (it != m_selected.end()) {
		if (additive) {
			// すでに選択されているオブジェクトが再度選択された場合、additive が true (Ctrlクリック) なら選択を解除するリストに追加する
			m_tempDeselected.push_back(object);
			return;
		}
	}
	// オブジェクトが選択されていない場合、一時的な選択リストに追加する
	m_tempSelected.push_back(object);
}

void EditorSelection::SelectTempRange(const std::shared_ptr<GameObject>& object, const std::vector<std::shared_ptr<GameObject>>& flatList)
{
	// もし選択が空なら、単純に一時選択する
	if (m_selected.empty()) {
		SelectTemp(object);
		return;
	}
	
	auto pivot = m_selected.back(); // 選択の基点となるオブジェクト
	auto itPivot = std::find(flatList.begin(), flatList.end(), pivot);
	auto itTarget = std::find(flatList.begin(), flatList.end(), object);
	
	if (itPivot == flatList.end() || itTarget == flatList.end()) {
		// どちらかがリストに存在しない場合は、単純に一時選択する
		SelectTemp(object);
		return;
	}
	if (itPivot > itTarget) {
		std::swap(itPivot, itTarget); // 常に itPivot < itTarget になるようにする
	}
	for (auto& it = itPivot; it <= itTarget; ++it) {
		m_tempSelected.push_back(*it);
	}
}

void EditorSelection::CommitTempSelection(bool additive)
{
	// 既存の選択をクリアする必要がある場合はクリアする
	if (m_isPreTempCommitInitClear) {
		m_selected.clear();
		m_isPreTempCommitInitClear = false;
	}

	// 一時的に選択されたオブジェクトを正式な選択に追加する
	for (auto& weakObj : m_tempSelected) {
		if (auto obj = weakObj.lock()) {
			Select(obj, additive);
		}
	}
	// 一時的に選択解除されたオブジェクトを正式な選択から削除する
	for (auto& weakObj : m_tempDeselected) {
		if (auto obj = weakObj.lock()) {
			Deselect(obj);
		}
	}
	m_tempSelected.clear();
	m_tempDeselected.clear();
}

void EditorSelection::LockCurrentSelection()
{
	m_isLocked = true;
	SyncLockedSelection();
}

void EditorSelection::UnlockCurrentSelection()
{
	m_isLocked = false;
	SyncLockedSelection();
}

void EditorSelection::Clear()
{
	m_selected.clear();
	m_tempSelected.clear();
	m_tempDeselected.clear();
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
	if (m_isLocked) {
		// ロックされている場合は、ロックされた選択状態が空かどうかを返す
		return m_lockedSelected.empty();
	}
	return m_selected.empty();
}

int EditorSelection::Count() const
{
	if (m_isLocked) {
		// ロックされている場合は、ロックされた選択状態の数を返す
		return static_cast<int>(m_lockedSelected.size());
	}
	return static_cast<int>(m_selected.size());
}

const std::vector<std::shared_ptr<GameObject>>& EditorSelection::GetAll() const
{
	std::vector<std::shared_ptr<GameObject>> result;
	if (m_isLocked) {
		// ロックされている場合は、ロックされた選択状態を返す
		for (const auto& weakObj : m_lockedSelected) {
			if (auto obj = weakObj.lock()) {
				result.push_back(obj);
			}
		}
	}
	else {
		// ロックされていない場合は、現在の選択状態を返す
		for (const auto& weakObj : m_selected) {
			if (auto& obj = weakObj/*.lock()*/) {
				result.push_back(obj);
			}
		}
	}

	return result;
}

std::shared_ptr<GameObject> EditorSelection::GetPrimary() const
{
	if (m_isLocked) {
		// ロックされている場合は、ロックされた選択状態の最後のオブジェクトを返す
		if (m_lockedSelected.empty()) {
			return nullptr;
		}
		if (auto obj = m_lockedSelected.back().lock()) {
			return obj;
		}
		return nullptr;
	}
	else {
		// ロックされていない場合は、現在の選択状態の最後のオブジェクトを返す
		if (m_selected.empty()) {
			return nullptr;
		}
		if (auto& obj = m_selected.back()/*.lock()*/) {
			return obj;
		}
		return nullptr;
	}
}

void EditorSelection::SyncLockedSelection()
{
	m_lockedSelected.clear();

	if (m_isLocked) {
		for (const auto& obj : m_selected) {
			m_lockedSelected.push_back(obj);
		}
	}
}