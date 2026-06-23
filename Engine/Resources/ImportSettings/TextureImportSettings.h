#pragma once
#include "IImportSettings.h"

namespace CurryEngine::Resources
{
	enum class TextureImporterType
	{
		Default, // デフォルトのテクスチャインポーター。一般的なテクスチャのインポートに使用される設定を提供します。
		NormalMap, // 法線マップ用のテクスチャインポーター。法線マップの特性に合わせた設定を提供します。
		GUI, // GUI用のテクスチャインポーター。エディタのUIに使用されるテクスチャのインポートに適した設定を提供します。
		Sprite, // スプライト用のテクスチャインポーター。ゲームのスプライトに使用されるテクスチャのインポートに適した設定を提供します。
		SpriteAtlas, // スプライトアトラス用のテクスチャインポーター。複数のスプライトを1つのテクスチャにまとめるためのインポート設定を提供します。
		Cubemap, // キューブマップ用のテクスチャインポーター。環境マップやスカイボックスに使用されるテクスチャのインポートに適した設定を提供します。
	};
	NLOHMANN_JSON_SERIALIZE_ENUM(TextureImporterType, {
		{TextureImporterType::Default, "Default"},
		{TextureImporterType::NormalMap, "NormalMap"},
		{TextureImporterType::GUI, "GUI"},
		{TextureImporterType::Sprite, "Sprite"},
		{TextureImporterType::SpriteAtlas, "SpriteAtlas"},
		{TextureImporterType::Cubemap, "Cubemap"},
		});


	/**
	 * @brief テクスチャのインポート設定を表す構造体。テクスチャアセットのインポート時に使用される設定を保持します。
	 * @details この構造体は、テクスチャアセットのインポートに関連する設定をまとめて管理するためのものです。例えば、ミップマップの生成や圧縮形式など、テクスチャの品質やパフォーマンスに影響を与える設定を含みます。
	 */
	struct TextureImportSettings : public IImportSettings
	{
		TextureImporterType textureType = TextureImporterType::Default; // インポーターの種類を指定するフィールドを追加
		bool generateMipmaps = true;
		std::string compression = "BC7";

		// nlohmann::jsonのNLOHMANN_DEFINE_TYPE系マクロでto_json/from_jsonを自動生成
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(TextureImportSettings, textureType, generateMipmaps, compression)
	};
}