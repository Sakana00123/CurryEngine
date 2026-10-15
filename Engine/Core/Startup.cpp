#include "Startup.h"
#include "Engine/Automation/SolutionGenerator.h"
#include "Engine/Automation/ProjectCreator.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <shellapi.h>
#include "External/tinygltf-release/json.hpp"
#include <stdexcept>
#include <fstream>

#pragma comment(lib, "Shell32.lib")

namespace
{
    std::filesystem::path startupLog;
    const char* startupStage = "Starting engine";

    std::wstring FromUtf8(const std::string& text)
    {
        const auto count = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
        std::wstring result(count, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), count);
        return result;
    }

    std::filesystem::path ExecutablePath()
    {
        std::wstring buffer(512, L'\0');
        for (;;)
        {
            const auto size = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
            if (!size) throw std::runtime_error("Cannot determine executable path.");
            if (size < buffer.size()) return std::filesystem::path(buffer.substr(0, size));
            if (buffer.size() >= 32768) throw std::runtime_error("Executable path is too long.");
            buffer.resize(buffer.size() * 2);
        }
    }

    std::vector<std::wstring> Arguments()
    {
        int count = 0;
        auto argv = CommandLineToArgvW(GetCommandLineW(), &count);
        if (!argv) throw std::runtime_error("Cannot parse command line.");
        struct ArgumentGuard
        {
            LPWSTR* value;
            ~ArgumentGuard() { LocalFree(value); }
        } guard{ argv };
        return std::vector<std::wstring>(argv + 1, argv + count);
    }

    std::string Utf8(const std::filesystem::path& path)
    {
        const auto text = path.generic_u8string();
        return std::string(text.begin(), text.end());
    }

    // GUI subsystem processes still support redirected standard handles (e.g. from Hub).
    bool WriteResult(DWORD stream, const std::string& text)
    {
        auto handle = GetStdHandle(stream);
        if (!handle || handle == INVALID_HANDLE_VALUE) return false;
        DWORD mode = 0;
        if (GetConsoleMode(handle, &mode))
        {
            const auto wide = FromUtf8(text);
            DWORD written = 0;
            return WriteConsoleW(handle, wide.data(), static_cast<DWORD>(wide.size()), &written, nullptr) != FALSE;
        }
        size_t offset = 0;
        while (offset < text.size())
        {
            DWORD written = 0;
            if (!WriteFile(handle, text.data() + offset, static_cast<DWORD>(text.size() - offset), &written, nullptr) || !written)
                return false;
            offset += written;
        }
        return true;
    }
}

namespace CurryEngine
{
    void BeginStartupDiagnostics(const ProjectContext& context) noexcept
    {
        try
        {
            startupLog = context.projectRoot / "Logs/startup.log";
            std::filesystem::create_directories(startupLog.parent_path());
            std::ofstream output(startupLog, std::ios::binary | std::ios::app);
            output << "\n--- Engine startup ---\nProject: " << Utf8(context.projectFile)
                   << "\nEngine: " << Utf8(context.engineRoot) << '\n';
        }
        catch (...) { /* A diagnostic failure must not prevent startup. */ }
    }

    void RecordStartupStage(const char* stage) noexcept
    {
        startupStage = stage;
        try
        {
            if (startupLog.empty()) return;
            std::ofstream output(startupLog, std::ios::binary | std::ios::app);
            output << "Stage: " << stage << '\n';
        }
        catch (...) { }
    }

    void ReportRuntimeFailure(const char* error) noexcept
    {
        try
        {
            const std::string message = std::string("Engine failed during: ") + startupStage + "\n" + error;
            bool saved = false;
            if (!startupLog.empty())
            {
                std::ofstream output(startupLog, std::ios::binary | std::ios::app);
                output << "ERROR: " << message << '\n';
                output.flush();
                saved = output.good();
            }
            WriteResult(STD_ERROR_HANDLE, message + "\n");
            auto display = FromUtf8(message);
            if (saved) display += L"\n\nLog: " + startupLog.wstring();
            MessageBoxW(nullptr, display.c_str(), L"CurryEngine startup error", MB_OK | MB_ICONERROR);
        }
        catch (...)
        {
            MessageBoxW(nullptr, L"The engine stopped unexpectedly. Startup diagnostics could not be written.",
                L"CurryEngine startup error", MB_OK | MB_ICONERROR);
        }
    }

    void NotifyProjectReady(const ProjectContext& context)
    {
        RecordStartupStage("First frame completed");
        if (context.IsExternalProject())
            WriteResult(STD_OUTPUT_HANDLE, nlohmann::json({{"ready", true}, {"projectFile", Utf8(context.projectFile)}}).dump() + "\n");
    }
    std::optional<int> PrepareStartup(ProjectContext& context)
    {
        try
        {
            const auto request = ParseStartupArguments(Arguments());
            if (request.mode == StartupMode::CreateProject)
            {
                const auto file = CreateProject(request, ExecutablePath(), std::filesystem::current_path());
                WriteResult(STD_OUTPUT_HANDLE, nlohmann::json({{"created", true}, {"projectFile", Utf8(file)}}).dump() + "\n");
                return 0;
            }
            auto resolved = ResolveProjectContext(request, ExecutablePath(), std::filesystem::current_path());
            if (request.mode == StartupMode::ValidateProject)
            {
                const nlohmann::json result = {
                    { "valid", true }, { "engineRoot", Utf8(resolved.engineRoot) },
                    { "projectRoot", Utf8(resolved.projectRoot) }, { "projectFile", Utf8(resolved.projectFile) },
                    { "name", resolved.projectName }, { "engineVersion", resolved.engineVersion }
                };
                WriteResult(STD_OUTPUT_HANDLE, result.dump() + "\n");
                return 0;
            }
            if (request.mode == StartupMode::OpenProject || request.mode == StartupMode::GenerateSolution)
            {
                if (request.mode == StartupMode::OpenProject)
                {
#ifndef _DEBUG
                    throw std::runtime_error("Opening an external project requires an editor build.");
#endif
                    for (const auto* directory : {"Assets", "Assets/Scenes", "ProjectSettings"})
                        if (!std::filesystem::is_directory(resolved.projectRoot / directory))
                            throw std::runtime_error(std::string("Missing project directory: ") + directory);
                    for (const auto* directory : {"Data/Icon", "Data/Fonts", "Shader", "Assets/Shaders"})
                        if (!std::filesystem::is_directory(resolved.resourceRoot / directory))
                            throw std::runtime_error(std::string("Missing engine resource directory: ") + directory);
                }
                SolutionGenerator::Generate(resolved);
                if (request.mode == StartupMode::GenerateSolution)
                {
                    WriteResult(STD_OUTPUT_HANDLE, "{\"generated\":true}\n");
                    return 0;
                }
                for (const auto* directory : {"Library/ShaderCache", "Library/ScriptAssemblies", "Temp", "Logs"})
                    std::filesystem::create_directories(resolved.projectRoot / directory);
                std::filesystem::current_path(resolved.projectRoot);
            }
            context = std::move(resolved);
            return std::nullopt;
        }
        catch (const std::exception& error)
        {
            const nlohmann::json result = { { "valid", false }, { "error", error.what() } };
            // Invalid arguments/projects never enter rendering or managed initialization.
            const auto message = result.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace) + "\n";
            if (!WriteResult(STD_ERROR_HANDLE, message))
                MessageBoxW(nullptr, FromUtf8(message).c_str(), L"CurryEngine startup", MB_OK | MB_ICONERROR);
            return 1;
        }
    }
}
