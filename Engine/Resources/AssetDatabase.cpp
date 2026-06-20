#include "pch.h"
#include "AssetDatabase.h"
#include "AssetMetaSerializer.h"
#include "Engine/Utils/AssetIdGenerator.h"
#include "Engine/Resources/AssetTypeUtils.h"

#include <filesystem>

namespace CurryEngine
{
	namespace Resources
	{
		std::unordered_map<std::string, AssetMeta> AssetDatabase::s_metaByPath; ///< アセットパスをキーとしたアセットメタデータのマップ
		std::unordered_map<std::string, std::string> AssetDatabase::s_pathById; ///< アセットIDをキーとしたアセットパスのマップ
		std::string AssetDatabase::s_assetRootDir; ///< アセットのルートディレクトリのパス
		bool AssetDatabase::s_initialized = false; ///< アセットデータベースが初期化されているかどうか


		namespace
		{
			std::string NormalizePath(const std::string& path)
			{
				std::string normalized = path;
				std::replace(normalized.begin(), normalized.end(), '\\', '/'); // Windowsのパス区切りを統一
				return normalized;
			}
		}


		void AssetDatabase::Initialize(const std::string& assetRootDir)
		{
			s_assetRootDir = assetRootDir;

			//s_assetWatcher.Start(assetRootDir);

			// 既存の.metaファイルを読み込んでデータベースを構築
			LoadExistingMetaFiles();

			// データベースの整合性を検証
			ValidateOnStartup();
			s_initialized = true;
		}

		void AssetDatabase::Finalize()
		{
			//s_assetWatcher.Stop();

			s_metaByPath.clear();
			s_pathById.clear();
			s_initialized = false;
		}

		const std::string& AssetDatabase::GetAssetRootDir()
		{
			return s_assetRootDir;
		}

		void AssetDatabase::LoadExistingMetaFiles()
		{
			for (const auto& entry : std::filesystem::recursive_directory_iterator(s_assetRootDir))
			{
				if (entry.is_regular_file() && entry.path().extension() == ".meta")
				{
					// 対応する実ファイルのパス（.metaを除いたパス）
					std::string assetPath = NormalizePath(
						entry.path().string().substr(0, entry.path().string().size() - 5)); // ".meta"の5文字を除く

					AssetMeta meta = AssetMetaSerializer::Load(assetPath);
					if (!meta.id.IsValid()) continue; // 読み込み失敗はスキップ

					s_metaByPath[assetPath] = meta;
					s_pathById[meta.id.ToString()] = assetPath;
				}
			}
		}

		void AssetDatabase::ValidateOnStartup()
		{
			std::vector<std::string> orphanedMetas;   // .metaはあるが実ファイルがない
			std::vector<std::string> missingMetas;    // 実ファイルはあるが.metaがない
			std::unordered_map<std::string, std::vector<std::string>> duplicateIds; // ID重複

			// s_metaByPathを走査
			for (auto& [path, meta] : s_metaByPath)
			{
				// 実ファイルの存在確認
				if (!std::filesystem::exists(path))
					orphanedMetas.push_back(path);

				// ID重複確認
				duplicateIds[meta.id.ToString()].push_back(path);
			}

			// ルートディレクトリ以下を走査して.metaのない実ファイルを検出
			for (const auto& entry : std::filesystem::recursive_directory_iterator(s_assetRootDir))
			{
				if (!entry.is_regular_file()) continue;
				std::string path = NormalizePath(entry.path().string());

				// .metaファイル自体はスキップ
				if (entry.path().extension() == ".meta") continue;

				if (s_metaByPath.find(path) == s_metaByPath.end())
					missingMetas.push_back(path);
			}

			// 結果をログ出力
			for (const auto& path : orphanedMetas)
				LOG_WARNING("Orphaned .meta (no asset file): " + path);

			for (const auto& path : missingMetas)
			{
				// 自動修復: .metaがないなら新規Import
				LOG_WARNING("Missing .meta, auto-importing: " + path);
				Import(path);
			}

			for (const auto& [id, paths] : duplicateIds)
			{
				if (paths.size() > 1)
				{
					// ID重複は重大な問題なのでエラーとしてログ出力
					LOG_ERROR("Duplicate AssetId detected: " + id);
					for (const auto& p : paths)
						LOG_ERROR("  -> " + p);
				}
			}
		}

		AssetMeta* AssetDatabase::Import(const std::string& assetPath)
		{
			// Check if the asset is already imported
			std::string normalizedPath = NormalizePath(assetPath);

			AssetMeta meta = AssetMetaSerializer::Load(normalizedPath);
			if (meta.id.IsValid()) {
				// .metaがある = 既存アセット。IDを再利用
			}
			else {
				// .metaが無い = 新規アセット。IDを新規発行
				AssetId newId(Utils::IdGenerator::GenerateAssetId());
				AssetType inferredType = AssetTypeUtils::DetectFromExtension(
					std::filesystem::path(normalizedPath).extension().string()); // 拡張子から推測
				nlohmann::json defaultSettings = nlohmann::json(); // デフォルトのインポート設定（必要に応じて変更）

				meta = AssetMeta(newId, normalizedPath, inferredType, defaultSettings);
				AssetMetaSerializer::Save(meta);
			}

			s_metaByPath[normalizedPath] = meta;
			s_pathById[meta.id.ToString()] = normalizedPath;
			return &s_metaByPath[normalizedPath];
		}

