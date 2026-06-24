#pragma once
#include <memory>
struct RenderContext;
namespace CurryEngine::Resources
{
	struct AssetId;
	class AssetMaterial;
}

namespace CurryEngine::Editor
{
	/**
	 * @brief マテリアルエディタクラス
	 * - マテリアルのプロパティを編集するためのGUIを提供
	 * - マテリアルのシェーダ、テクスチャ、定数バッファの値を編集可能
	 * - ImGuiなどのGUIライブラリを使用して描画
	 */
	class MaterialEditor
	{
	public:
		MaterialEditor(const CurryEngine::Resources::AssetId& materialId);
		~MaterialEditor() = default;

#ifdef USE_IMGUI
		/**
		 * @brief マテリアルエディタのGUIを描画します。
		 * @param context 描画コンテキスト。
		 * - ImGuiを使用してマテリアルのプロパティを表示・編集します。
		 */
		void DrawGUI(RenderContext* context);
#endif // USE_IMGUI

		/**
		 * @brief マテリアルの変更を保存します。
		 * - 編集したマテリアルのプロパティをアセットとして保存します。
		 */
		void Save();

	private:
		std::shared_ptr<CurryEngine::Resources::AssetMaterial> m_material;
		bool m_isOpen{ false }; ///< エディタが開いているかどうかのフラグ
	};
}