#include "pch.h"
#include "ImGuiTheme.h"
#include "Engine/Resources/ResourceManager.h"

void ImGuiTheme::Initialize()
{
	// 初期テーマを設定
	SetTheme(NightCurryDeluxeTheme);
}

void ImGuiTheme::SetTheme(ImGuiThemeType themeType)
{
	switch (themeType)
	{
	case CurryTheme:
		SetCurryTheme();
		break;
	case NightCurryDeluxeTheme:
		SetNightCurryDeluxeTheme();
		break;
	case DarkCurryTheme:
		SetDarkCurryTheme();
		break;
	case CurryRiceTheme:
        SetCurryRiceTheme();
		break;
	case BeefCurryTheme:
        SetBeefCurryTheme();
		break;
    case GreenCurryTheme:
		SetGreenCurryTheme();
        break;
	case ButterChickenCurryTheme:
        SetButterChickenCurryTheme();
		break;
	default:
		break;
	}
	currentTheme = themeType;
}

ImGuiThemeType ImGuiTheme::GetCurrentTheme()
{
    return currentTheme;
}

void ImGuiTheme::DrawGUI()
{
#ifdef USE_IMGUI
	// テーマ変更画面
    if (isOpen)
    {
		// ドッキング無効化
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking;

        ImGui::Begin("ImGui Theme Settings", &isOpen, windowFlags);
        const char* themeNames[NumImGuiThemes] =
        {
            "Curry Theme",
            "Night Curry Deluxe Theme",
            "Dark Curry Theme",
            "Curry Rice Theme",
            "Beef Curry Theme",
            "Green Curry Theme",
            "Butter Chicken Curry Theme"
		};
        static int currentThemeIndex = static_cast<int>(currentTheme);
        if (ImGui::Combo("Select Theme", &currentThemeIndex, themeNames, NumImGuiThemes))
        {
            SetTheme(static_cast<ImGuiThemeType>(currentThemeIndex));
        }
        ImGui::End();
	}
#endif // USE_IMGUI
}

