#include "ProjectCreator.h"
#include "SolutionGenerator.h"
#include "External/tinygltf-release/json.hpp"
#include <fstream>
#include <random>
#include <stdexcept>

namespace fs = std::filesystem;
namespace CurryEngine
{
    fs::path CreateProject(const StartupRequest& request, const fs::path& executable, const fs::path& launchDirectory)
    {
        const auto engine = fs::canonical(executable).parent_path();
        std::ifstream manifestFile(engine / "engine.json");
        auto manifest = nlohmann::json::parse(manifestFile);
        if (!manifest.is_object() || !manifest.contains("formatVersion") ||
            !manifest["formatVersion"].is_number_integer() || manifest["formatVersion"] != 1 ||
            !manifest.contains("version") || !manifest["version"].is_string() ||
            manifest["version"].get<std::string>().find_first_not_of(" \t\r\n") == std::string::npos)
            throw std::runtime_error("Invalid engine manifest.");
        const auto name8 = fs::path(request.projectName).generic_u8string();
        const std::string name(name8.begin(), name8.end());
        if (name.find_first_not_of(" \t\r\n") == std::string::npos || name.find('\0') != std::string::npos)
            throw std::runtime_error("A project name is required.");
        auto target = (request.projectDirectory.is_absolute() ? request.projectDirectory : launchDirectory / request.projectDirectory).lexically_normal();
        if (target.filename().empty() || fs::exists(target))
            throw std::runtime_error("The project destination must not exist.");
        const auto parent = fs::canonical(target.parent_path());
        target = parent / target.filename();
        for (auto path = parent; !path.empty(); path = path.parent_path())
        {
            if (fs::equivalent(path, engine)) throw std::runtime_error("Create the project outside the engine installation.");
            if (path == path.parent_path()) break;
        }
        const auto templateRoot = engine / "Templates/Empty";
        const char* files[] = {"Assets/Scenes/Main.scene", "Assets/Scripts/HelloCurry.cs",
            "ProjectSettings/settings.json", "ProjectSettings/EditorConfig.json", "ProjectSettings/Physics.json", "ProjectSettings/Input.json"};
        for (auto file : files)
            if (!fs::is_regular_file(templateRoot / file)) throw std::runtime_error(std::string("Missing template file: ") + file);
        // Build in a sibling directory; only publish a complete project. Keep failed staging
        // directories for diagnostics instead of deleting any user-selected destination.
        fs::path staging;
        std::random_device random;
        for (int attempt = 0; attempt < 16; ++attempt)
        {
            auto candidate = parent / (".curry-create-" + std::to_string(random()) + "-" + std::to_string(random()));
            if (fs::create_directory(candidate)) { staging = candidate; break; }
        }
        if (staging.empty()) throw std::runtime_error("Cannot create project staging directory.");
        try
        {
            for (auto file : files)
            {
                fs::create_directories((staging / file).parent_path());
                fs::copy_file(templateRoot / file, staging / file);
            }
            const nlohmann::json project = {{"formatVersion", 1}, {"name", name}, {"engineVersion", manifest["version"]}};
            std::ofstream output(staging / "Project.curryproject", std::ios::binary);
            output << project.dump(2) << '\n';
            output.close();
            if (!output) throw std::runtime_error("Cannot write project file.");
            auto context = ResolveProjectContext({StartupMode::GenerateSolution, staging / "Project.curryproject"}, executable, launchDirectory);
            SolutionGenerator::Generate(context);
            for (auto dir : {"Library/ScriptAssemblies", "Library/ShaderCache", "Temp", "Logs"}) fs::create_directories(staging / dir);
            fs::rename(staging, target);
        }
        catch (const std::exception& error)
        {
            const auto path8 = staging.generic_u8string();
            throw std::runtime_error(std::string(error.what()) + " Staging: " + std::string(path8.begin(), path8.end()));
        }
        return target / "Project.curryproject";
    }
}
