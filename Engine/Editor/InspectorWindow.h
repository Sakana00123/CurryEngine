#pragma once
#ifdef USE_IMGUI
class ObjectManager;


namespace CurryEngine
{
	/**
	 * @brief インスペクタウィンドウを管理するクラス。選択されたオブジェクトのプロパティを表示・編集するためのウィンドウを提供します。
	 */
	class InspectorWindow
	{
	public:
		InspectorWindow() = default;

		// シングルトンインスタンスへのアクセス(ただし、将来的には複数インスタンスをサポートする可能性があるため、シングルトンパターンは一時的な措置として使用しています。)
		// TODO: 将来的には、複数のインスペクタウィンドウをサポートするために、シングルトンパターンを廃止し、インスタンス管理をリファクタリングする必要があります。
		static InspectorWindow& Get() {
			static InspectorWindow instance;
			return instance;
		}

		// TODO: 早期動作確認のため、引数のリファクタリング必須。
		/** @brief インスペクタウィンドウを描画する関数。選択されたオブジェクトのプロパティを表示・編集します。*/
		void Draw(ObjectManager* objectManager);
	};
}
#endif // USE_IMGUI