void ImGuiTheme::SetNightCurryDeluxeTheme()
{
#ifdef USE_IMGUI
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    //------------------------------------
    // Base Palette
    //------------------------------------
    ImVec4 curry = ImVec4(0.12f, 0.08f, 0.05f, 1.00f);  // 背景（カレーのルー）
    ImVec4 rice = ImVec4(0.95f, 0.93f, 0.88f, 1.00f);  // テキスト（白米）
    ImVec4 pickle = ImVec4(0.80f, 0.25f, 0.15f, 1.00f);  // 福神漬け（アクセント）
    ImVec4 gravy = ImVec4(0.25f, 0.17f, 0.10f, 1.00f);  // ルーの影
    ImVec4 highlight = ImVec4(0.95f, 0.45f, 0.25f, 1.00f);  // 温かみのある強調色

    //------------------------------------
    // Window / Base
    //------------------------------------
    colors[ImGuiCol_Text] = rice;
    colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.50f, 0.45f, 1.00f);
    colors[ImGuiCol_WindowBg] = curry;
    colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.07f, 0.04f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.10f, 0.07f, 0.98f);
    colors[ImGuiCol_Border] = gravy;
    colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.3f);

    //------------------------------------
    // Frame / Inputs
    //------------------------------------
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.13f, 0.08f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30f, 0.18f, 0.10f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.40f, 0.25f, 0.12f, 1.00f);

    //------------------------------------
    // Buttons
    //------------------------------------
    colors[ImGuiCol_Button] = ImVec4(0.35f, 0.22f, 0.10f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = pickle;
    colors[ImGuiCol_ButtonActive] = ImVec4(0.95f, 0.40f, 0.25f, 1.00f);

    //------------------------------------
    // Check / Sliders
    //------------------------------------
    colors[ImGuiCol_CheckMark] = highlight;
    colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.40f, 0.20f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = highlight;

    //------------------------------------
    // Scrollbars
    //------------------------------------
    colors[ImGuiCol_ScrollbarBg] = curry;
    colors[ImGuiCol_ScrollbarGrab] = gravy;
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.45f, 0.28f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.55f, 0.33f, 0.16f, 1.00f);

    //------------------------------------
    // Tabs
    //------------------------------------
    colors[ImGuiCol_Tab] = ImVec4(0.50f, 0.17f, 0.10f, 1.00f);
    colors[ImGuiCol_TabHovered] = pickle;
    colors[ImGuiCol_TabActive] = ImVec4(0.85f, 0.28f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.25f, 0.10f, 0.07f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.45f, 0.18f, 0.12f, 1.00f);

    //------------------------------------
    // Headers / Tree
    //------------------------------------
    colors[ImGuiCol_Header] = ImVec4(0.45f, 0.18f, 0.12f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.65f, 0.23f, 0.14f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.85f, 0.30f, 0.18f, 1.00f);

    //------------------------------------
    // Separators
    //------------------------------------
    colors[ImGuiCol_Separator] = ImVec4(0.35f, 0.25f, 0.15f, 0.6f);
    colors[ImGuiCol_SeparatorHovered] = highlight;
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.95f, 0.45f, 0.20f, 1.00f);

    //------------------------------------
    // Resize grips
    //------------------------------------
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.80f, 0.40f, 0.20f, 0.30f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.95f, 0.50f, 0.30f, 0.60f);
    colors[ImGuiCol_ResizeGripActive] = highlight;

    //------------------------------------
    // Title / Menu
    //------------------------------------
    colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.12f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.45f, 0.25f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.10f, 0.05f, 0.9f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.10f, 0.06f, 1.00f);

    //------------------------------------
    // Plots
    //------------------------------------
    colors[ImGuiCol_PlotLines] = ImVec4(0.90f, 0.80f, 0.60f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = pickle;
    colors[ImGuiCol_PlotHistogram] = highlight;
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.98f, 0.55f, 0.30f, 1.00f);

    //------------------------------------
    // Tables
    //------------------------------------
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.20f, 0.12f, 0.06f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.35f, 0.25f, 0.15f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.25f, 0.18f, 0.12f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.12f, 0.08f, 0.05f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.16f, 0.10f, 0.06f, 1.00f);

    //------------------------------------
    // Highlights / Selection
    //------------------------------------
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.85f, 0.40f, 0.20f, 0.45f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.45f, 0.20f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.45f, 0.25f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.50f, 0.30f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.05f, 0.03f, 0.02f, 0.6f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.05f, 0.03f, 0.02f, 0.7f);

    //------------------------------------
    // Rounded / Padding adjustments
    //------------------------------------
    style.FrameRounding = 5.0f;
    style.GrabRounding = 5.0f;
    style.WindowRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.TabRounding = 5.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(6, 4);
#endif // USE_IMGUI
}

void ImGuiTheme::SetCurryTheme()
{
#ifdef USE_IMGUI
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // カレーのルー（全体背景）
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.07f, 0.03f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.08f, 0.04f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.10f, 0.06f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.18f, 0.10f, 1.00f);

    // ごはん（文字や強調）
    colors[ImGuiCol_Text] = ImVec4(0.96f, 0.92f, 0.85f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.50f, 0.45f, 1.00f);

    // ルーの濃淡（ウィジェット）
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.13f, 0.07f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.28f, 0.17f, 0.09f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.36f, 0.22f, 0.11f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.07f, 0.03f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.35f, 0.22f, 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.45f, 0.28f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.55f, 0.33f, 0.16f, 1.00f);

    // 福神漬け（アクセント系）
    colors[ImGuiCol_Header] = ImVec4(0.60f, 0.18f, 0.10f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.75f, 0.25f, 0.15f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.85f, 0.30f, 0.18f, 1.00f);

    // タブ（福神漬けメイン）
    colors[ImGuiCol_Tab] = ImVec4(0.55f, 0.15f, 0.10f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.80f, 0.25f, 0.15f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.90f, 0.30f, 0.20f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.30f, 0.10f, 0.07f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.45f, 0.18f, 0.12f, 1.00f);

    // ボタン（カレー＋福神漬け風）
    colors[ImGuiCol_Button] = ImVec4(0.35f, 0.20f, 0.10f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.60f, 0.25f, 0.15f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.30f, 0.18f, 1.00f);

    // 選択・ハイライト
    colors[ImGuiCol_CheckMark] = ImVec4(0.95f, 0.45f, 0.25f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.40f, 0.20f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.95f, 0.50f, 0.30f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.80f, 0.40f, 0.20f, 0.30f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.95f, 0.50f, 0.30f, 0.60f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.95f, 0.50f, 0.30f, 1.00f);

    // メニューバー・タイトルバー
    colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.12f, 0.05f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.45f, 0.25f, 0.12f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.10f, 0.05f, 1.00f);

    // トーンバランス調整（角丸・パディング）
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.ScrollbarRounding = 5.0f;
    style.TabRounding = 5.0f;
