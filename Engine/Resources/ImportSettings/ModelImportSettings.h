#pragma once
#include "IImportSettings.h"

namespace CurryEngine::Resources
{
	/**
	 * @brief モデルのインポート設定を表す構造体。モデルアセットのインポート時に使用される設定を保持します。
	 * @details この構造体は、モデルアセットのインポートに関連する設定をまとめて管理するためのものです。例えば、法線や接線の生成、スケーリングなど、モデルの品質やパフォーマンスに影響を与える設定を含みます。
	 */
	struct ModelImportSettings : public IImportSettings
	{
		float scaleFactor = 1.0f; ///< モデルのスケーリング係数
		bool staticBatching = false; ///< モデルの静的バッチングを有効にするかどうか

		// nlohmann::jsonのNLOHMANN_DEFINE_TYPE系マクロでto_json/from_jsonを自動生成
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(ModelImportSettings, scaleFactor, staticBatching)
	};
}