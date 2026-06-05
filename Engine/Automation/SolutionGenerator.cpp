#include "pch.h"
#include "SolutionGenerator.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

// ランダムなGUIDもどきが必要なので固定値でOK（変わらないなら）
// 本来はCoCreateGuid()で生成するが、固定でも動く
static const char* PROJECT_GUID = "{8A5B4A7C-1234-4321-ABCD-000000000001}";
static const char* SOLUTION_GUID = "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}"; // C#プロジェクト種別

bool SolutionGenerator::Generate(const std::string& projectRoot,
    const std::string& projectName,
    const std::string& runtimeDllPath)
{
    fs::path root(projectRoot);

    // --- Assembly-CSharp.csproj ---
    std::string csproj =
        R"(<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <OutputType>Library</OutputType>
    <TargetFramework>net8.0</TargetFramework>
    <Nullable>enable</Nullable>
    <ImplicitUsings>disable</ImplicitUsings>
    <AssemblyName>Assembly-CSharp</AssemblyName>
  </PropertyGroup>

  <ItemGroup>
    <Compile Include="**\*.cs" />
  </ItemGroup>

  <ItemGroup>
    <Reference Include="CurryEngine.Runtime">
      <HintPath>)" + runtimeDllPath + R"(</HintPath>
    </Reference>
  </ItemGroup>

</Project>
)";

    // --- MyProject.sln ---
    std::string sln =
        "\xEF\xBB\xBF\n"   // UTF-8 BOM（VS要求）
        "Microsoft Visual Studio Solution File, Format Version 12.00\n"
        "# Visual Studio Version 17\n"
        "VisualStudioVersion = 17.0.31903.59\n"
        "Project(\"" + std::string(SOLUTION_GUID) + "\") "
        "= \"Assembly-CSharp\", \"Assembly-CSharp.csproj\", \"" + std::string(PROJECT_GUID) + "\"\n"
        "EndProject\n"
        "Global\n"
        "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n"
        "\t\tDebug|Any CPU = Debug|Any CPU\n"
        "\t\tRelease|Any CPU = Release|Any CPU\n"
        "\tEndGlobalSection\n"
        "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n"
        "\t\t" + std::string(PROJECT_GUID) + ".Debug|Any CPU.ActiveCfg = Debug|Any CPU\n"
        "\t\t" + std::string(PROJECT_GUID) + ".Release|Any CPU.ActiveCfg = Release|Any CPU\n"
        "\tEndGlobalSection\n"
        "EndGlobal\n";

    // 書き出し
    auto writeFile = [](const fs::path& path, const std::string& content) -> bool
        {
            std::ofstream f(path);
            if (!f.is_open()) return false;
            f << content;
            return true;
        };

    if (!writeFile(root / "Assembly-CSharp.csproj", csproj)) return false;
    if (!writeFile(root / (projectName + ".sln"), sln))    return false;

    return true;
}