		AssetMeta* AssetDatabase::GetOrImport(const std::string& assetPath)
		{
			std::string normalizedPath = NormalizePath(assetPath);

			auto it = s_metaByPath.find(normalizedPath);
			if (it != s_metaByPath.end())
			{
				return &it->second;
			}
			else
			{
				return Import(normalizedPath);
			}
		}

		const AssetMeta* AssetDatabase::Find(const AssetId& id)
		{
			auto it = s_pathById.find(id.ToString());
			if (it != s_pathById.end())
			{
				return FindByPath(it->second);
			}
			return nullptr;
		}

		AssetMeta* AssetDatabase::FindMutable(const AssetId& id)
		{
			auto it = s_pathById.find(id.ToString());
			if (it != s_pathById.end())
			{
				auto metaIt = s_metaByPath.find(it->second);
				if (metaIt != s_metaByPath.end())
				{
					return &metaIt->second;
				}
			}
			return nullptr;
		}

		const AssetMeta* AssetDatabase::FindByPath(const std::string& assetPath)
		{
			std::string normalizedPath = NormalizePath(assetPath);

			auto it = s_metaByPath.find(normalizedPath);
			if (it != s_metaByPath.end())
			{
				return &it->second;
			}
			return nullptr;
		}

		void AssetDatabase::ReBuild()
		{
			s_metaByPath.clear();
			s_pathById.clear();
			
			// アセットルートディレクトリ以下のすべての.metaファイルを再読み込みしてデータベースを再構築
			LoadExistingMetaFiles();
			// データベースの整合性を再検証
			ValidateOnStartup();
		}

		void AssetDatabase::Rename(const std::string& oldPath, const std::string& newPath)
		{
			std::string normalizedOldPath = NormalizePath(oldPath);
			std::string normalizedNewPath = NormalizePath(newPath);

			auto it = s_metaByPath.find(normalizedOldPath);
			if (it == s_metaByPath.end())
			{
				// 古いパスが見つからない場合は何もしない
				LOG_WARNING("[AssetDatabase] Rename failed: old path not found: " + normalizedOldPath);
				return;
			}

			AssetMeta meta = it->second;
			std::filesystem::rename(
				AssetMetaSerializer::MetaPathFor(normalizedOldPath),
				AssetMetaSerializer::MetaPathFor(normalizedNewPath)
			);

			meta.path = normalizedNewPath;
			s_metaByPath.erase(it);
			s_metaByPath[normalizedNewPath] = meta;
			s_pathById[meta.id.ToString()] = normalizedNewPath; // IDは変わらないので、パスだけ更新
			// 変更後のパスでメタデータを保存
			AssetMetaSerializer::Save(meta);
		}

		void AssetDatabase::RemapPathPrefix(const std::string& oldPrefix, const std::string& newPrefix)
		{
			std::string normalizedOldPrefix = NormalizePath(oldPrefix);
			std::string normalizedNewPrefix = NormalizePath(newPrefix);

			std::vector<std::pair<std::string, AssetMeta>> entriesToRename;
			for (auto it = s_metaByPath.begin(); it != s_metaByPath.end(); )
			{
				if (it->first.starts_with(normalizedOldPrefix))
				{
					AssetMeta meta = it->second;
					std::string newPath = normalizedNewPrefix + it->first.substr(normalizedOldPrefix.size());
					meta.path = newPath;
					entriesToRename.emplace_back(newPath, meta); // 新しいキーと更新されたメタデータを保存
					it = s_metaByPath.erase(it); // 後で新しいキーで再挿入するため、ここで削除
				}
				else ++it;
			}
			for (const auto& [newPath, meta] : entriesToRename)
			{
				AssetMetaSerializer::Save(meta); // メタデータを保存
				s_metaByPath[newPath] = meta; // 新しいキーで再挿入
				s_pathById[meta.id.ToString()] = newPath; // IDは変わらないので、パスだけ更新
			}
		}

		void AssetDatabase::Remove(const AssetId& id)
		{
			auto it = s_pathById.find(id.ToString());
			if (it != s_pathById.end())
			{
				RemoveByPath(it->second);
			}
		}

		void AssetDatabase::RemoveByPath(const std::string& assetPath)
		{
			std::string normalizedPath = NormalizePath(assetPath);
			auto it = s_metaByPath.find(normalizedPath);
			if (it != s_metaByPath.end())
			{
				// メタデータファイルを削除
				std::filesystem::remove(AssetMetaSerializer::MetaPathFor(normalizedPath));

				// マップから削除
				s_pathById.erase(it->second.id.ToString());
				s_metaByPath.erase(it);
			}
		}

		void AssetDatabase::RemoveByPathPrefix(const std::string& pathPrefix)
		{
			std::string normalizedPrefix = NormalizePath(pathPrefix);
			for (auto it = s_metaByPath.begin(); it != s_metaByPath.end(); )
			{
				if (it->first.starts_with(normalizedPrefix))
				{
					// メタデータファイルを削除
					std::filesystem::remove(AssetMetaSerializer::MetaPathFor(it->first));
					// マップから削除
					s_pathById.erase(it->second.id.ToString());
					it = s_metaByPath.erase(it); // eraseはイテレータを返すので、次の要素に進む
				}
				else ++it;
			}
		}
	}
}