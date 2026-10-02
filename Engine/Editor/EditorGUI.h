#pragma once

class Scene;

/** @brief EditorGUI を表すクラスです。 */
class EditorGUI
{
public:
	// メインメニューの描画
	/**
	 * @brief 描画処理を行います。
	 */
	static float DrawMainMenu();

	// ツールバーの描画
	/**
	 * @brief 描画処理を行います。
	 */
	static float DrawToolbar(float offsetY);

	// シーンビューのツールバーの描画
	/**
	 * @brief 描画処理を行います。
	 */
	static float DrawSceneViewToolbar();

public:
	// ファイルメニューの描画
	/**
	 * @brief 描画処理を行います。
	 */
	static void DrawFileMenu();

	/**
	 * @brief 描画処理を行います。
	 */
	//static void DrawEditMenu();

	// シーンメニューの描画
	/**
	 * @brief 描画処理を行います。
	 */
	static void DrawSceneMenu();

	// ゲームオブジェクトメニューの描画
	/**
	 * @brief 描画処理を行います。
	 */
	static void DrawGameObjectMenu();

	// ウィンドウメニューの描画
	/**
	 * @brief 描画処理を行います。
	 */
	static void DrawWindowMenu();

private:
	
	// 新規シーンの作成
	/**
	 * @brief 新しい要素を生成します。
	 */
	static void CreateNewScene();
	// シーンのオープン
	/**
	 * @brief OpenScene の処理を行います。
	 */
	static void OpenScene();
	// シーンの保存
	/**
	 * @brief SaveScene の処理を行います。
	 */
	static void SaveScene();
	// シーンの別名保存
	/**
	 * @brief SaveSceneAs の処理を行います。
	 */
	static void SaveSceneAs();
};
