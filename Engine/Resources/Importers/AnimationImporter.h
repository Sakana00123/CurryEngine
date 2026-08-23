#pragma once
#include "IImporter.h"

namespace CurryEngine
{
	namespace Resources
	{
		/**
		 * @brief アニメーションファイルをインポートするためのインポータークラス。
		 * @details このクラスは、アニメーション形式のファイル（例: .anim）を読み込み、アセットデータに変換する機能を提供します。アニメーションデータを解析し、必要な情報（キーフレーム、トランスフォームなど）を抽出します。
		 */
		class AnimationImporter : public IImporter
		{
			public:
			/**
			 * @brief アニメーションファイルを読み込み、アセットデータに変換する。
			 * @param meta 読み込むアニメーションアセットのメタデータ。
			 * @return 読み込んだアセットデータの共有ポインタ。読み込みに失敗した場合はnullptrを返します。
			 */
			std::shared_ptr<Resource> Import(const AssetMeta& meta) override;
			/**
			 * @brief このインポーターがサポートするファイル拡張子のリストを返す。
			 * @return サポートするファイル拡張子のリスト（例: {".anim"}）。
			 */
			std::vector<std::string> GetSupportedExtensions() const override;
		};
	}
}

