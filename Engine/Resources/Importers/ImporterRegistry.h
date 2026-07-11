#pragma once
#include "IImporter.h"

namespace CurryEngine
{
	namespace Resources
	{
		/**
		 * @brief アセットインポーターのレジストリクラス
		 * 
		 * このクラスは、アセットインポーターを登録し、ファイル拡張子に基づいて適切なインポーターを取得するための機能を提供します。
		 */
		class ImporterRegistry
		{
		public:
			/**
			 * @brief インポーターを初期化します。必要に応じて、ここでデフォルトのインポーターを登録します。
			 */
			static void Initialize();

			/**
			 * @brief 指定されたファイル拡張子に対応するインポーターを検索します。
			 * @param type アセットタイプ
			 * @param extension 検索するファイル拡張子（例: ".png", ".fbx"）
			 * @return 対応するインポーターのポインタ、存在しない場合はnullptr
			 */
			static IImporter* Find(AssetType type, const std::filesystem::path& extension);

			/**
			 * @brief 指定されたアセットメタデータに対応するインポーターを検索します。
			 * @param meta 検索するアセットのメタデータ
			 * @return 対応するインポーターのポインタ、存在しない場合はnullptr
			 */
			static IImporter* Find(const AssetMeta& meta);

		private:
			/**
			 * @brief インポーターを登録します。
			 * @param type 登録するインポーターのアセットタイプ
			 * @param importer 登録するインポーターの共有ポインタ
			 */
			static void Register(AssetType type, std::unique_ptr<IImporter> importer);

			/**
			 * @brief インポーターのマップを取得します。
			 * @return アセットタイプとインポーターの共有ポインタのマップ
			 */
			static std::unordered_map<AssetType, std::unique_ptr<IImporter>>& GetMap();
		};
	}
}