#endif // USE_IMGUI
}

void ImGuiTheme::SetDarkCurryTheme()
{
#ifdef USE_IMGUI
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // ベースカラー
    ImVec4 curryDark = ImVec4(0.15f, 0.10f, 0.05f, 1.0f);  // カレーのルー（背景）
    ImVec4 curryMedium = ImVec4(0.30f, 0.20f, 0.10f, 1.0f);  // フレーム・ボタン
    ImVec4 curryLight = ImVec4(0.55f, 0.38f, 0.18f, 1.0f);  // ホバー
    ImVec4 spice = ImVec4(0.85f, 0.45f, 0.20f, 1.0f);  // スパイスオレンジ（アクティブ）
    ImVec4 rice = ImVec4(0.96f, 0.92f, 0.82f, 1.0f);  // ご飯（テキスト）
    ImVec4 plate = ImVec4(0.20f, 0.15f, 0.10f, 1.0f);  // お皿
    ImVec4 border = ImVec4(0.45f, 0.35f, 0.20f, 1.0f);  // 境界線

    // 背景
    colors[ImGuiCol_WindowBg] = curryDark;
    colors[ImGuiCol_ChildBg] = curryDark;
    colors[ImGuiCol_PopupBg] = plate;
    colors[ImGuiCol_Border] = border;
    colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

    // タブ（カレー部分）
    colors[ImGuiCol_Tab] = ImVec4(0.30f, 0.22f, 0.10f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.45f, 0.32f, 0.15f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.60f, 0.42f, 0.20f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.20f, 0.15f, 0.08f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.35f, 0.25f, 0.10f, 1.00f);

    // テキスト
    colors[ImGuiCol_Text] = rice;
    colors[ImGuiCol_TextDisabled] = ImVec4(0.65f, 0.60f, 0.55f, 1.0f);

    // ボタン
    colors[ImGuiCol_Button] = curryMedium;
    colors[ImGuiCol_ButtonHovered] = curryLight;
    colors[ImGuiCol_ButtonActive] = spice;

    // フレーム（入力欄・スライダー）
    colors[ImGuiCol_FrameBg] = curryMedium;
    colors[ImGuiCol_FrameBgHovered] = curryLight;
    colors[ImGuiCol_FrameBgActive] = spice;

    // タイトル・ヘッダー
    colors[ImGuiCol_TitleBg] = curryMedium;
    colors[ImGuiCol_TitleBgActive] = curryLight;
    colors[ImGuiCol_Header] = curryMedium;
    colors[ImGuiCol_HeaderHovered] = curryLight;
    colors[ImGuiCol_HeaderActive] = spice;

    // チェック・スライダー
    colors[ImGuiCol_CheckMark] = rice;
    colors[ImGuiCol_SliderGrab] = rice;
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.85f, 0.55f, 1.0f);

    // スクロールバー・リサイズ
    colors[ImGuiCol_ScrollbarBg] = curryDark;
    colors[ImGuiCol_ScrollbarGrab] = curryMedium;
    colors[ImGuiCol_ScrollbarGrabHovered] = curryLight;
    colors[ImGuiCol_ScrollbarGrabActive] = spice;
    colors[ImGuiCol_ResizeGrip] = curryLight;
    colors[ImGuiCol_ResizeGripHovered] = spice;
    colors[ImGuiCol_ResizeGripActive] = spice;

    // 選択・分離線
    colors[ImGuiCol_Separator] = border;
    colors[ImGuiCol_SeparatorHovered] = spice;
    colors[ImGuiCol_SeparatorActive] = spice;

    // 設定（丸み・余白）
    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.ScrollbarRounding = 8.0f;
    style.WindowPadding = ImVec2(10, 8);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);
