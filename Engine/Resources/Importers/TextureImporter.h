#pragma once
#include "IImporter.h"

namespace CurryEngine
{
	namespace Resources
	{
		/**
		 * @brief テクスチャインポーター。テクスチャファイルを読み込み、`Texture` アセットデータに変換するクラス。
		 * @details このクラスは、テクスチャファイル（例: PNG, JPEG）を読み込み、`Texture` クラスのインスタンスに変換するためのインポーターです。DirectX 11 のテクスチャリソースを作成し、アセットデータとして管理します。
		 */
		class TextureImporter : public IImporter
		{
		public:
			virtual ~TextureImporter() = default;
			/**
			 * @brief テクスチャファイルを読み込み、`Texture` アセットデータに変換する関数。
			 * @param meta 読み込むテクスチャのメタデータ。
			 * @return 読み込んだテクスチャアセットデータの共有ポインタ。読み込みに失敗した場合はnullptrを返します。
			 */
			std::shared_ptr<Resource> Import(const AssetMeta& meta) override;
			/**
			 * @brief このインポーターがサポートするファイル拡張子のリストを返す関数。
			 * @return サポートするファイル拡張子のリスト（例: {".png", ".jpg"}）。
			 */
			std::vector<std::string> GetSupportedExtensions() const override;

		private:
			
			bool LoadTextureFromFile(const AssetMeta& meta, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& textureView, Microsoft::WRL::ComPtr<ID3D11Resource>& textureResource);

		};
	}
}