#pragma once
#include "IImporter.h"

namespace CurryEngine
{
	namespace Resources
	{
		/**
		 * @brief モデルインポーター。モデルファイルを読み込み、`Model` アセットデータに変換するクラス。
		 * @details このクラスは、モデルファイル（例: FBX, gltf, glb）を読み込み、`Model` クラスのインスタンスに変換するためのインポーターです。DirectX 11 のメッシュリソースを作成し、アセットデータとして管理します。
		 */
		class ModelImporter : public IImporter
		{
		public:
			virtual ~ModelImporter() = default;
			/**
			 * @brief モデルファイルを読み込み、`Model` アセットデータに変換する関数。
			 * @param meta 読み込むモデルのメタデータ。
			 * @return 読み込んだモデルアセットデータの共有ポインタ。読み込みに失敗した場合はnullptrを返します。
			 */
			std::shared_ptr<Resource> Import(const AssetMeta& meta) override;
			/**
			 * @brief このインポーターがサポートするファイル拡張子のリストを返す関数。
			 * @return サポートするファイル拡張子のリスト（例: {".fbx", ".obj"}）。
			 */
			std::vector<std::string> GetSupportedExtensions() const override;
		};
	}
}