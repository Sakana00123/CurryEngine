#pragma once
#include <string>
#include <unordered_map>
#include "AssetMeta.h"
#include "ResourceManager.h"
#include "Importers/ImporterRegistry.h"
//#include "AssetWatcher.h"

namespace CurryEngine
{
	namespace Resources
	{
		class AssetDatabase
		{
		public:
			/**
			 * @brief アセットデータベースを初期化します。
			 * @param assetRootDir アセットのルートディレクトリのパス
			 */
			static void Initialize(const std::string& assetRootDir);

			/**
			 * @brief アセットデータベースを終了します。
			 */
			static void Finalize();

			/**
			 * @brief アセットのルートディレクトリを取得します。
			 * @return アセットのルートディレクトリのパス
			 */
			static const std::string& GetAssetRootDir();

			/**
			 * @brief 既存の.metaファイルを読み込み、アセットデータベースを構築します。
			 * @details 例えば、アセットルートディレクトリ以下のすべての.metaファイルを読み込んで、s_metaByPathやs_pathByIdなどのマップを更新します。
			 */
			static void LoadExistingMetaFiles();

			/**
			 * @brief アセットデータベースの整合性を検証します。必要に応じて、アセットの存在やメタデータの正当性をチェックします。
			 * @details 例えば、アセットファイルが存在しない場合や、メタデータが不正な場合に警告を出すなどの処理を行います。
			 */
			static void ValidateOnStartup();

			/**
			 * @brief 指定されたアセットIDのアセットをロードします。
			 * @tparam T ロードするアセットの型
			 * @param id ロードするアセットの一意な識別子
			 * @return ロードしたアセットの共有ポインタ、存在しない場合はnullptr
			 */
			template<typename T>
			static std::shared_ptr<T> LoadAsset(const AssetId& id)
			{
				const AssetMeta* meta = Find(id);
				if (meta)
				{
					// アセットタイプに対応するインポーターを取得
					if (IImporter* importer = ImporterRegistry::Find(meta->type))
					{
						auto resource = importer->Import(*meta);
						ResourceManager::Register(meta->path, resource);
						return std::dynamic_pointer_cast<T>(resource);
					}

					LOG_WARNING("[AssetDatabase] No importer found for asset type: " + std::to_string(static_cast<int>(meta->type)) + ". Asset path: " + meta->path);
					return nullptr;
				}
				else
				{
					LOG_ERROR("[AssetDatabase] Failed to load asset. AssetId not found: " + id.ToString());
					return nullptr;
				}
			}

			/**
			 * @brief 指定されたアセットパスのアセットをインポートします。
			 * @param assetPath インポートするアセットのファイルパス
			 */
			static AssetMeta* Import(const std::string& assetPath);

			/**
			 * @brief 指定されたアセットパスのアセットを取得します。存在しない場合はインポートします。
			 * @param assetPath 取得またはインポートするアセットのファイルパス
			 */
			static AssetMeta* GetOrImport(const std::string& assetPath);

			/**
			 * @brief 指定されたアセットIDのアセットを取得します。存在しない場合はnullptrを返します。
			 * @param id 取得するアセットの一意な識別子
			 * @return アセットのメタデータへのポインタ、存在しない場合はnullptr
			 */
			static const AssetMeta* Find(const AssetId& id);

			/**
			 * @brief 指定されたアセットIDのアセットを取得します。存在しない場合はnullptrを返します。
			 * @param id 取得するアセットの一意な識別子
			 * @return アセットのメタデータへのポインタ、存在しない場合はnullptr
			 * @details Findと異なり、こちらは書き込み可能なAssetMeta*を返します。アセットのインポート設定を変更する場合などに使用します。
			 */
			static AssetMeta* FindMutable(const AssetId& id);

			/**
			 * @brief 指定されたアセットパスのアセットを取得します。存在しない場合はnullptrを返します。
			 * @param assetPath 取得するアセットのファイルパス
			 * @return アセットのメタデータへのポインタ、存在しない場合はnullptr
			 */
			static const AssetMeta* FindByPath(const std::string& assetPath);

			/**
			 * @brief アセットデータベースを再構築します。すべてのアセットを再スキャンしてメタデータを更新します。
			 * @details 例えば、アセットの移動や削除があった場合に、データベースの整合性を保つために呼び出されます。
			 */
			static void ReBuild();

			/**
			 * @brief 指定されたアセットパスのアセットをリネームします。
			 * @param oldPath 旧アセットのファイルパス
			 * @param newPath 新アセットのファイルパス
			 */
			static void Rename(const std::string& oldPath, const std::string& newPath);

			/**
			 * @brief 指定されたアセットパスのプレフィックスを新しいプレフィックスにリマップします。
			 * @param oldPrefix 旧プレフィックス
			 * @param newPrefix 新プレフィックス
			 * @details 例えば、oldPrefixが"Assets/Textures/"でnewPrefixが"Assets/Images/"の場合、"Assets/Textures/wood.png"は"Assets/Images/wood.png"にリマップされます。
			 */
			static void RemapPathPrefix(const std::string& oldPrefix, const std::string& newPrefix);

			/**
			 * @brief 指定されたアセットIDのアセットを削除します。
			 * @param id 削除するアセットの一意な識別子
			 */
			static void Remove(const AssetId& id);

			/**
			 * @brief 指定されたアセットパスのアセットを削除します。
			 * @param assetPath 削除するアセットのファイルパス
			 */
			static void RemoveByPath(const std::string& assetPath);

			/**
			 * @brief 指定されたアセットパスのプレフィックスに一致するすべてのアセットを削除します。
			 * @param pathPrefix 削除するアセットのパスのプレフィックス
			 * @details 例えば、pathPrefixが"Assets/Textures/"の場合、"Assets/Textures/wood.png"や"Assets/Textures/stone.png"など、すべての一致するアセットが削除されます。
			 */
			static void RemoveByPathPrefix(const std::string& pathPrefix);

		private:
			static std::unordered_map<std::string, AssetMeta> s_metaByPath; ///< アセットパスをキーとしたアセットメタデータのマップ
			static std::unordered_map<std::string, std::string> s_pathById; ///< アセットIDをキーとしたアセットパスのマップ
			static std::string s_assetRootDir; ///< アセットのルートディレクトリのパス
			static bool s_initialized; ///< アセットデータベースが初期化されているかどうか

			//static inline AssetWatcher s_assetWatcher; ///< アセットの変更を監視するAssetWatcherのインスタンス
		};
	}
}