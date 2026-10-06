#pragma once

namespace CurryEngine::Editor
{
	/**
	 * @brief 入力設定ウィンドウの表示・操作を提供します。
	 */
	class InputSettingsWindow
	{
	public:
		/**
		 * @brief 入力設定ウィンドウを表示します。
		 */
		static void Show();

		/**
		 * @brief 入力設定ウィンドウを非表示にします。
		 */
		static void Close();

		/**
		 * @brief 入力設定ウィンドウの表示状態を取得します。
		 * @return 表示中なら true
		 */
		static bool IsOpen() { return isOpen; }

		/**
		 * @brief 入力設定ウィンドウを描画します。
		 */
		static void Draw();


	private:
		static inline bool isOpen = false; //!< ウィンドウが開いているか

	};
}
