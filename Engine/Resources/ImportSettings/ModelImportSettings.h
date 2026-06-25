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
		bool makeLeftHanded = false; ///< 左手座標系に変換するかどうか
		bool flipUVs = true; ///< UV座標を反転するかどうか
		bool flipWindingOrder = true; ///< 頂点の順序を反転するかどうか（法線の向きに影響）
		bool optimizeMesh = true; ///< メッシュの最適化を行うかどうか

		// nlohmann::jsonのNLOHMANN_DEFINE_TYPE系マクロでto_json/from_jsonを自動生成
		friend void to_json(nlohmann::json& j, const ModelImportSettings& settings) {
			j = nlohmann::json{
				{"scaleFactor", settings.scaleFactor},
				{"staticBatching", settings.staticBatching},
				{"makeLeftHanded", settings.makeLeftHanded},
				{"flipUVs", settings.flipUVs},
				{"flipWindingOrder", settings.flipWindingOrder},
				{"optimizeMesh", settings.optimizeMesh}
			};
		}
		friend void from_json(const nlohmann::json& j, ModelImportSettings& settings) {
			if (j.contains("scaleFactor")) j.at("scaleFactor").get_to(settings.scaleFactor);
			if (j.contains("staticBatching")) j.at("staticBatching").get_to(settings.staticBatching);
			if (j.contains("makeLeftHanded")) j.at("makeLeftHanded").get_to(settings.makeLeftHanded);
			if (j.contains("flipUVs")) j.at("flipUVs").get_to(settings.flipUVs);
			if (j.contains("flipWindingOrder")) j.at("flipWindingOrder").get_to(settings.flipWindingOrder);
			if (j.contains("optimizeMesh")) j.at("optimizeMesh").get_to(settings.optimizeMesh);
		}
	};
}