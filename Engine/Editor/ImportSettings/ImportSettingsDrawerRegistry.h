#pragma once
#include <Engine\Resources\AssetType.h>
#include "IImportSettingsDrawer.h"


namespace CurryEngine::Resources
{
	/**
	 * @brief インポート設定描画のレジストリクラス。
	 * @details このクラスは、特定のアセットタイプに対応するインポート設定描画クラスを登録し、取得するための機能を提供します。
	 *          具体的なアセットタイプごとにこのレジストリに描画クラスを登録することで、異なるアセットのインポート設定描画を統一的に扱うことができます。
	 */
	class ImportSettingsDrawerRegistry
	{
	public:
		/**
		 * @brief インポート設定描画クラスを初期化します。必要に応じて、ここでデフォルトの描画クラスを登録します。
		 */
		static void Initialize();
		/**
		 * @brief 指定されたアセットタイプに対応するインポート設定描画クラスを取得します。
		 * @param assetType 取得するインポート設定描画クラスのアセットタイプ
		 * @return 対応するインポート設定描画クラスのポインタ、存在しない場合はnullptr
		 */
		static IImportSettingsDrawer* Find(AssetType assetType);

	private:
		/**
		 * @brief インポート設定描画クラスを登録します。
		 * @param assetType 登録するアセットタイプ
		 * @param drawer 登録するインポート設定描画クラスのユニークポインタ
		 */
		static void Register(AssetType assetType, std::unique_ptr<IImportSettingsDrawer> drawer);
		/**
		 * @brief インポート設定描画クラスのマップを取得します。
		 * @return アセットタイプとインポート設定描画クラスの共有ポインタのマップ
		 */
		static std::unordered_map<AssetType, std::unique_ptr<IImportSettingsDrawer>>& GetMap();
	};
}