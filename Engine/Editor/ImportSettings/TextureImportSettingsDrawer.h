#pragma once
#include "IImportSettingsDrawer.h"
#include <dxgiformat.h>

namespace CurryEngine::Resources
{
	/**
	 * @brief テクスチャのインポート設定描画クラス。
	 * @details このクラスは、テクスチャアセットのインポート設定を描画するための具体的な実装です。
	 *          ユーザーがUIで設定を変更した場合は、対応するプレビューリソースも更新されます。
	 */
	class TextureImportSettingsDrawer : public IImportSettingsDrawer
	{
	public:
		/**
		 * @brief プレビュー描画関数。テクスチャのプレビューを描画します。
		 * @param previewResource 描画するプレビュー用のリソース。ユーザーがUIで設定を変更した場合は、このリソースも更新されます。
		 * @param context 描画に使用するレンダリングコンテキスト。必要に応じて、描画処理でこのコンテキストを使用してリソースの描画を行います。
		 */
		virtual void DrawPreview(const std::shared_ptr<Resource>& previewResource, RenderContext* context) override;
		/**
		 * @brief インポート設定のフィールドを描画する関数。テクスチャのインポート設定をUIに表示します。
		 * @param settings 描画するインポート設定のJSONデータ。ユーザーがUIで変更した内容はこのJSONに反映されます。
		 * @param isDirty インポート設定がユーザーによって変更されたかどうかを示すフラグ。ユーザーがUIで設定を変更した場合はtrueになります。
		 * @return 設定が変更された場合はtrue、変更されなかった場合はfalseを返します。
		 */
		virtual bool DrawSettingsFields(nlohmann::json& settings, bool& isDirty) override;

		/**
		  * @brief デフォルトのインポート設定をJSON形式で取得する関数。テクスチャのデフォルトインポート設定を返します。
		  * @return デフォルトのインポート設定を表すJSONデータ。テクスチャアセットに対応するデフォルト設定を返す必要があります。
		  */
		virtual nlohmann::json GetDefaultSettings() const override;
	private:
		/**
		 * @brief DXGI_FORMATを文字列に変換するヘルパー関数。
		 * @param format 変換するDXGI_FORMAT
		 * @return 変換後の文字列。対応するフォーマットがない場合は"Unknown"を返します。
		 */
		const char* DxgiFormatToString(DXGI_FORMAT format);
	};
}