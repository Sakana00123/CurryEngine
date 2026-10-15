#include "Engine/Editor/LegacyModelCache.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2) throw std::runtime_error("Expected an unused test directory.");
        const auto root = fs::absolute(argv[1]);
        if (fs::exists(root)) throw std::runtime_error("Test directory already exists.");
        fs::create_directories(root / "Assets/Scenes");
        fs::create_directories(root / "Assets/Scripts");
        const auto models = root / "Assets/Models";
        int checks = 0;
        const auto check = [&](bool condition, const char* message)
        {
            if (!condition) throw std::runtime_error(message);
            ++checks;
        };
        const auto write = [](const fs::path& file)
        {
            std::ofstream output(file);
            output << "keep or remove fixture";
            if (!output) throw std::runtime_error("Could not write fixture.");
        };

        // Reproduce Empty-template/Test01 layout: Models has never existed.
        check(CurryEngine::RemoveLegacyModelCaches(models).empty(), "Missing Models must be accepted.");
        check(!fs::exists(models), "Cleanup must not create optional directories.");
        write(models);
        check(CurryEngine::RemoveLegacyModelCaches(models).empty(), "A file named Models must be ignored.");
        check(fs::is_regular_file(models), "Cleanup changed the Models file.");
        fs::remove(models);
        fs::create_directories(models / "Nested");
        check(CurryEngine::RemoveLegacyModelCaches(models).empty(), "An empty Models directory must be accepted.");
        write(models / "old.cereal");
        write(models / "Nested/old.batchCereal");
        write(models / "model.fbx");
        write(models / "Nested/material.json");
        write(root / "Assets/Scenes/keep.cereal");
        const auto removed = CurryEngine::RemoveLegacyModelCaches(models);
        check(removed.size() == 2, "Both legacy cache formats must be removed.");
        check(!fs::exists(models / "old.cereal"), "Root cache still exists.");
        check(!fs::exists(models / "Nested/old.batchCereal"), "Nested cache still exists.");
        check(fs::exists(models / "model.fbx") && fs::exists(models / "Nested/material.json"), "Model sources changed.");
        check(fs::exists(root / "Assets/Scenes/keep.cereal"), "Cleanup escaped Models.");
        check(CurryEngine::RemoveLegacyModelCaches(models).empty(), "Repeated refresh failed.");
        std::cout << checks << " checks passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
