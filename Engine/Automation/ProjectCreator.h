#pragma once
#include "Engine/Core/ProjectContext.h"

namespace CurryEngine
{
    std::filesystem::path CreateProject(const StartupRequest& request,
        const std::filesystem::path& executable, const std::filesystem::path& launchDirectory);
}