#endif // USE_IMGUI
}

void ImGuiTheme::SetCurryRiceTheme()
{
#ifdef USE_IMGUI
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- ベースカラー（カレーライス風） ---
    ImVec4 rice = ImVec4(0.95f, 0.93f, 0.85f, 1.0f);  // ごはん
    ImVec4 curry = ImVec4(0.50f, 0.33f, 0.10f, 1.0f);  // カレーのルー
    ImVec4 curryLight = ImVec4(0.70f, 0.45f, 0.15f, 1.0f);  // カレーのルー（明るめ）
    ImVec4 plate = ImVec4(0.80f, 0.80f, 0.83f, 1.0f);  // お皿
    ImVec4 fukujin = ImVec4(0.85f, 0.20f, 0.15f, 1.0f);  // 福神漬け（アクセント）
    ImVec4 text = ImVec4(0.15f, 0.10f, 0.05f, 1.0f);  // メイン文字
    ImVec4 textDisabled = ImVec4(0.45f, 0.40f, 0.35f, 1.0f);

    // --- ウィンドウ・背景系 ---
    colors[ImGuiCol_WindowBg] = plate;
    colors[ImGuiCol_ChildBg] = ImVec4(0.83f, 0.83f, 0.86f, 1.0f);
    colors[ImGuiCol_PopupBg] = plate;
    colors[ImGuiCol_Border] = ImVec4(0.6f, 0.55f, 0.45f, 1.0f);
    colors[ImGuiCol_Separator] = ImVec4(0.45f, 0.35f, 0.20f, 1.0f);
    colors[ImGuiCol_SeparatorHovered] = fukujin;
    colors[ImGuiCol_SeparatorActive] = fukujin;

    // メインメニューバー背景
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.90f, 0.80f, 0.60f, 1.0f); // 明るめのカレー色

    // --- テキスト ---
    colors[ImGuiCol_Text] = text;
    colors[ImGuiCol_TextDisabled] = textDisabled;

    // --- ボタン・フレーム系 ---
    colors[ImGuiCol_Button] = curry;
    colors[ImGuiCol_ButtonHovered] = fukujin;
    colors[ImGuiCol_ButtonActive] = ImVec4(0.95f, 0.30f, 0.25f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.55f, 0.40f, 0.15f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.70f, 0.50f, 0.20f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.85f, 0.60f, 0.25f, 1.0f);

    // --- タイトルバー・ヘッダー ---
    colors[ImGuiCol_TitleBg] = curry;
    colors[ImGuiCol_TitleBgActive] = curryLight;
    colors[ImGuiCol_Header] = curry;
    colors[ImGuiCol_HeaderHovered] = fukujin;
    colors[ImGuiCol_HeaderActive] = ImVec4(0.95f, 0.35f, 0.30f, 1.0f);

    // --- スライダー・チェック ---
    colors[ImGuiCol_CheckMark] = rice;
    colors[ImGuiCol_SliderGrab] = rice;
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.90f, 0.65f, 1.0f);

    // --- タブ ---
    colors[ImGuiCol_Tab] = curry;
    colors[ImGuiCol_TabHovered] = fukujin;
    colors[ImGuiCol_TabActive] = ImVec4(0.95f, 0.50f, 0.30f, 1.0f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.55f, 0.40f, 0.20f, 1.0f);
    colors[ImGuiCol_TabUnfocusedActive] = curry;

    // --- グリップ・スクロールバー ---
    colors[ImGuiCol_ResizeGrip] = curry;
    colors[ImGuiCol_ResizeGripHovered] = fukujin;
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.95f, 0.35f, 0.30f, 1.0f);
    colors[ImGuiCol_ScrollbarBg] = plate;
    colors[ImGuiCol_ScrollbarGrab] = curry;
    colors[ImGuiCol_ScrollbarGrabHovered] = fukujin;
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.95f, 0.30f, 0.25f, 1.0f);

    // --- 調整 ---
    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.ScrollbarRounding = 9.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(6, 4);
