#pragma once
#include <vector>
#include <memory>
#include "Engine/Core/GameObject.h"

/**
 * @file
 * @brief EditorSelection.h
 * @details エディタ上で選択されているオブジェクトを管理するクラス
 */
class EditorSelection
{
public:
	EditorSelection() = default;
	~EditorSelection() = default;

	/**
	 * @brief オブジェクトを選択します。
	 * @param object 選択するオブジェクト
	 * @param additive `true` の場合、既存の選択に追加します。`false` の場合、既存の選択をクリアしてから選択します。
	 */
	void Select(const std::shared_ptr<GameObject>& object, bool additive = false);

	/**
	 * @brief オブジェクトの範囲選択を行います。選択されているオブジェクトと指定されたオブジェクトの間にあるオブジェクトを選択します。
	 * @param object 範囲選択の終点となるオブジェクト
	 * @param flatList オブジェクトのフラットなリスト（階層構造を無視したリスト）。範囲選択はこのリストの順序に基づいて行われます。
	 * @param additive `true` の場合、既存の選択に追加します。`false` の場合、既存の選択をクリアしてから選択します。
	 */
	void SelectRange(const std::shared_ptr<GameObject>& object, const std::vector<std::shared_ptr<GameObject>>& flatList, bool additive = false);

	/**
	 * @brief オブジェクトの選択を解除します。
	 * @param object 選択解除するオブジェクト
	 */
	void Deselect(const std::shared_ptr<GameObject>& object);

	/**
	 * @brief 一時的にオブジェクトを選択します。
	 * @param object 一時的に選択するオブジェクト
	 */
	void SelectTemp(const std::shared_ptr<GameObject>& object, bool additive = false);

	/**
	 * @brief 一時的にオブジェクトの範囲選択を行います。選択されているオブジェクトと指定されたオブジェクトの間にあるオブジェクトを一時的に選択します。
	 * @param object 範囲選択の終点となるオブジェクト
	 * @param flatList オブジェクトのフラットなリスト（階層構造を無視したリスト）。範囲選択はこのリストの順序に基づいて行われます。
	 */
	void SelectTempRange(const std::shared_ptr<GameObject>& object, const std::vector<std::shared_ptr<GameObject>>& flatList);

	/**
	 * @brief 一時的に選択されたオブジェクトを確定します。これにより、一時的な選択が正式な選択に変わります。
	 * @param additive `true` の場合、既存の選択に追加します。`false` の場合、既存の選択をクリアしてから選択します。
	 */
	void CommitTempSelection(bool additive = false);

	/**
	 * @brief 現在の選択をロックします。これにより、選択状態が変更されなくなります。
	 */
	void LockCurrentSelection();

	/**
	 * @brief 現在の選択のロックを解除します。これにより、選択状態が変更可能になります。
	 */
	void UnlockCurrentSelection();

	/**
	 * @brief 全ての選択をクリアします。
	 */
	void Clear();

	/**
	 * @brief オブジェクトが選択されているか確認します。
	 * @param object 確認するオブジェクト
	 * @return 選択されている場合は `true`、そうでない場合は `false`
	 */
	bool IsSelected(const std::shared_ptr<GameObject>& object) const;
	bool IsSelected(const GameObject* object) const;

	/**
	 * @brief 選択が空か確認します。
	 * @return 選択が空の場合は `true`、そうでない場合は `false`
	 */
	bool IsEmpty() const;

	/**
	 * @brief 選択されているオブジェクトの数を取得します。
	 * @return 選択されているオブジェクトの数
	 */
	int Count() const;

	/**
	 * @brief 選択されているオブジェクトのリストを取得します。
	 * @return 選択されているオブジェクトのリスト
	 */
	std::vector<std::shared_ptr<GameObject>> GetAll() const;

	/**
	 * @brief 選択されているオブジェクトのリストを取得します。
	 * @return 選択されているオブジェクトのリスト
	 */
	const std::vector<std::shared_ptr<GameObject>>& GetSelectedAll() const;

	/**
	 * @brief ロックされている選択状態のオブジェクトのリストを取得します。
	 * @return ロックされている選択状態のオブジェクトのリスト
	 */
	const std::vector<std::shared_ptr<GameObject>>& GetLockedAll() const;

	/**
	 * @brief 選択されているオブジェクトのうち、最後に選択されたものを取得します。(いわゆる「主選択」)
	 * @return 最後に選択されているオブジェクト。選択が空の場合は `nullptr`
	 */
	std::shared_ptr<GameObject> GetPrimary() const;

	/**
	 * @brief ロックされている選択状態のうち、最後に選択されたものを取得します。(いわゆる「主選択」)
	 * @return 最後にロックされている選択状態のオブジェクト。ロックされている選択が空の場合は `nullptr`
	 */
	std::shared_ptr<GameObject> GetPrimarySelected() const;

	/**
	 * @brief ロックされている選択状態のうち、最後に選択されたものを取得します。(いわゆる「主選択」)
	 * @return 最後にロックされている選択状態のオブジェクト。ロックされている選択が空の場合は `nullptr`
	 */
	std::shared_ptr<GameObject> GetPrimaryLocked() const;

private:

	/**
	 * @brief ロックされた選択状態を同期します。これにより、ロックされた選択状態が現在の選択状態と一致するようになります。
	 */
	void SyncLockedSelection();

private:
	// TODO: あとで選択状態の管理の型を見直すこと。
	std::vector<std::shared_ptr<GameObject>> m_selected;
	std::vector<std::weak_ptr<GameObject>> m_tempSelected; // 一時的に選択されているオブジェクトのリスト（ドラッグ・リリースの間などで使用）
	std::vector<std::weak_ptr<GameObject>> m_tempDeselected; // 一時的に選択解除されているオブジェクトのリスト（ドラッグ・リリースの間などで使用）
	std::vector<std::weak_ptr<GameObject>> m_lockedSelected; // 選択がロックされているオブジェクトのリスト（選択状態を維持するために使用）
	bool m_isPreTempCommitInitClear = false; // 一時的な選択を確定する前に、既存の選択をクリアするかどうかのフラグ
	bool m_isLocked = false; // 選択がロックされているかどうかのフラグ
};
