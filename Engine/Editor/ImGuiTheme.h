#pragma once
#include "Engine/Resources/Texture.h"

// ImGuiのテーマ設定
/** @brief ImGuiThemeType を表す列挙型です。 */
enum ImGuiThemeType
{
	CurryTheme,
	NightCurryDeluxeTheme,
	DarkCurryTheme,
	CurryRiceTheme,
	BeefCurryTheme,
	GreenCurryTheme,
	ButterChickenCurryTheme,

	NumImGuiThemes
};

// ImGuiテーマ管理クラス
/** @brief ImGuiTheme を表すクラスです。 */
class ImGuiTheme
{
public:
	// 初期化
	/**
	 * @brief 初期化します。
	 */
	static void Initialize();

	// テーマ設定
	/**
	 * @brief Theme を設定します。
	 */
	static void SetTheme(ImGuiThemeType themeType);

	// 現在のテーマ取得
	/**
	 * @brief GetCurrentTheme に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	static ImGuiThemeType GetCurrentTheme();

	// GUI描画
	/**
	 * @brief 描画処理を行います。
	 */
	static void DrawGUI();

	// テーマ設定ウィンドウを開く
	static void Show() { isOpen = true; }

	// テーマ設定ウィンドウが開いているか
	static bool IsOpen() { return isOpen; }
private:
	
	/**
	 * @brief NightCurryDeluxeTheme を設定します。
	 */
	static void SetNightCurryDeluxeTheme();

	/**
	 * @brief CurryTheme を設定します。
	 */
	static void SetCurryTheme();

	/**
	 * @brief DarkCurryTheme を設定します。
	 */
	static void SetDarkCurryTheme();

	/**
	 * @brief CurryRiceTheme を設定します。
	 */
	static void SetCurryRiceTheme();

	/**
	 * @brief BeefCurryTheme を設定します。
	 */
	static void SetBeefCurryTheme();

	/**
	 * @brief GreenCurryTheme を設定します。
	 */
	static void SetGreenCurryTheme();

	/**
	 * @brief ButterChickenCurryTheme を設定します。
	 */
	static void SetButterChickenCurryTheme();

private:
	static inline ImGuiThemeType currentTheme;
	static inline bool isOpen = false;
};