#endif // USE_IMGUI
}

void ImGuiTheme::SetBeefCurryTheme()
{
#ifdef USE_IMGUI
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text] = ImVec4(0.96f, 0.93f, 0.88f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.40f, 0.35f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.07f, 0.05f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.09f, 0.06f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.09f, 0.06f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.35f, 0.25f, 0.20f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.28f, 0.20f, 0.15f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.42f, 0.30f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.50f, 0.35f, 0.22f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.35f, 0.18f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.50f, 0.25f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.10f, 0.05f, 0.75f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.13f, 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.10f, 0.07f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.45f, 0.30f, 0.20f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.55f, 0.35f, 0.25f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.65f, 0.40f, 0.30f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.85f, 0.30f, 0.25f, 1.00f); // 福神漬け
    colors[ImGuiCol_SliderGrab] = ImVec4(0.65f, 0.40f, 0.25f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.75f, 0.45f, 0.25f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.45f, 0.25f, 0.15f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.60f, 0.35f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.70f, 0.40f, 0.25f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.55f, 0.30f, 0.15f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.70f, 0.35f, 0.20f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.80f, 0.45f, 0.25f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.40f, 0.25f, 0.18f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.60f, 0.35f, 0.25f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.80f, 0.40f, 0.25f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.55f, 0.30f, 0.20f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.70f, 0.40f, 0.25f, 0.70f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.90f, 0.50f, 0.30f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.55f, 0.15f, 0.10f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.80f, 0.25f, 0.15f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.90f, 0.30f, 0.20f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.35f, 0.10f, 0.08f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.60f, 0.20f, 0.15f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.85f, 0.30f, 0.15f, 0.75f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.08f, 0.06f, 0.04f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.80f, 0.60f, 0.45f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.80f, 0.60f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.55f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.70f, 0.30f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.25f, 0.15f, 0.10f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.35f, 0.25f, 0.18f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.25f, 0.18f, 0.13f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.18f, 0.13f, 0.09f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.22f, 0.15f, 0.10f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.80f, 0.30f, 0.20f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.95f, 0.40f, 0.25f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.95f, 0.40f, 0.25f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.50f, 0.30f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.10f, 0.05f, 0.03f, 0.60f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.05f, 0.03f, 0.60f);
#endif // USE_IMGUI
}

