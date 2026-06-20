#pragma once
#include "IImportSettings.h"

namespace CurryEngine::Resources
{
	/**
	 * @brief テクスチャのインポート設定を表す構造体。テクスチャアセットのインポート時に使用される設定を保持します。
	 * @details この構造体は、テクスチャアセットのインポートに関連する設定をまとめて管理するためのものです。例えば、ミップマップの生成や圧縮形式など、テクスチャの品質やパフォーマンスに影響を与える設定を含みます。
	 */
	struct TextureImportSettings : public IImportSettings
	{
		bool generateMipmaps = true;
		std::string compression = "BC7";

		// nlohmann::jsonのNLOHMANN_DEFINE_TYPE系マクロでto_json/from_jsonを自動生成
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(TextureImportSettings, generateMipmaps, compression)
	};
}