#pragma once
#include "AnimatorControllerEditorWindow.h"

class AnimatorControllerEditor
{
public:
	AnimatorControllerEditor();
	~AnimatorControllerEditor();


	/** @brief エディタを開く。*/
	static void Open();

	/** @brief エディタを閉じる。*/
	static void Close();

	/** @brief エディタが開いているかどうかを取得。*/
	static bool IsOpen();

	/** @brief アセットを開く。*/
	static void OpenAsset(const std::filesystem::path& path);
	
	/** @brief エディタのGUIを描画。*/
	static void DrawGUI();

private:
	static inline bool s_isOpen; ///< エディタが開いているかどうかのフラグ
	static inline std::shared_ptr<AnimatorController> s_animatorController; ///< 編集中のAnimatorController
	static inline std::unique_ptr<CurryEngine::Editor::AnimatorControllerEditorWindow> s_editorWindow; ///< エディタウィンドウのインスタンス
};