void ImGuiTheme::SetGreenCurryTheme()
{
#ifdef USE_IMGUI
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- 新しいベーストーン（抹茶感除去版） ---
    ImVec4 greenCurry = ImVec4(0.48f, 0.52f, 0.28f, 1.00f); // 黄み寄りのオリーブグリーン
    ImVec4 coconut = ImVec4(0.95f, 0.92f, 0.85f, 1.00f); // やや温かみのあるココナッツ
    ImVec4 lime = ImVec4(0.78f, 0.83f, 0.48f, 1.00f); // ライムの明るい緑（控えめ）
    ImVec4 chili = ImVec4(0.82f, 0.23f, 0.14f, 1.00f); // 唐辛子
    ImVec4 turmeric = ImVec4(0.88f, 0.74f, 0.30f, 1.00f); // 黄金スパイス
    ImVec4 leaf = ImVec4(0.16f, 0.19f, 0.11f, 1.00f); // 背景（深緑〜オリーブ）
    ImVec4 shadow = ImVec4(0.09f, 0.11f, 0.07f, 1.00f);

    // --- 背景 ---
    colors[ImGuiCol_WindowBg] = leaf;
    colors[ImGuiCol_ChildBg] = ImVec4(0.18f, 0.20f, 0.12f, 0.95f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.20f, 0.23f, 0.14f, 0.97f);

    // --- テキスト ---
    colors[ImGuiCol_Text] = coconut;
    colors[ImGuiCol_TextDisabled] = ImVec4(0.6f, 0.6f, 0.55f, 1.0f);

    // --- ボーダー ---
    colors[ImGuiCol_Border] = ImVec4(0.4f, 0.35f, 0.2f, 0.6f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

    // --- フレーム（入力欄など） ---
    colors[ImGuiCol_FrameBg] = ImVec4(0.28f, 0.32f, 0.18f, 0.9f);
    colors[ImGuiCol_FrameBgHovered] = greenCurry;
    colors[ImGuiCol_FrameBgActive] = lime;

    // --- タイトルバー ---
    colors[ImGuiCol_TitleBg] = greenCurry;
    colors[ImGuiCol_TitleBgActive] = turmeric;
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.18f, 0.10f, 0.75f);

    // --- メニューバー・スクロールバー ---
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.19f, 0.22f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = shadow;
    colors[ImGuiCol_ScrollbarGrab] = greenCurry;
    colors[ImGuiCol_ScrollbarGrabHovered] = turmeric;
    colors[ImGuiCol_ScrollbarGrabActive] = chili;

    // --- タブ ---
    colors[ImGuiCol_Tab] = greenCurry;
    colors[ImGuiCol_TabHovered] = turmeric;
    colors[ImGuiCol_TabActive] = coconut;
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.26f, 0.3f, 0.17f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.32f, 0.37f, 0.20f, 1.00f);

    // --- ボタン ---
    colors[ImGuiCol_Button] = greenCurry;
    colors[ImGuiCol_ButtonHovered] = turmeric;
    colors[ImGuiCol_ButtonActive] = chili;

    // --- チェック・スライダーなど ---
    colors[ImGuiCol_CheckMark] = coconut;
    colors[ImGuiCol_SliderGrab] = turmeric;
    colors[ImGuiCol_SliderGrabActive] = chili;

    // --- プログレスバー ---
    colors[ImGuiCol_PlotLines] = coconut;
    colors[ImGuiCol_PlotLinesHovered] = lime;
    colors[ImGuiCol_PlotHistogram] = greenCurry;
    colors[ImGuiCol_PlotHistogramHovered] = chili;

    // --- ナビゲーション・選択 ---
    colors[ImGuiCol_Header] = greenCurry;
    colors[ImGuiCol_HeaderHovered] = turmeric;
    colors[ImGuiCol_HeaderActive] = chili;

    colors[ImGuiCol_Separator] = greenCurry;
    colors[ImGuiCol_SeparatorHovered] = turmeric;
    colors[ImGuiCol_SeparatorActive] = chili;

    colors[ImGuiCol_ResizeGrip] = greenCurry;
    colors[ImGuiCol_ResizeGripHovered] = turmeric;
    colors[ImGuiCol_ResizeGripActive] = chili;

    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.85f, 0.8f, 0.4f, 0.35f);
    colors[ImGuiCol_NavHighlight] = turmeric;
    colors[ImGuiCol_NavWindowingHighlight] = lime;
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.2f, 0.2f, 0.1f, 0.6f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.1f, 0.1f, 0.05f, 0.6f);

    // --- 丸み・構成 ---
    style.WindowRounding = 7.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
#endif // USE_IMGUI

}

