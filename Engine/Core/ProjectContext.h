#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace CurryEngine
{
    // Resolved before any subsystem starts. Reading a project never creates files.
    struct ProjectContext
    {
        std::filesystem::path engineRoot;
        std::filesystem::path projectRoot;
        std::filesystem::path projectFile;
        std::string projectName;
        std::string engineVersion;
        std::filesystem::path resourceRoot;

        bool IsExternalProject() const { return !projectFile.empty(); }
    };

    enum class StartupMode { Legacy, ValidateProject, OpenProject, GenerateSolution, CreateProject };

    struct StartupRequest
    {
        StartupMode mode = StartupMode::Legacy;
        std::filesystem::path projectFile;
        std::filesystem::path projectDirectory;
        std::wstring projectName;
    };

    // Arguments exclude argv[0]. Relative project paths are relative to the launch directory.
    StartupRequest ParseStartupArguments(const std::vector<std::wstring>& arguments);
    ProjectContext ResolveProjectContext(const StartupRequest& request,
        const std::filesystem::path& executablePath,
        const std::filesystem::path& launchDirectory);
}
