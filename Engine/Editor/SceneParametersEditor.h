#pragma once

/** @brief SceneParametersEditor を表すクラスです。 */
class SceneParametersEditor
{
public:
	/**
	 * @brief Show の処理を行います。
	 */
	static void Show();
	/**
	 * @brief 描画処理を行います。
	 */
	static void DrawGUI();

private:
	static inline bool isOpen = true;
};