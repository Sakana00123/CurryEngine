#pragma once
#include "AssetType.h"

namespace CurryEngine
{
	namespace Resources
	{
		/**
		 * @brief アセットタイプに関連するユーティリティ関数を提供するクラス
		 */
		class AssetTypeUtils
		{
		public:
			/**
			 * @brief ファイル拡張子からアセットタイプを取得します。
			 * @param extension ファイル拡張子（例: ".png", ".fbx"）
			 * @return 対応するアセットタイプ、対応するものがない場合はAssetType::Unknown
			 */
			static AssetType DetectFromExtension(const std::string& extension);
		};
	}
}