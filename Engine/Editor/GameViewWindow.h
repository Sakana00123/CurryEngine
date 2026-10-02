#pragma once
struct RenderContext;
class Scene;

namespace CurryEngine
{
	/** @brief GameViewWindow を表すクラスです。 */
	class GameViewWindow
	{
	public:
		/**
		 * @brief GameViewWindow を構築します。
		 */
		GameViewWindow() = default;
		/**
		 * @brief GameViewWindow を破棄します。
		 */
		virtual ~GameViewWindow() = default;
		// シングルトンインスタンスへのアクセス(ただし、将来的には複数インスタンスをサポートする可能性があるため、シングルトンパターンは一時的な措置として使用しています。)
		static GameViewWindow& Get() {
			static GameViewWindow instance;
			return instance;
		}
		/** @brief ゲームビューウィンドウを描画する関数。ゲームのレンダリングを管理します。*/
		void Draw(RenderContext* rtx, Scene* scene);

		/* ゲームビューウィンドウが現在フォーカスされているかどうかを返す関数。*/
		bool IsFocused() const { return isGameWindowFocused; }

	private:
		bool isGameWindowFocused = false; // ゲームビューウィンドウが現在フォーカスされているかどうか
	};
}