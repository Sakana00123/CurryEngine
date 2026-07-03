#pragma once
#include <Engine\Resources\AssetId.h>
class Resource;
struct RenderContext;
#include <thread>
#include <mutex>

namespace CurryEngine::Resources
{
	/**
	 * @brief インポート設定ウィンドウクラス。アセットのインポート設定を編集するためのウィンドウを管理します。
	 * @details このクラスは、アセットのインポート設定をユーザーが編集できるようにするためのGUIウィンドウを提供します。例えば、テクスチャのミップマップ生成や圧縮形式などの設定を変更することができます。
	 */
	class ImportSettingsWindow
	{
	public:

		/**
		 * @brief インポート設定ウィンドウを開きます。
		 * @param id 編集するアセットの一意な識別子。ウィンドウはこのIDに対応するアセットのインポート設定を表示します。
		 */
		static void Open(const AssetId& id);

		/**
		 * @brief 新しいアセットのインポート設定ウィンドウを開きます。
		 * @param id 編集する新しいアセットの一意な識別子。ウィンドウはこのIDに対応するアセットのインポート設定を表示します。
		 * @details この関数は、まだインポートされていない新しいアセットのインポート設定を編集するために使用されます。例えば、外部から新しいファイルがドラッグ＆ドロップされたときなどに呼び出されることがあります。
		 */
		static void OpenForNewAsset(const AssetId& id);

		/**
		 * @brief 3Dプレビューを描画します。通常はエディタのメインループ内で呼び出されます。
		 * @details この関数は、ウィンドウが開いている場合に3Dプレビューを描画します。ユーザーがインポート設定を変更した場合は、対応するプレビューも更新されます。
		 */
		static void Render3DPreview(RenderContext* context);

		/**
		 * @brief インポート設定ウィンドウのGUIを描画します。通常はエディタのメインループ内で呼び出されます。
		 * @details この関数は、ウィンドウが開いている場合にインポート設定の編集UIを描画します。ユーザーが設定を変更した場合は、対応するアセットのメタデータも更新されます。
		 */
		static void DrawGUI(RenderContext* context);

		/**
		 * @brief インポート設定ウィンドウが現在開いているかどうかを判定します。
		 * @return ウィンドウが開いている場合はtrue、閉じている場合はfalseを返します。
		 */
		static bool IsOpen();

		/**
		 * @brief プレビューリソースの更新進捗を取得します。
		 * @return プレビューリソースの更新進捗（0.0から1.0の範囲）を返します。非同期でプレビューを生成する場合に使用されます。
		 */
		static void UpdatePreviewProgress(float progress) { _previewProgress = progress; }

		/**
		 * @brief プレビューリソースの更新がキャンセルされたかどうかを判定します。
		 * @return プレビューリソースの更新がキャンセルされた場合はtrue、そうでない場合はfalseを返します。非同期でプレビューを生成する場合に使用されます。
		 */
		static bool IsPreviewLoadCancelled() { return _isPreviewLoadCancelled; }
	private:
		static void OpenInternal(const AssetId& id, bool isNewAsset);
		static void DrawPreview(const AssetId& id, RenderContext* context);
		static void DrawSettingsFields(const AssetId& id);
		static void OnApply(const AssetId& id);

		static void RequestPreviewUpdate(const AssetId& id);
		static void UpdatePreview(const AssetId& id);

		static void ShowCloseConfirmDialog();
		static void CloseConfirmDialog();
		static void CloseWindow();


		static inline bool _isOpen = false; ///< インポート設定ウィンドウが開いているかどうか
		static inline AssetId _targetId; ///< 現在編集しているアセットのID

		static inline nlohmann::json _editingSettings; ///< 現在編集しているインポート設定のJSONデータ。ユーザーがUIで変更した内容を一時的に保持します。
		static inline bool _isDirty = false; ///< インポート設定がユーザーによって変更されたかどうかを示すフラグ。ユーザーがUIで設定を変更した場合はtrueになります。
		static inline std::shared_ptr<Resource> _previewResource; ///< プレビュー用のリソース。
		static inline bool _showCloseConfirm = false; ///< ウィンドウを閉じるときに変更が保存されていない場合に確認ダイアログを表示するかどうかのフラグ
		
		// 非同期でプレビューを生成する場合のためのロック
		static inline std::mutex _previewMutex; ///< プレビューリソースの更新を保護するためのミューテックス。非同期でプレビューを生成する場合に使用されます。
		// 非同期でプレビューを生成する場合のためのスレッド
		static inline std::thread _previewThread; ///< プレビューリソースの生成を非同期で行うためのスレッド。非同期でプレビューを生成する場合に使用されます。
		static inline bool _isPreviewUpdating = false; ///< プレビューリソースの更新中かどうかを示すフラグ。非同期でプレビューを生成する場合に使用されます。

		static inline float _previewProgress = 0.0f; ///< プレビューリソースの更新進捗を示すフラグ。非同期でプレビューを生成する場合に使用されます。
		static inline bool _isPreviewLoadCancelled = false; ///< プレビューリソースの更新がキャンセルされたかどうかを示すフラグ。非同期でプレビューを生成する場合に使用されます。
	};
}