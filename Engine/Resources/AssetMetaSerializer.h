#pragma once
#include <string>
#include "AssetMeta.h"

namespace CurryEngine::Resources
{
	class AssetMetaSerializer
	{
	public:
		/**
		 * @brief アセットメタデータを保存する関数。
		 * @param meta 保存するアセットメタデータ。
		 * @return 保存が成功した場合はtrue、失敗した場合はfalseを返します。
		 */
		static bool Save(const AssetMeta& meta);
		/**
		 * @brief アセットメタデータを読み込む関数。
		 * @param assetPath 読み込むアセットのパス。
		 * @return 読み込んだアセットメタデータ。読み込みに失敗した場合は空のAssetMetaを返します。
		 */
		static AssetMeta Load(const std::filesystem::path& assetPath);


		/**
		 * @brief アセットのパスから対応するメタデータファイルのパスを取得する関数。
		 * @param assetPath アセットのパス。
		 * @return 対応するメタデータファイルのパス。
		 */
		static std::filesystem::path MetaPathFor(const std::filesystem::path& assetPath);

		/**
		 * @brief 文字列からAssetTypeを取得する関数。
		 * @param typeStr AssetTypeを表す文字列。
		 * @return 対応するAssetType。文字列が不正な場合はAssetType::Unknownを返します。
		 */
		static AssetType AssetTypeFromString(const std::string& typeStr);

		/**
		 * @brief AssetTypeを文字列に変換する関数。
		 * @param type 変換するAssetType。
		 * @return 対応する文字列。AssetTypeが不正な場合は"Unknown"を返します。
		 */
		static std::string AssetTypeToString(AssetType type);

	};
}