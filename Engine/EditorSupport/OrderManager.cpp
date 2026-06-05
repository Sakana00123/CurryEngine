#include "pch.h"
#include "OrderManager.h"

namespace CurryEngine
{
	int OrderManager::CalcInsertPriority(int prevPriority, int nextPriority)
	{
		if (prevPriority < 0 && nextPriority < 0) {
			// —¼•û‚Æ‚à•‰‚Ì’l‚Ìê‡‚ÍASTEP ‚ðŠî€‚ÉV‚µ‚¢—Dæ‡ˆÊ‚ðŒvŽZ
			return 0; // —á‚¦‚ÎA—¼•û‚ª•‰‚Ìê‡‚Í 0 ‚ð•Ô‚·‚È‚ÇA“KØ‚È‰Šú’l‚ð•Ô‚·
		}
		if (prevPriority < 0) {
			// ‘O‚Ì—Dæ‡ˆÊ‚ª•‰‚Ìê‡‚ÍAŽŸ‚Ì—Dæ‡ˆÊ‚©‚ç STEP ‚ðˆø‚¢‚ÄV‚µ‚¢—Dæ‡ˆÊ‚ðŒvŽZ
			return nextPriority - STEP;
		}
		if (nextPriority < 0) {
			// ŽŸ‚Ì—Dæ‡ˆÊ‚ª•‰‚Ìê‡‚ÍA‘O‚Ì—Dæ‡ˆÊ‚É STEP ‚ð‰Á‚¦‚ÄV‚µ‚¢—Dæ‡ˆÊ‚ðŒvŽZ
			return prevPriority + STEP;
		}

		// ‡˜‚ÌƒMƒƒƒbƒv‚ðŒvŽZ
		int gap = nextPriority - prevPriority;
		if (gap <= MIN_GAP) {
			return -1; // ƒMƒƒƒbƒv‚ª¬‚³‚·‚¬‚éê‡‚Í‘}“ü‚Å‚«‚È‚¢‚±‚Æ‚ðŽ¦‚·
		}

		// ‘O‚Ì—Dæ‡ˆÊ‚ÆŽŸ‚Ì—Dæ‡ˆÊ‚Ì’†ŠÔ‚ðV‚µ‚¢—Dæ‡ˆÊ‚Æ‚µ‚Ä•Ô‚·
		return prevPriority + gap / 2;
	}

}