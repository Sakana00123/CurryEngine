#pragma once
#include <json.hpp>
#include <memory>
class Resource;
struct RenderContext;

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


		virtual void Reset() {} // インポート設定描画クラスの状態をリセットするための仮想関数。必要に応じてオーバーライドして使用します。

		/**
		 * @brief 3Dプレビュー描画の仮想関数。必要に応じてオーバーライドして使用します。
		 * @param previewResource 描画するプレビュー用のリソース。ユーザーがUIで設定を変更した場合は、このリソースも更新されます。
		 * @details デフォルトの実装は空のため、3Dプレビューが必要ないアセットタイプの場合はこの関数をオーバーライドする必要はありません。
		 */
		virtual void Draw3DPreview(const std::shared_ptr<Resource>& previewResource, RenderContext* context) {}

		/**
		 * @brief プレビュー描画の純粋仮想関数。
		 * @param previewResource 描画するプレビュー用のリソース。ユーザーがUIで設定を変更した場合は、このリソースも更新されます。
		 * @param context 描画に使用するレンダリングコンテキスト。必要に応じて、描画処理でこのコンテキストを使用してリソースの描画を行います。
			 * @details この関数は、アセットのプレビューをGUI上に描画するための純粋仮想関数です。具体的なアセットタイプに対応する描画ロジックを実装する必要があります。プレビューが利用できない場合は、適切なメッセージを表示するなどの対応が必要です。
		 */
		virtual void DrawPreview(const std::shared_ptr<Resource>& previewResource, RenderContext* context) = 0;

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