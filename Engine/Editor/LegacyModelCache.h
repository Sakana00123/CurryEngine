#pragma once
#include <filesystem>
#include <vector>

namespace CurryEngine
{
    // Models is optional in a new project. Never create it merely to clean old caches.
    // Do not follow links into another project's files.
    inline std::vector<std::filesystem::path> RemoveLegacyModelCaches(const std::filesystem::path& modelsRoot)
    {
        namespace fs = std::filesystem;
        std::vector<fs::path> removed;
        if (!fs::is_directory(modelsRoot) || fs::is_symlink(modelsRoot)) return removed;
        for (const auto& entry : fs::recursive_directory_iterator(modelsRoot))
        {
            if (entry.is_symlink() || !entry.is_regular_file()) continue;
            const auto extension = entry.path().extension();
            if (extension == ".cereal" || extension == ".batchCereal")
            {
                if (fs::remove(entry.path())) removed.push_back(entry.path());
            }
        }
        return removed;
    }
}
