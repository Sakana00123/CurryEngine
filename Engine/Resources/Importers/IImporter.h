#pragma once
#include "Engine/Resources/AssetMeta.h"
#include "Engine/Resources/Resource.h"

namespace CurryEngine
{
	namespace Resources
	{
		/**
		 * @brief インポーターのインターフェース。アセットファイルを読み込み、アセットデータに変換するための抽象クラス。
		 * @details 具体的なアセットフォーマット（例: テクスチャ、モデル、シーン）ごとにこのインターフェースを実装することで、異なるフォーマットのアセットを統一的に扱うことができます。
		 */
		class IImporter
		{
		public:
			virtual ~IImporter() = default;
			/**
			 * @brief アセットファイルを読み込み、アセットデータに変換する純粋仮想関数。
			 * @param meta 読み込むアセットのメタデータ。
			 * @return 読み込んだアセットデータの共有ポインタ。読み込みに失敗した場合はnullptrを返します。
			 */
			virtual std::shared_ptr<Resource> Import(const AssetMeta& meta) = 0;
			/**
			 * @brief このインポーターがサポートするファイル拡張子のリストを返す純粋仮想関数。
			 * @return サポートするファイル拡張子のリスト（例: {".png", ".jpg"}）。
			 */
			virtual std::vector<std::string> GetSupportedExtensions() const = 0;
		};
	}
}