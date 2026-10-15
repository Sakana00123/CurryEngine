#include "Engine/Core/ProjectContext.h"
#include "External/tinygltf-release/json.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;
using namespace CurryEngine;
using nlohmann::json;

namespace
{
    int checks = 0;

    void Check(bool condition, const char* message)
    {
        ++checks;
        if (!condition) throw std::runtime_error(message);
    }

    template<class Function>
    void Reject(Function function, const char* message)
    {
        bool rejected = false;
        try { function(); }
        catch (const std::exception&) { rejected = true; }
        Check(rejected, message);
    }

    void Write(const fs::path& path, const std::string& content)
    {
        fs::create_directories(path.parent_path());
        std::ofstream stream(path, std::ios::binary);
        stream << content;
        if (!stream) throw std::runtime_error("Cannot create fixture.");
    }

    std::string Read(const fs::path& path)
    {
        std::ifstream stream(path, std::ios::binary);
        return { std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };
    }
}

int wmain(int argc, wchar_t** argv)
{
    try
    {
        if (argc != 2) throw std::runtime_error("Expected a new, empty fixture directory.");
        const auto root = fs::absolute(argv[1]);
        if (fs::exists(root)) throw std::runtime_error("Fixture directory already exists.");
        const auto engine = root / L"Engine 日本語 & spaces";
        const auto project = root / L"作品 A & spaces";
        const auto executable = engine / "CurryEngine.exe";
        const auto manifest = engine / "engine.json";
        const auto file = project / L"作品.curryproject";
        const std::string valid = R"({"formatVersion":1,"name":"My Game","engineVersion":"0.1.0"})";
        Write(executable, "fixture");
        Write(manifest, R"({"formatVersion":1,"version":"0.1.0"})");
        Write(file, valid);
        const auto originalCwd = fs::current_path();

        Check(ParseStartupArguments({}).mode == StartupMode::Legacy, "No-argument compatibility.");
        Check(ParseStartupArguments({file.wstring()}).mode == StartupMode::Legacy, "Existing Hub launch compatibility.");
        Check(ParseStartupArguments({L"--project", file.wstring()}).mode == StartupMode::OpenProject, "Open mode.");
        Check(ParseStartupArguments({L"--generate-solution", file.wstring()}).mode == StartupMode::GenerateSolution, "Generate mode.");
        for (const auto& args : std::vector<std::vector<std::wstring>>{
            {L"--validate-project"}, {L"--validate-project", L""},
            {L"--validate-project", file.wstring(), L"extra"}, {L"--unknown", file.wstring()},
            {L"--project"},
            {L"--validate-project", L"--unknown"}})
            Reject([&] { ParseStartupArguments(args); }, "Invalid arguments accepted.");

        const auto request = ParseStartupArguments({L"--validate-project", file.wstring()});
        const auto resolved = ResolveProjectContext(request, executable, root);
        Check(resolved.IsExternalProject(), "Expected an external project.");
        Check(fs::equivalent(resolved.engineRoot, engine), "Engine root must be executable-relative.");
        Check(fs::equivalent(resolved.projectRoot, project), "Project root must be project-file-relative.");
        Check(resolved.projectName == "My Game" && resolved.engineVersion == "0.1.0", "Metadata lost.");
        Check(fs::current_path() == originalCwd, "Resolver changed the working directory.");
        Check(Read(file) == valid, "Resolver changed the project file.");
        Check(!fs::exists(project / "Library"), "Validation created project output.");

        const auto relative = StartupRequest{StartupMode::ValidateProject, fs::relative(file, root)};
        Check(ResolveProjectContext(relative, executable, root).projectFile == resolved.projectFile,
            "Relative project path ignored the launch directory.");
        const auto legacy = ResolveProjectContext({}, executable, project);
        Check(!legacy.IsExternalProject() && fs::equivalent(legacy.projectRoot, project),
            "Legacy working directory changed.");

        const auto second = root / L"作品 B" / "Other.curryproject";
        Write(second, valid);
        Check(ResolveProjectContext({StartupMode::ValidateProject, second}, executable, root).projectRoot != resolved.projectRoot,
            "Two projects share a project root.");

        const std::vector<std::string> invalid = {
            "{", "[]", "null", valid + " trailing", "{}",
            R"({"formatVersion":2,"name":"Game","engineVersion":"0.1.0"})",
            R"({"formatVersion":1.0,"name":"Game","engineVersion":"0.1.0"})",
            R"({"formatVersion":true,"name":"Game","engineVersion":"0.1.0"})",
            R"({"formatVersion":1,"name":42,"engineVersion":"0.1.0"})",
            R"({"formatVersion":1,"name":" ","engineVersion":"0.1.0"})",
            R"({"formatVersion":1,"name":"Game\u0000","engineVersion":"0.1.0"})",
            R"({"formatVersion":1,"name":"Game","engineVersion":""})",
            R"({"formatVersion":1,"name":"Game","engineVersion":"0.2.0"})"
        };
        for (const auto& value : invalid)
        {
            Write(file, value);
            Reject([&] { ResolveProjectContext(request, executable, root); }, "Invalid project accepted.");
            Check(Read(file) == value, "Rejected input was rewritten.");
        }
        Write(file, "\xEF\xBB\xBF" + valid);
        Check(ResolveProjectContext(request, executable, root).engineVersion == "0.1.0", "UTF-8 BOM rejected.");
        Write(file, valid);

        for (const auto& value : std::vector<std::string>{"{}", "[1]", "{", R"({"formatVersion":2,"version":"0.1.0"})",
            R"({"formatVersion":1,"version":9})", R"({"formatVersion":1,"version":"0.2.0"})"})
        {
            Write(manifest, value);
            Reject([&] { ResolveProjectContext(request, executable, root); }, "Invalid engine manifest accepted.");
        }
        fs::rename(manifest, engine / "engine.saved.json");
        Reject([&] { ResolveProjectContext(request, executable, root); }, "Missing engine manifest accepted.");
        Check(!fs::exists(manifest), "Missing engine manifest was created.");
        Write(manifest, R"({"formatVersion":1,"version":"0.1.0"})");

        Reject([&] { ResolveProjectContext({StartupMode::ValidateProject, project / "missing.curryproject"}, executable, root); }, "Missing file accepted.");
        Check(!fs::exists(project / "missing.curryproject"), "Missing project was created.");
        Reject([&] { ResolveProjectContext({StartupMode::ValidateProject, project}, executable, root); }, "Directory accepted.");
        Write(project / "wrong.json", valid);
        Reject([&] { ResolveProjectContext({StartupMode::ValidateProject, project / "wrong.json"}, executable, root); }, "Wrong extension accepted.");
        const auto nested = engine / "nested" / "bad.curryproject";
        Write(nested, valid);
        Reject([&] { ResolveProjectContext({StartupMode::ValidateProject, nested}, executable, root); }, "Project inside engine accepted.");
        Write(root / "ancestor.curryproject", valid);
        Reject([&] { ResolveProjectContext({StartupMode::ValidateProject, root / "ancestor.curryproject"}, executable, root); }, "Engine inside project accepted.");
        Reject([&] { ResolveProjectContext(request, "relative.exe", root); }, "Relative executable accepted.");
        Check(fs::current_path() == originalCwd, "Error changed working directory.");

        std::cout << checks << " checks passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