void ImGuiTheme::SetButterChickenCurryTheme()
{
#ifdef USE_IMGUI
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- ベースカラー（雰囲気の基盤） ---
    ImVec4 butter = ImVec4(0.97f, 0.83f, 0.54f, 1.00f); // バターのような黄色
    ImVec4 curry = ImVec4(0.82f, 0.39f, 0.16f, 1.00f); // 濃いバターチキンのルー
    ImVec4 tomato = ImVec4(0.93f, 0.47f, 0.33f, 1.00f); // トマト感
    ImVec4 cream = ImVec4(0.96f, 0.90f, 0.82f, 1.00f); // 生クリームの明るい部分
    ImVec4 rice = ImVec4(0.98f, 0.96f, 0.91f, 1.00f); // 白ごはん
    ImVec4 background = ImVec4(0.10f, 0.07f, 0.05f, 1.00f); // 落ち着いた背景

    // --- ウィンドウ・背景 ---
    colors[ImGuiCol_WindowBg] = background;
    colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.08f, 0.05f, 0.9f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.10f, 0.06f, 0.95f);

    // --- テキスト ---
    colors[ImGuiCol_Text] = rice;
    colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.55f, 0.50f, 1.00f);

    // --- ボーダー・セパレーター ---
    colors[ImGuiCol_Border] = ImVec4(0.50f, 0.25f, 0.10f, 0.60f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    // --- フレーム（入力欄、スライダーなど） ---
    colors[ImGuiCol_FrameBg] = ImVec4(0.24f, 0.12f, 0.08f, 0.9f);
    colors[ImGuiCol_FrameBgHovered] = tomato;
    colors[ImGuiCol_FrameBgActive] = curry;

    // --- タイトルバー ---
    colors[ImGuiCol_TitleBg] = curry;
    colors[ImGuiCol_TitleBgActive] = tomato;
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.08f, 0.05f, 0.75f);

    // --- メニューバー・スクロールバー ---
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.18f, 0.09f, 0.05f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.07f, 0.05f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = butter;
    colors[ImGuiCol_ScrollbarGrabHovered] = tomato;
    colors[ImGuiCol_ScrollbarGrabActive] = curry;

    // --- タブ ---
    colors[ImGuiCol_Tab] = curry;
    colors[ImGuiCol_TabHovered] = tomato;
    colors[ImGuiCol_TabActive] = butter;
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.25f, 0.15f, 0.08f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.35f, 0.20f, 0.10f, 1.00f);

    // --- ボタン ---
    colors[ImGuiCol_Button] = curry;
    colors[ImGuiCol_ButtonHovered] = tomato;
    colors[ImGuiCol_ButtonActive] = butter;

    // --- チェックボックス・スライダー・ドロップダウンなど ---
    colors[ImGuiCol_CheckMark] = butter;
    colors[ImGuiCol_SliderGrab] = butter;
    colors[ImGuiCol_SliderGrabActive] = tomato;

    // --- プログレスバー ---
    colors[ImGuiCol_PlotLines] = butter;
    colors[ImGuiCol_PlotLinesHovered] = tomato;
    colors[ImGuiCol_PlotHistogram] = curry;
    colors[ImGuiCol_PlotHistogramHovered] = tomato;

    // --- ナビゲーション・ドラッグ・選択 ---
    colors[ImGuiCol_Header] = curry;
    colors[ImGuiCol_HeaderHovered] = tomato;
    colors[ImGuiCol_HeaderActive] = butter;

    colors[ImGuiCol_Separator] = curry;
    colors[ImGuiCol_SeparatorHovered] = tomato;
    colors[ImGuiCol_SeparatorActive] = butter;

    colors[ImGuiCol_ResizeGrip] = curry;
    colors[ImGuiCol_ResizeGripHovered] = tomato;
    colors[ImGuiCol_ResizeGripActive] = butter;

    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.93f, 0.47f, 0.33f, 0.35f);
    colors[ImGuiCol_NavHighlight] = tomato;
    colors[ImGuiCol_NavWindowingHighlight] = butter;
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.20f, 0.10f, 0.05f, 0.60f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.05f, 0.03f, 0.60f);

    // --- スタイル設定 ---
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.WindowRounding = 8.0f;
    style.ScrollbarRounding = 6.0f;
    style.TabRounding = 6.0f;
#endif // USE_IMGUI
}