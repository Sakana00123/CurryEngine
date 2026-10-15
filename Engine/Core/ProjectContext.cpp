#include "ProjectContext.h"
#include "External/tinygltf-release/json.hpp"
#include <fstream>
#include <stdexcept>

namespace fs = std::filesystem;
using nlohmann::json;

namespace
{
    std::string Utf8(const fs::path& path)
    {
        const auto text = path.generic_u8string();
        return std::string(text.begin(), text.end());
    }

    json ReadObject(const fs::path& path)
    {
        std::ifstream stream(path, std::ios::binary);
        if (!stream) throw std::runtime_error("Cannot open JSON file: " + Utf8(path));
        auto value = json::parse(stream);
        if (!value.is_object()) throw std::runtime_error("Expected a JSON object: " + Utf8(path));
        return value;
    }

    std::string RequiredString(const json& value, const char* key)
    {
        if (!value.contains(key) || !value.at(key).is_string())
            throw std::runtime_error(std::string("Missing or invalid string: ") + key);
        auto result = value.at(key).get<std::string>();
        if (result.empty() || result.find_first_not_of(" \t\r\n") == std::string::npos ||
            result.find('\0') != std::string::npos)
            throw std::runtime_error(std::string("Empty or invalid string: ") + key);
        return result;
    }

    void CheckFormat(const json& value)
    {
        if (!value.contains("formatVersion") || !value.at("formatVersion").is_number_integer() ||
            value.at("formatVersion") != 1)
            throw std::runtime_error("Unsupported formatVersion (expected 1).");
    }

    bool IsWithin(const fs::path& child, const fs::path& parent)
    {
        // Compare existing ancestors by filesystem identity (case and junctions on Windows).
        for (auto current = child; !current.empty(); current = current.parent_path())
        {
            if (fs::equivalent(current, parent)) return true;
            if (current == current.parent_path()) break;
        }
        return false;
    }
}

namespace CurryEngine
{
    StartupRequest ParseStartupArguments(const std::vector<std::wstring>& arguments)
    {
        if (arguments.empty()) return {};
        if (arguments.size() == 4 && arguments[0] == L"--create-project" && arguments[2] == L"--name" &&
            !arguments[1].empty() && !arguments[3].empty())
            return {StartupMode::CreateProject, {}, fs::path(arguments[1]), arguments[3]};
        // Existing Hub passes one bare path, which the old entry point ignored.
        // Preserve that legacy launch until Hub is migrated to the explicit --project contract.
        if (arguments.size() == 1 && !arguments[0].empty() && !arguments[0].starts_with(L"--"))
            return {};
        if (arguments.size() != 2 || (arguments[0] != L"--validate-project" &&
            arguments[0] != L"--project" && arguments[0] != L"--generate-solution") ||
            arguments[1].empty() || arguments[1].starts_with(L"--"))
            throw std::runtime_error("Usage: CurryEngine.exe [--validate-project | --project | --generate-solution] <file.curryproject>");
        const auto mode = arguments[0] == L"--project" ? StartupMode::OpenProject :
            arguments[0] == L"--generate-solution" ? StartupMode::GenerateSolution : StartupMode::ValidateProject;
        return { mode, fs::path(arguments[1]) };
    }

    ProjectContext ResolveProjectContext(const StartupRequest& request,
        const fs::path& executablePath, const fs::path& launchDirectory)
    {
        if (!executablePath.is_absolute() || !launchDirectory.is_absolute())
            throw std::runtime_error("Executable and launch directory must be absolute paths.");

        ProjectContext result;
        result.engineRoot = fs::canonical(executablePath).parent_path();
        result.projectRoot = fs::canonical(launchDirectory);
        if (request.mode == StartupMode::Legacy) return result;

        if (request.projectFile.empty() || request.projectFile.extension() != L".curryproject")
            throw std::runtime_error("A .curryproject file is required.");
        const auto file = request.projectFile.is_absolute() ? request.projectFile : launchDirectory / request.projectFile;
        if (!fs::is_regular_file(file)) throw std::runtime_error("Project file does not exist.");
        result.projectFile = fs::canonical(file);
        result.projectRoot = result.projectFile.parent_path();
        if (IsWithin(result.projectRoot, result.engineRoot) || IsWithin(result.engineRoot, result.projectRoot))
            throw std::runtime_error("Project and engine directories must be separate.");

        const auto project = ReadObject(result.projectFile);
        CheckFormat(project);
        result.projectName = RequiredString(project, "name");
        result.engineVersion = RequiredString(project, "engineVersion");

        // A missing manifest cannot silently bypass version pinning.
        const auto engine = ReadObject(result.engineRoot / "engine.json");
        CheckFormat(engine);
        if (RequiredString(engine, "version") != result.engineVersion)
            throw std::runtime_error("The project engineVersion does not match this engine installation.");
        result.resourceRoot = result.engineRoot / "Resources";
        return result;
    }
}
