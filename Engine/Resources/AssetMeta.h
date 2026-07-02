#pragma once
#include "AssetId.h"
#include <string>
#include <json.hpp>
#include "AssetType.h"
#include "Engine/Resources/ImportSettings/IImportSettings.h"

namespace CurryEngine
{
	namespace Resources
	{

		/**
		 * @brief アセットメタデータを表す構造体。アセットの種類やパスなどの情報を保持します。
		 * @details この構造体は、アセットの管理やロード時に使用されるメタデータを提供します。
		 */
		struct AssetMeta
		{
			AssetId id; ///< アセットの一意な識別子
			std::filesystem::path path; ///< アセットのファイルパス
			AssetType type; ///< アセットの種類
			bool isFolder = false; ///< アセットがフォルダかどうかを示すフラグ
			nlohmann::json importSettings;

			/**
			 * @brief デフォルトコンストラクタ。空のメタデータを初期化します。
			 */
			AssetMeta() = default;
			/**
			 * @brief コンストラクタ。指定されたID、パス、種類で初期化します。
			 * @param assetId アセットの一意な識別子
			 * @param assetPath アセットのファイルパス
			 * @param assetType アセットの種類
			 */
			AssetMeta(const AssetId& assetId, const std::filesystem::path& assetPath, AssetType assetType, bool isFolder = false, const nlohmann::json& settings = nlohmann::json())
				: id(assetId), path(assetPath), type(assetType), isFolder(isFolder), importSettings(settings) {
			}
			
			template<typename T>
			T GetImportSettings() const
			{
				static_assert(std::is_base_of_v<IImportSettings, T>, "T must derive from IImportSettings");
				if (importSettings.is_null())
				{
					return T(); // デフォルトコンストラクタで初期化されたTを返す
				}
				return importSettings.get<T>();
			}

			template<typename T>
			void SetImportSettings(const T& settings)
			{
				static_assert(std::is_base_of_v<IImportSettings, T>, "T must derive from IImportSettings");
				importSettings = settings; // to_json/from_jsonがあれば自動変換される
			}

		};
	}
}