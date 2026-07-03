#include "pch.h"
#include "OrderManager.h"

namespace CurryEngine
{
	int OrderManager::CalcInsertPriority(int prevPriority, int nextPriority)
	{
		if (prevPriority < 0 && nextPriority < 0) {
			// 両方とも負の値の場合は、STEP を基準に新しい優先順位を計算
			return 0; // 例えば、両方が負の場合は 0 を返すなど、適切な初期値を返す
		}
		if (prevPriority < 0) {
			// 前の優先順位が負の場合は、次の優先順位から STEP を引いて新しい優先順位を計算
			return nextPriority - STEP;
		}
		if (nextPriority < 0) {
			// 次の優先順位が負の場合は、前の優先順位に STEP を加えて新しい優先順位を計算
			return prevPriority + STEP;
		}

		// 順序のギャップを計算
		int gap = nextPriority - prevPriority;
		if (gap <= MIN_GAP) {
			return -1; // ギャップが小さすぎる場合は挿入できないことを示す
		}

		// 前の優先順位と次の優先順位の中間を新しい優先順位として返す
		return prevPriority + gap / 2;
	}

}