#include "Engine/Core/Startup.h"
#include "Engine/Core/EnginePaths.h"
#include "External/tinygltf-release/json.hpp"
#include <iostream>

int main()
{
    CurryEngine::ProjectContext context;
    if (const auto exitCode = CurryEngine::PrepareStartup(context)) return *exitCode;
    if (context.IsExternalProject())
    {
        EnginePaths::StartupContext = context;
        EnginePaths::ConfigureResourcePaths();
        std::cout << nlohmann::json({
            {"opened", true}, {"cwd", EnginePaths::Utf8(std::filesystem::current_path())},
            {"shader", EnginePaths::Utf8(EnginePaths::ResolveResource(std::filesystem::path(EnginePaths::ShaderSourceDir) / "test.hlsl"))},
            {"cache", EnginePaths::Utf8(EnginePaths::ShaderCache())},
            {"asset", EnginePaths::Utf8(EnginePaths::ResolveResource("Assets/Shaders/custom.hlsl"))}
        }).dump() << '\n';
        return 0;
    }
    std::cout << "LEGACY_STARTUP\n";
    return 0;
}
