#pragma once
#include "Engine/Resources/Resource.h"
#include "Engine/Rendering/Material.h"

namespace CurryEngine::Resources
{
	/**
	 * @brief マテリアルアセットを表すクラス。
	 * @details マテリアルアセットは、シェーダやテクスチャ、定数バッファの設定を含むリソースです。
	 *          このクラスは、マテリアルアセットのロード、保存、管理を行います。
	 */
	class AssetMaterial : public Resource
	{
	public:
		AssetMaterial() = default;
		virtual ~AssetMaterial() = default;
		/**
		 * @brief ファイルからマテリアルアセットをロードします。
		 * @param path ロードするファイルのパス。
		 * @return 成功した場合は true、失敗した場合は false を返します。
		 */
		virtual bool LoadFromFile(const std::string& path) override;
		/**
		 * @brief マテリアルアセットを保存します。
		 * @param path 保存するファイルのパス。
		 * @return 成功した場合は true、失敗した場合は false を返します。
		 */
		bool SaveToFile(const std::string& path);

		/**
		 * @brief マテリアルの内部データを取得します。
		 * @return マテリアルの内部データ。
		 */
		Material& GetMaterial() { return m_material; }

	private:
		// マテリアルの内部データ（シェーダ、テクスチャ、定数バッファなど）を保持するメンバ変数
		Material m_material;
	};
}