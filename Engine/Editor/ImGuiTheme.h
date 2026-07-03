#pragma once
#include "Engine/Resources/Texture.h"

// ImGuiのテーマ設定
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
class ImGuiTheme
{
public:
	// 初期化
	static void Initialize();

	// テーマ設定
	static void SetTheme(ImGuiThemeType themeType);

	// 現在のテーマ取得
	static ImGuiThemeType GetCurrentTheme();

	// GUI描画
	static void DrawGUI();

	// テーマ設定ウィンドウを開く
	static void Show() { isOpen = true; }

	// テーマ設定ウィンドウが開いているか
	static bool IsOpen() { return isOpen; }
private:
	
	static void SetNightCurryDeluxeTheme();

	static void SetCurryTheme();

	static void SetDarkCurryTheme();

	static void SetCurryRiceTheme();

	static void SetBeefCurryTheme();

	static void SetGreenCurryTheme();

	static void SetButterChickenCurryTheme();

private:
	static inline ImGuiThemeType currentTheme;
	static inline bool isOpen = false;
};