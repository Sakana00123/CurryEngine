param([string[]]$Only)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$msbuild = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild/Current/Bin/MSBuild.exe'
if (-not $msbuild) { throw 'MSBuild was not found.' }
$files = @(
    'Engine\main.cpp', 'Engine\Core\ProjectContext.cpp', 'Engine\Core\Startup.cpp',
    'Engine\Automation\SolutionGenerator.cpp', 'Engine\ProjectSettings.cpp',
    'Engine\Scripting\ScriptSystem.cpp', 'Engine\Scripting\ScriptHost.cpp', 'Engine\Scripting\ScriptWatcher.cpp',
    'Engine\Resources\Texture.cpp', 'Engine\Resources\Shader.cpp', 'Engine\Resources\ResourceManager.cpp',
    'Engine\Core\framework.cpp', 'Engine\Core\ScriptComponent.cpp', 'Engine\Editor\AssetBrowser.cpp', 'Engine\Rendering\Material.cpp'
)
Push-Location $repoRoot
if ($Only) { $files = $Only }
try {
    foreach ($file in $files) {
        & $msbuild CurryEngine.vcxproj /t:ClCompile /p:Configuration=Debug /p:Platform=x64 "/p:SelectedFiles=$file" "/p:SolutionDir=$repoRoot\" '/p:IntDir=Library\Tests\ProjectRuntime\native\' '/p:OutDir=Library\Tests\ProjectRuntime\bin\' /nologo /v:minimal
        if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $file" }
    }
}
finally { Pop-Location }
