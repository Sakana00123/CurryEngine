#pragma once
#include <json.hpp>

namespace CurryEngine
{
	namespace Resources
	{
		/**
		 * @brief インポート設定のインターフェース。アセットのインポート時に使用される設定を表す抽象クラス。
		 * @details このクラスは、アセットの種類ごとに異なるインポート設定を表すための基底クラスとして機能します。具体的なアセットタイプ（例: テクスチャ、モデル、シーン）ごとにこのインターフェースを実装することで、異なるフォーマットのアセットに対して統一的なインポート設定の管理が可能になります。
		 */
		struct IImportSettings
		{
		};
	}
}