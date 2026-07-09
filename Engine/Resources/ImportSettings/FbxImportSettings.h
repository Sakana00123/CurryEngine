#pragma once
#include "IImportSettings.h"

namespace CurryEngine
{
	namespace Resources
	{
		enum class FbxImportScaleMode
		{
			None, // スケーリングなし
			Millimeters, // ミリメートル単位
			Centimeters, // センチメートル単位
			Meters, // メートル単位
			Kilometers, // キロメートル単位
			Inches, // インチ単位
			Feet, // フィート単位
			Yards, // ヤード単位
			Miles // マイル単位
		};
		NLOHMANN_JSON_SERIALIZE_ENUM(FbxImportScaleMode, {
			{FbxImportScaleMode::None, "None"},
			{FbxImportScaleMode::Millimeters, "Millimeters"},
			{FbxImportScaleMode::Centimeters, "Centimeters"},
			{FbxImportScaleMode::Meters, "Meters"},
			{FbxImportScaleMode::Kilometers, "Kilometers"},
			{FbxImportScaleMode::Inches, "Inches"},
			{FbxImportScaleMode::Feet, "Feet"},
			{FbxImportScaleMode::Yards, "Yards"},
			{FbxImportScaleMode::Miles, "Miles"}
			});

		/**
		 * @brief FBXインポート設定を表す構造体。
		 * @details この構造体は、FBXファイルのインポート時に使用される設定を保持します。具体的には、スケーリングやアニメーションのサンプリングレートなど、FBXファイルの読み込みに関するパラメータを管理します。
		 */
		struct FbxImportSettings : public IImportSettings
		{
			/** @brief インポート時のスケーリングモード。*/
			FbxImportScaleMode scale{ FbxImportScaleMode::None };
			/** @brief アニメーションのサンプリングレート（フレーム/秒）。*/
			float animationSamplingRate{ 60.0f };
			
			friend void to_json(nlohmann::json& j, const FbxImportSettings& settings) {
				j = nlohmann::json{
					{"scale", settings.scale},
					{"animationSamplingRate", settings.animationSamplingRate}
				};
			}
			friend void from_json(const nlohmann::json& j, FbxImportSettings& settings) {
				if (j.contains("scale")) j.at("scale").get_to(settings.scale);
				if (j.contains("animationSamplingRate")) j.at("animationSamplingRate").get_to(settings.animationSamplingRate);
			}
		};
	}
}
