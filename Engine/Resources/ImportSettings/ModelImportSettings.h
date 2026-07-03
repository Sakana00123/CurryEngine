#pragma once
#include "IImportSettings.h"
#include <assimp\postprocess.h>

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

		// assimp のインポートフラグをまとめたビットフィールド
		unsigned int importFlags =
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_JoinIdenticalVertices |
			aiProcess_LimitBoneWeights | // ボーン影響数を 4 に制限
			aiProcess_GlobalScale | // グローバルスケールを適用
			aiProcess_ConvertToLeftHanded; // 左手座標系に変換（DirectX用）

		// assimp のプリセットを選択するための変数（0: Custom, 1: Fast, 2: Quality, 3: MaxQuality）
		char preset = 1;

		// nlohmann::jsonのNLOHMANN_DEFINE_TYPE系マクロでto_json/from_jsonを自動生成
		friend void to_json(nlohmann::json& j, const ModelImportSettings& settings) {
			j = nlohmann::json{
				{"scaleFactor", settings.scaleFactor},
				{"staticBatching", settings.staticBatching},
				{"importFlags", settings.importFlags},
				{"preset", settings.preset }
			};
		}
		friend void from_json(const nlohmann::json& j, ModelImportSettings& settings) {
			if (j.contains("scaleFactor")) j.at("scaleFactor").get_to(settings.scaleFactor);
			if (j.contains("staticBatching")) j.at("staticBatching").get_to(settings.staticBatching);
			if (j.contains("importFlags")) j.at("importFlags").get_to(settings.importFlags);
			if (j.contains("preset")) j.at("preset").get_to(settings.preset);
		}
	};
}