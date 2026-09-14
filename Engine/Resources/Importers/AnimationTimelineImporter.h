#pragma once
#include "IImporter.h"

namespace CurryEngine
{
	namespace Resources
	{
		/**
		 * @brief AnimationTimelineファイルをインポートするためのインポータークラス。
		 * @details このクラスは、AnimationTimeline形式のファイル（例: .timeline）を読み込み、アセットデータに変換する機能を提供します。AnimationTimelineデータを解析し、必要な情報（イベントトラック、キーフレームなど）を抽出します。
		 */
		class AnimationTimelineImporter : public IImporter
		{
		public:
			/**
			 * @brief AnimationTimelineファイルを読み込み、アセットデータに変換する。
			 * @param meta 読み込むAnimationTimelineアセットのメタデータ。
			 * @return 読み込んだアセットデータの共有ポインタ。読み込みに失敗した場合はnullptrを返します。
			 */
			std::shared_ptr<Resource> Import(const AssetMeta& meta) override;
			/**
			 * @brief このインポーターがサポートするファイル拡張子のリストを返す。
			 * @return サポートするファイル拡張子のリスト（例: {".timeline"}）。
			 */
			std::vector<std::string> GetSupportedExtensions() const override;
		};
	}
}
