#pragma once
#include "IImporter.h"

namespace CurryEngine
{
	namespace Resources
	{
		/**
		 * @brief FBXファイルをインポートするためのインポータークラス。
		 * @details このクラスは、FBX形式の3Dモデルファイルを読み込み、アセットデータに変換する機能を提供します。FBX SDKを使用してFBXファイルを解析し、必要なデータ（メッシュ、マテリアル、テクスチャなど）を抽出します。
		 */
		class FbxImporter : public IImporter
		{
			public:
			/**
			 * @brief FBXファイルを読み込み、アセットデータに変換する。
			 * @param meta 読み込むFBXアセットのメタデータ。
			 * @return 読み込んだアセットデータの共有ポインタ。読み込みに失敗した場合はnullptrを返します。
			 */
			std::shared_ptr<Resource> Import(const AssetMeta& meta) override;
			/**
			 * @brief このインポーターがサポートするファイル拡張子のリストを返す。
			 * @return サポートするファイル拡張子のリスト（例: {".fbx"}）。
			 */
			std::vector<std::string> GetSupportedExtensions() const override;
		};
	}
}
