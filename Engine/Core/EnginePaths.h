#pragma once
#include <filesystem>
#include "ProjectContext.h"

/** @brief エンジンが使用する標準パスを定義します。 */
namespace EnginePaths
{
    // Initialized once in main, before subsystem construction.
    inline CurryEngine::ProjectContext StartupContext;
    inline const std::filesystem::path& EngineRoot() { return StartupContext.engineRoot; }
    inline const std::filesystem::path& ProjectRoot() { return StartupContext.projectRoot; }
    inline bool IsExternalProject() { return StartupContext.IsExternalProject(); }
    inline std::filesystem::path ScriptProject() { return ProjectRoot() / "Assembly-CSharp.csproj"; }
    inline std::filesystem::path ScriptAssembly() { return ProjectRoot() / "Library/ScriptAssemblies/Assembly-CSharp.dll"; }
    inline std::filesystem::path ProjectSolution() { auto path = StartupContext.projectFile; return path.replace_extension(".sln"); }
    inline std::string Utf8(const std::filesystem::path& path)
    {
        const auto value = path.generic_u8string();
        return {value.begin(), value.end()};
    }
	// リソースパスの解決。"./Assets/..."や"./Data/..."などの相対パスを、外部プロジェクトの場合はプロジェクトルートからの絶対パスに変換する。
    inline std::filesystem::path ResolveResource(const std::filesystem::path& path)
    {
        if (!IsExternalProject() || path.is_absolute()) return path;
        const auto normalized = path.lexically_normal();
        const auto value = normalized.generic_u8string();
        if (value.starts_with(u8"EngineResources/Shaders/"))
            return StartupContext.resourceRoot / "Assets/Shaders" / normalized.lexically_relative("EngineResources/Shaders");
        if (value.starts_with(u8"EngineResources/Shader/"))
            return StartupContext.resourceRoot / "Shader" / normalized.lexically_relative("EngineResources/Shader");
        if (value.starts_with(u8"Data/") || value.starts_with(u8"Shader/"))
            return StartupContext.resourceRoot / normalized;
        return path;
    }
    inline std::filesystem::path ShaderCache()
    {
        return IsExternalProject() ? ProjectRoot() / "Library/ShaderCache" : std::filesystem::path("./Assets/Shaders");
    }

    // アセット関連
    inline constexpr const char* AssetsDir       = "./Assets/";
    inline constexpr const char* ScenesDir       = "./Assets/Scenes/";

    // データ関連（ビルド済みリソースなど）
    inline constexpr const char* DataDir         = "./Data/";
    inline std::string ShadersDataDir  = "./Assets/Shaders/"; // CSOファイルのロード元など
    inline constexpr const char* ImagesDataDir   = "./Data/Images/";
    inline constexpr const char* IconsDir        = "./Data/Icon/";
	inline const std::filesystem::path ArtifactDir = ".\\Library\\Artifacts\\"; // ビルド済みリソースの保存先

    // ソースコード関連
    inline std::string ShaderSourceDir = "./Shader/";       // HLSL・HLSLIファイルの配置先

    // エディタ・プロジェクト関連
    inline constexpr const char* ProjectSettingsFile = "./ProjectSettings/settings.json";
	inline constexpr const char* PhysicsSettingsFile = "./ProjectSettings/Physics.json";
	inline constexpr const char* InputSettingsFile = "./ProjectSettings/Input.json";
	inline constexpr const char* EditorConfigFile = "./ProjectSettings/EditorConfig.json";
    inline constexpr const char* SolutionFile        = "./CurryEngine.sln";

	inline void ConfigureResourcePaths()
    {
        ShadersDataDir = IsExternalProject() ? "./EngineResources/Shaders/" : "./Assets/Shaders/";
        ShaderSourceDir = IsExternalProject() ? "./EngineResources/Shader/" : "./Shader/";
    }

    inline std::filesystem::path ShaderSourceFor(std::filesystem::path path)
    {
        path.replace_extension(".hlsl");
        if (IsExternalProject() && path.lexically_normal().generic_u8string().starts_with(u8"Assets/"))
            return path;
        return std::filesystem::path(ShaderSourceDir) / path.filename();
    }

    // ツール関連
	inline constexpr const char* TracyProfilerExe = "./Tools/tracy/windows-0.13.1/tracy-profiler.exe";
}
