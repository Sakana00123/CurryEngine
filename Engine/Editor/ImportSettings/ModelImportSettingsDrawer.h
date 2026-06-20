#pragma once
#include "IImportSettingsDrawer.h"


namespace CurryEngine::Resources
{
	/**
	 * @brief モデルのインポート設定を描画するためのクラス。モデルアセットのインポート設定をGUI上で編集するための描画ロジックを提供します。
	 * @details このクラスは、モデルアセットのインポート設定をユーザーが編集できるようにするためのGUI描画機能を提供します。例えば、スケーリング係数や静的バッチングの有効化などの設定を変更することができます。
	 */
	class ModelImportSettingsDrawer : public IImportSettingsDrawer
	{
	public:
		virtual ~ModelImportSettingsDrawer() = default;
		/**
		 * @brief プレビュー用のリソースを描画する関数。
		 * @param previewResource プレビューとして表示するリソースへの共有ポインタ。
		 * @details この関数は、モデルアセットのプレビューをGUI上に描画します。プレビューが利用できない場合は、適切なメッセージを表示します。
		 */
		void DrawPreview(const std::shared_ptr<Resource>& previewResource) override;
		/**
		 * @brief インポート設定フィールドを描画し、ユーザーが編集できるようにする関数。
		 * @param editingSettings 現在編集中のインポート設定を保持するJSONオブジェクトへの参照。
		 * @param isDirty 設定が変更されたかどうかを示すフラグへの参照。変更があった場合はtrueに設定されます。
		 * @return 設定が変更された場合はtrue、それ以外の場合はfalseを返します。
		 */
		bool DrawSettingsFields(nlohmann::json& editingSettings, bool& isDirty) override;
		/**
		 * @brief デフォルトのインポート設定を取得する関数。
		 * @return デフォルトのインポート設定を表すJSONオブジェクト。
		 */
		nlohmann::json GetDefaultSettings() const override;
	};
}