#pragma once
#include <string>
#include <unordered_map>

namespace CurryEngine
{
	/**
	 * @brief PropertyDrawer の各インスタンスが持つ前フレーム値を保存するための構造体。
	 */
	template<typename T>
	struct DrawerState
	{
		std::unordered_map<std::string, T> previousValues; // プロパティ名をキーとして、前フレームの値を保存するマップ

		/**
		 * @brief 指定したプロパティの前フレーム値を取得する関数。
		 * @param propName 取得するプロパティの名前。
		 * @return 前フレームの値。プロパティが存在しない場合は T のデフォルト値を返す。
		 */
		T& Prev(const std::string& propName)
		{
			return previousValues[propName]; // 存在しないキーにアクセスした場合はデフォルト値が自動的に生成される
		}
	};
}