#pragma once
#include "IImportSettingsDrawer.h"

namespace CurryEngine::Resources
{
	/**
	 * @brief FBXのインポート設定描画クラス。
	 * @details このクラスは、FBXアセットのインポート設定を描画するための具体的な実装です。
	 *          ユーザーがUIで設定を変更した場合は、対応するプレビューリソースも更新されます。
	 */
	class FbxImportSettingsDrawer : public IImportSettingsDrawer
	{
	public:

		/**
		 * @brief 3Dプレビュー描画関数。FBXの3Dプレビューを描画します。
		 * @param previewResource 描画するプレビュー用のリソース。ユーザーがUIで設定を変更した場合は、このリソースも更新されます。
		 * @param context 描画に使用するレンダリングコンテキスト。必要に応じて、描画処理でこのコンテキストを使用してリソースの描画を行います。
		 */
		void Draw3DPreview(const std::shared_ptr<Resource>& previewResource, RenderContext* context) override;

		/**
		 * @brief プレビュー描画関数。FBXのプレビューを描画します。
		 * @param previewResource 描画するプレビュー用のリソース。ユーザーがUIで設定を変更した場合は、このリソースも更新されます。
		 * @param context 描画に使用するレンダリングコンテキスト。必要に応じて、描画処理でこのコンテキストを使用してリソースの描画を行います。
		 */
		void DrawPreview(const std::shared_ptr<Resource>& previewResource, RenderContext* context) override;

		/**
		 * @brief インポート設定のフィールドを描画する関数。FBXのインポート設定をUIに表示します。
		 * @param settings 描画するインポート設定のJSONデータ。ユーザーがUIで変更した内容はこのJSONに反映されます。
		 * @param isDirty インポート設定がユーザーによって変更されたかどうかを示すフラグ。ユーザーがUIで設定を変更した場合はtrueになります。
		 * @return 設定が変更された場合はtrue、変更されなかった場合はfalseを返します。
		 */
		bool DrawSettingsFields(nlohmann::json& settings, bool& isDirty) override;

		/**
		  * @brief デフォルトのインポート設定をJSON形式で取得する関数。FBXのデフォルトインポート設定を返します。
		  * @return デフォルトのインポート設定を表すJSONデータ。FBXアセットに対応するデフォルト設定を返す必要があります。
		  */
		nlohmann::json GetDefaultSettings() const override;
	};
}
