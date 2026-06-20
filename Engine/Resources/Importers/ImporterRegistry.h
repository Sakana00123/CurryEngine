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
			 * @brief 指定されたアセットタイプに対応するインポーターを取得します。
			 * @param type 取得するインポーターのアセットタイプ
			 * @return 対応するインポーターのポインタ、存在しない場合はnullptr
			 */
			static IImporter* Find(AssetType type);

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