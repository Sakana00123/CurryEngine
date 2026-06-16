#pragma once
struct RenderContext;
class Scene;

namespace CurryEngine
{
	class SceneViewWindow
	{
	public:
		SceneViewWindow() = default;
		virtual ~SceneViewWindow() = default;

		// シングルトンインスタンスへのアクセス(ただし、将来的には複数インスタンスをサポートする可能性があるため、シングルトンパターンは一時的な措置として使用しています。)
		static SceneViewWindow& Get() {
			static SceneViewWindow instance;
			return instance;
		}

		/** @brief シーンビューウィンドウを描画する関数。シーンのレンダリングとオブジェクト選択を管理します。*/
		void Draw(RenderContext* rtx, Scene* scene);

		/* シーンビューウィンドウが現在フォーカスされているかどうかを返す関数。*/
		bool IsFocused() const { return isSceneWindowFocused; }

	private:
		bool isSceneWindowFocused = false; // シーンビューウィンドウが現在フォーカスされているかどうか
	};
}