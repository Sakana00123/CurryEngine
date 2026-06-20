#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "AssetId.h"
#include <unordered_set>


namespace CurryEngine
{
	namespace Resources
	{
		/**
		 * @brief アセット参照インデックスを管理するクラス。アセットIDとその参照元のパスを関連付けます。
		 * @details このクラスは、アセットの依存関係や参照関係を追跡するために使用されます。
		 */
		class AssetReferenceIndex
		{
		public:
			/**
			 * @brief アセット参照インデックスを再構築します。指定されたスキャン対象のファイルから参照情報を収集します。
			 * @details 起動時やアセットの変更時に呼び出され、アセット参照インデックスを最新の状態に更新します。
			 * @note この関数は、スキャン対象のファイルが存在しない場合や、アセットIDが不正な場合には適切にエラーハンドリングを行う必要があります。
			 * @param scanTargetFiles スキャン対象のファイルパスのリスト
			 */
			static void RebuildFull(const std::vector<std::string>& scanTargetFiles);
			
			/**
			 * @brief 指定されたファイルに関連するアセット参照インデックスを再構築します。
			 * @details ファイルの変更や追加があった場合に呼び出され、該当するアセット参照情報を更新します。
			 * @param filePath 変更または追加されたファイルのパス
			 */
			static void RebuildForFile(const std::string& filePath);

			/**
			 * @brief 指定されたアセットIDを参照しているファイルのリストを取得します。
			 * @details アセットIDに関連する参照元のファイルパスを返します。参照が存在しない場合は空のリストを返します。
			 * @param assetId 参照元を検索するアセットの一意な識別子
			 * @return 参照元のファイルパスのリスト
			 */
			static std::vector<std::string> FindReferencingFiles(const AssetId& assetId);

		private:
			/**
			 * @brief アセットIDとその参照元のファイルパスを関連付けるマップ
			 * @details このマップは、アセットIDをキーとして、参照元のファイルパスのリストを値として保持します。
			 */
			static std::unordered_map<AssetId, std::unordered_set<std::string>> s_referencedBy;

			/**
			 * @brief ファイルパスとその参照しているアセットIDを関連付けるマップ
			 * @details このマップは、ファイルパスをキーとして、参照しているアセットIDのリストを値として保持します。
			 */
			static std::unordered_map<std::string, std::unordered_set<AssetId>> s_referencesOf;

		};
	}
}