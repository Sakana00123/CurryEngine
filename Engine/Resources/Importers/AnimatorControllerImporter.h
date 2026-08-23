#pragma once
#include "IImporter.h"

namespace CurryEngine
{
	namespace Resources
	{
		/**
		 * @brief AnimatorControllerファイルをインポートするためのインポータークラス。
		 * @details このクラスは、AnimatorController形式のファイル（例: .controller）を読み込み、アセットデータに変換する機能を提供します。AnimatorControllerデータを解析し、必要な情報（ステート、トランジション、パラメータなど）を抽出します。
		 */
		class AnimatorControllerImporter : public IImporter
		{
		public:
			/**
			 * @brief AnimatorControllerファイルを読み込み、アセットデータに変換する。
			 * @param meta 読み込むAnimatorControllerアセットのメタデータ。
			 * @return 読み込んだアセットデータの共有ポインタ。読み込みに失敗した場合はnullptrを返します。
			 */
			std::shared_ptr<Resource> Import(const AssetMeta& meta) override;
			/**
			 * @brief このインポーターがサポートするファイル拡張子のリストを返す。
			 * @return サポートするファイル拡張子のリスト（例: {".controller"}）。
			 */
			std::vector<std::string> GetSupportedExtensions() const override;
		};
	}
}
