#pragma once
#include "Engine/Core/ProjectContext.h"

class SolutionGenerator
{
public:
    // Throws on missing references, user-owned files or write errors.
    static void Generate(const CurryEngine::ProjectContext& context);
};
