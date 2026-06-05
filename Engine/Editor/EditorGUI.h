#pragma once

class Scene;

class EditorGUI
{
public:
	// メインメニューの描画
	static float DrawMainMenu();

	// ツールバーの描画
	static float DrawToolbar(float offsetY);

public:
	// ファイルメニューの描画
	static void DrawFileMenu();

	//static void DrawEditMenu();

	// シーンメニューの描画
	static void DrawSceneMenu();

	// ゲームオブジェクトメニューの描画
	static void DrawGameObjectMenu();

	// ウィンドウメニューの描画
	static void DrawWindowMenu();

private:
	
	// 新規シーンの作成
	static void CreateNewScene();
	// シーンのオープン
	static void OpenScene();
	// シーンの保存
	static void SaveScene();
	// シーンの別名保存
	static void SaveSceneAs();
};
