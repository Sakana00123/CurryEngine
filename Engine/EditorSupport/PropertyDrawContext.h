#pragma once
#include <vector>

class Object;

/**
 * @brief プロパティ描画のコンテキストを表す構造体。プロパティ描画関数に渡され、描画対象のオブジェクトや複数選択の状態を提供します。
 * @details 複数選択に対応しており、targets に描画対象のオブジェクトが格納されます。isMultiSelect フラグは targets のサイズに基づいて自動的に設定されます。
 */
struct PropertyDrawContext
{
	/**
	 * @brief プロパティを描画する対象のオブジェクトのリスト。複数選択に対応するため、ベクターで保持しています。
	 * @details targets[0] が表示の基準値となります。
	 */
	std::vector<Object*> targets;

	/**
	 * @brief 複数選択されているかどうか。targets のサイズが 2 以上の場合は true になります。
	 * @details 複数選択されている場合、プロパティの描画や編集の際に targets[0] を基準にして、他の targets[i] と比較しながら描画することができます。
	 */
	bool isMultiSelect = false;


	// --- 静的ファクトリーメソッド ---

	/** @brief 単一選択の PropertyDrawContext を作成するファクトリーメソッド。*/
	static PropertyDrawContext MakeSingle(Object* target);
	/** @brief 複数選択の PropertyDrawContext を作成するファクトリーメソッド。*/
	static PropertyDrawContext MakeMulti(const std::vector<Object*>& targets);

	// --- ユーティリティメソッド ---

	/** @brief targets[0] を返す。targets が空の場合は nullptr を返します。*/
	Object* Primary() const;

	/** @brief targets が空かどうかを返す。*/
	bool IsEmpty() const;
};