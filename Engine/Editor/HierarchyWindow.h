#pragma once
#ifdef USE_IMGUI
#include <optional>
class GameObject;
class ObjectManager;


namespace CurryEngine
{
	/**
	 * @brief ヒエラルキーウィンドウを管理するクラス。シーン内のオブジェクトの階層構造を表示・操作するためのウィンドウを提供します。
	 */
	class HierarchyWindow
	{
	public:
		HierarchyWindow() = default;
		// シングルトンインスタンスへのアクセス(ただし、将来的には複数インスタンスをサポートする可能性があるため、シングルトンパターンは一時的な措置として使用しています。)
		static HierarchyWindow& Get() {
			static HierarchyWindow instance;
			return instance;
		}

		/** @brief ヒエラルキーウィンドウを描画する関数。シーン内のオブジェクトの階層構造を表示・操作します。*/
		void Draw(ObjectManager* objectManager);
	private:
		struct PendingDrop {
			GameObject* target;       // ドロップ先
			bool reorder;             // true=並び替え / false=親子関係
			bool appendToEnd = false; // 末尾に追加するかどうか
		};
		// ドロップ処理の保留状態を管理するオプション。ドラッグ終了後にドロップ処理を実行するために使用されます。
		std::optional<PendingDrop> m_pendingDrop;
	};
}

#endif // USE_IMGUI
