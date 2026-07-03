#pragma once
#include <string>
#include <json.hpp>

namespace CurryEngine
{
	namespace Resources
	{
		/**
		 * @brief アセットIDを表す構造体。アセットの一意な識別子として使用されます。
		 * @details この構造体は、アセットの管理や参照時に使用されるIDを提供します。
		 */
		struct AssetId
		{
			std::string id; ///< アセットの一意な識別子
			/**
			 * @brief デフォルトコンストラクタ。空のIDを初期化します。
			 */
			AssetId() = default;
			/**
			 * @brief コンストラクタ。指定されたIDで初期化します。
			 * @param assetId アセットの一意な識別子
			 */
			explicit AssetId(const std::string& assetId) : id(assetId) {}
			/**
			 * @brief IDが有効かどうかを判定します。
			 * @return 有効なIDの場合はtrue、無効な場合はfalseを返します。
			 */
			bool IsValid() const { return !id.empty(); }
			/**
			 * @brief IDを文字列として取得します。
			 * @return アセットの一意な識別子
			 */
			const std::string& ToString() const { return id; }

			/**
			 * @brief 等価演算子。2つのAssetIdが同じIDを持つかどうかを判定します。
			 * @param other 比較対象のAssetId
			 * @return 同じIDの場合はtrue、異なる場合はfalseを返します。
			 */
			bool operator==(const AssetId& other) const { return id == other.id; }

			/**
			 * @brief 不等価演算子。2つのAssetIdが異なるIDを持つかどうかを判定します。
			 * @param other 比較対象のAssetId
			 * @return 異なるIDの場合はtrue、同じ場合はfalseを返します。
			 */
			bool operator!=(const AssetId& other) const { return id != other.id; }

			/**
			 * @brief JSONシリアライズ関数。AssetIdをJSON形式に変換します。
			 * @param j JSONオブジェクト
			 * @param assetId シリアライズするAssetId
			 */
			friend void to_json(nlohmann::json& j, const AssetId& assetId)
			{
				j = assetId.id;
			}
			/**
			 * @brief JSONデシリアライズ関数。JSON形式からAssetIdに変換します。
			 * @param j JSONオブジェクト
			 * @param assetId デシリアライズするAssetId
			 */
			friend void from_json(const nlohmann::json& j, AssetId& assetId)
			{
				assetId.id = j.get<std::string>();
			}
		};
	}
}

namespace std
{
	template<>
	struct hash<CurryEngine::Resources::AssetId>
	{
		std::size_t operator()(const CurryEngine::Resources::AssetId& assetId) const noexcept
		{
			return std::hash<std::string>{}(assetId.id);
		}
	};
}