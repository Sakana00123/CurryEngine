#pragma once
#include <json.hpp>
#include <memory>
class Resource;

namespace CurryEngine::Resources
{
	/**
	 * @brief インポート設定の描画インターフェース。
	 * @details このインターフェースは、特定のアセットタイプに対応するインポート設定を描画するための抽象クラスです。
	 *          具体的なアセットタイプごとにこのインターフェースを実装することで、異なるアセットのインポート設定を統一的に扱うことができます。
	 */
	class IImportSettingsDrawer
	{
	public:
		virtual ~IImportSettingsDrawer() = default;
		
		/**
		 * @brief プレビュー描画の純粋仮想関数。
		 * @param previewResource 描画するプレビュー用のリソース。ユーザーがUIで設定を変更した場合は、このリソースも更新されます。
		 */
		virtual void DrawPreview(const std::shared_ptr<Resource>& previewResource) = 0;

		/**
		 * @brief インポート設定のフィールドを描画する純粋仮想関数。
		 * @param settings 描画するインポート設定のJSONデータ。ユーザーがUIで変更した内容はこのJSONに反映されます。
		 * @param isDirty インポート設定がユーザーによって変更されたかどうかを示すフラグ。ユーザーがUIで設定を変更した場合はtrueになります。
		 * @return 設定が変更された場合はtrue、変更されなかった場合はfalseを返します。
		 */
		virtual bool DrawSettingsFields(nlohmann::json& settings, bool& isDirty) = 0;

		/**
		 * @brief デフォルトのインポート設定をJSON形式で取得する純粋仮想関数。
		 * @return デフォルトのインポート設定を表すJSONデータ。具体的なアセットタイプに対応するデフォルト設定を返す必要があります。
		 */
		virtual nlohmann::json GetDefaultSettings() const = 0;
	};
}