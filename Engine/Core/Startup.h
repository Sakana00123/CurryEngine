#pragma once
#include "ProjectContext.h"
#include <optional>

namespace CurryEngine
{
    void NotifyProjectReady(const ProjectContext& context);
    // Only interactive engine startup writes these diagnostics, never the headless commands.
    void BeginStartupDiagnostics(const ProjectContext& context) noexcept;
    void RecordStartupStage(const char* stage) noexcept;
    void ReportRuntimeFailure(const char* error) noexcept;
    // Uses the full Unicode Windows command line. A returned exit code means no engine startup.
    std::optional<int> PrepareStartup(ProjectContext& context);
}
