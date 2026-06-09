#pragma once

namespace EnginePaths
{
    // アセット関連
    inline constexpr const char* AssetsDir       = "./Assets/";
    inline constexpr const char* ScenesDir       = "./Assets/Scenes/";

    // データ関連（ビルド済みリソースなど）
    inline constexpr const char* DataDir         = "./Data/";
    inline constexpr const char* ShadersDataDir  = "./Assets/Shaders/"; // CSOファイルのロード元など
    inline constexpr const char* ImagesDataDir   = "./Data/Images/";
    inline constexpr const char* IconsDir        = "./Data/Icon/";

    // ソースコード関連
    inline constexpr const char* ShaderSourceDir = "./Shader/";       // HLSL・HLSLIファイルの配置先

    // エディタ・プロジェクト関連
    inline constexpr const char* ProjectSettingsFile = "./ProjectSettings/settings.json";
	inline constexpr const char* PhysicsSettingsFile = "./ProjectSettings/Physics.json";
	inline constexpr const char* EditorConfigFile = "./ProjectSettings/EditorConfig.json";
    inline constexpr const char* SolutionFile        = "./CurryEngine.sln";
}