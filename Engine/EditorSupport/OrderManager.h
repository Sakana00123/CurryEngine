#pragma once
#include <vector>
#include <algorithm>
#include "Engine/Core/Object.h"

namespace CurryEngine
{
	class OrderManager
	{
	public:
		static constexpr int STEP = 1000; // 順序のステップ値
		static constexpr int MIN_GAP = 1; // 順序の最小ギャップ値

		/// <summary>
		/// 新しいオブジェクトの優先順位を計算します。前の優先順位と次の優先順位の中間を計算し、順序のギャップが十分でない場合は -1 を返します。
		/// </summary>
		/// <param name="prevPriority">前のオブジェクトの優先順位。負の値の場合は、前のオブジェクトが存在しないことを示します。</param>
		/// <param name="nextPriority">次のオブジェクトの優先順位。負の値の場合は、次のオブジェクトが存在しないことを示します。</param>
		/// <returns>新しいオブジェクトの優先順位。順序のギャップが小さすぎる場合は -1 を返します。</returns>
		static int CalcInsertPriority(int prevPriority, int nextPriority);

		/// <summary>
		/// オブジェクトの優先順位に基づいてリストをソートします。これにより、優先順位の順序が正しく保たれます。
		/// </summary>
		/// <typeparam name="T">オブジェクトの型。Object クラスを継承している必要があります。</typeparam>
		/// <param name="objects">ソートするオブジェクトのリスト。</param>
		template<typename T>
		static void Sort(std::vector<std::shared_ptr<T>>& objects)
		{
			// 優先順位に基づいてオブジェクトを安定ソート
			std::stable_sort(objects.begin(), objects.end(),
				[](const std::shared_ptr<T>& a, const std::shared_ptr<T>& b) {
					return a->GetPriority() < b->GetPriority();
				});
		}

		/// <summary>
		/// オブジェクトの優先順位を再割り当てします。これにより、優先順位のギャップが解消され、新しいオブジェクトの挿入が容易になります。
		/// </summary>
		/// <typeparam name="T">オブジェクトの型。Object クラスを継承している必要があります。</typeparam>
		/// <param name="objects">優先順位を再割り当てするオブジェクトのリスト。</param>
		template<typename T>
		static void Renumber(std::vector<std::shared_ptr<T>>& objects)
		{
			// オブジェクトの優先順位を再割り当て
			for (size_t i = 0; i < objects.size(); ++i) {
				if (objects[i]) {
					objects[i]->SetPriority(static_cast<int>(i) * STEP);
				}
			}
		}

		/// <summary>
		/// オブジェクトを指定した位置に移動し、優先順位を適切に更新します。移動前と移動後の位置に基づいて新しい優先順位を計算し、必要に応じてリストをソートします。
		/// </summary>
		/// <typeparam name="T">オブジェクトの型。Object クラスを継承している必要があります。</typeparam>
		/// <param name="objects">移動するオブジェクトのリスト。</param>
		/// <param name="fromIndex">移動元のインデックス。</param>
		/// <param name="toIndex">移動先のインデックス。</param>
		/// <returns>移動後のオブジェクトの新しい優先順位。移動前と同じ位置の場合は、元の優先順位を返します。</returns>
		template<typename T>
		static int MoveObject(std::vector<std::shared_ptr<T>>& objects, size_t fromIndex, size_t toIndex)
		{
			if (fromIndex == toIndex) {
				return objects[fromIndex]->GetPriority(); // 移動前と同じ位置の場合は優先順位を変更しない
			}

			// fromIndex を除いた priority リストを作る
			std::vector<int> priorities;
			priorities.reserve(objects.size() - 1); // fromIndex を除くため、サイズは objects.size() - 1
			for (size_t i = 0; i < objects.size(); ++i) {
				if (i != fromIndex && objects[i]) {
					priorities.push_back(objects[i]->GetPriority());
				}
			}

			// toIndex を 除いた後のリスト上の挿入位置 に補正する
			// fromIndex より後ろに移動する場合は、toIndex は fromIndex を除いた後のリスト上では 1 つずれる
			size_t insertIndex = (fromIndex < toIndex) ? toIndex - 1 : toIndex;

			int prevPriority = (insertIndex > 0)
				? priorities[insertIndex - 1] : -1;
			int nextPriority = (insertIndex < priorities.size())
				? priorities[insertIndex] : -1;

			// 中間値を計算
			int newPriority = CalcInsertPriority(prevPriority, nextPriority);

			if (newPriority == -1) {
				// ギャップが小さすぎる場合は、優先順位を再割り当てしてから再度計算
				Renumber(objects);
				return MoveObject(objects, fromIndex, toIndex); // 1回だけ再帰的に呼び出して新しい優先順位を計算
			}

			objects[fromIndex]->SetPriority(newPriority);
			Sort(objects); // 優先順位に基づいてリストをソート
			return newPriority;
		}


	};
}