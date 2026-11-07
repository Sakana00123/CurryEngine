$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
if (-not (Test-Path -LiteralPath $vswhere)) { throw 'Visual Studio Installer / vswhere was not found.' }
$vsRoot = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsRoot) { throw 'Visual Studio C++ tools were not found.' }
$buildDir = Join-Path $repoRoot ('Library/Tests/ProjectContext/' + [guid]::NewGuid().ToString('N'))
$null = New-Item -ItemType Directory -Path $buildDir
$buildCmd = Join-Path $buildDir 'build.cmd'
$vcvars = Join-Path $vsRoot 'VC/Auxiliary/Build/vcvars64.bat'
$commands = @"
@echo off
call "$vcvars" > nul
if errorlevel 1 exit /b 1
cl /nologo /std:c++20 /utf-8 /EHsc /W4 /I "$repoRoot" "$repoRoot/Tests/LegacyModelCacheTests.cpp" /Fe:LegacyModelCacheTests.exe
if errorlevel 1 exit /b 1
cl /nologo /std:c++20 /utf-8 /EHsc /W4 /I "$repoRoot" "$repoRoot/Engine/Core/ProjectContext.cpp" "$repoRoot/Tests/ProjectContextTests.cpp" /Fe:ProjectContextTests.exe
if errorlevel 1 exit /b 1
cl /nologo /std:c++20 /utf-8 /EHsc /W4 /MTd /I "$repoRoot" "$repoRoot/Engine/Core/ProjectContext.cpp" "$repoRoot/Engine/Core/Startup.cpp" "$repoRoot/Engine/Automation/SolutionGenerator.cpp" "$repoRoot/Engine/Automation/ProjectCreator.cpp" "$repoRoot/Tests/StartupProbe.cpp" /Fe:StartupProbe.exe /link user32.lib
exit /b %errorlevel%
"@
[IO.File]::WriteAllText($buildCmd, $commands, [Text.Encoding]::Default)
Push-Location $buildDir
try {
    & $env:ComSpec /d /c "`"$buildCmd`""
    if ($LASTEXITCODE -ne 0) { throw 'C++ test build failed.' }
    & (Join-Path $buildDir 'LegacyModelCacheTests.exe') (Join-Path $buildDir 'cache-fixtures')
    if ($LASTEXITCODE -ne 0) { throw 'Legacy model cache tests failed.' }
    & (Join-Path $buildDir 'ProjectContextTests.exe') (Join-Path $buildDir 'fixtures')
    if ($LASTEXITCODE -ne 0) { throw 'ProjectContext tests failed.' }

    # Use a sibling project so the installation and project directories are disjoint.
    $projectDir = Join-Path (Split-Path $buildDir -Parent) ((Split-Path $buildDir -Leaf) + '-作品 & space')
    $null = New-Item -ItemType Directory -Path $projectDir
    $projectFile = Join-Path $projectDir 'My Game.curryproject'
    $utf8 = [Text.UTF8Encoding]::new($false)
    [IO.File]::WriteAllText($projectFile, '{"formatVersion":1,"name":"テスト作品","engineVersion":"test-1"}', $utf8)
    [IO.File]::WriteAllText((Join-Path $buildDir 'engine.json'), '{"formatVersion":1,"version":"test-1"}', $utf8)
    $probe = Join-Path $buildDir 'StartupProbe.exe'
    $resultFile = Join-Path $buildDir 'stdout.json'
    $errorFile = Join-Path $buildDir 'stderr.json'
    & $probe --validate-project $projectFile > $resultFile 2> $errorFile
    if ($LASTEXITCODE -ne 0) { throw 'Startup integration failed.' }
    $result = Get-Content -Raw -LiteralPath $resultFile | ConvertFrom-Json
    if (-not $result.valid -or $result.name -ne 'テスト作品') { throw 'Invalid UTF-8 validation result.' }
    if ([IO.Path]::GetFullPath($result.projectRoot) -ne [IO.Path]::GetFullPath($projectDir)) { throw 'Wrong project root in CLI output.' }
    $legacy = & $probe
    if ($LASTEXITCODE -ne 0 -or $legacy -ne 'LEGACY_STARTUP') { throw 'Legacy startup changed.' }

    $null = New-Item -ItemType Directory -Path (Join-Path $buildDir 'Templates')
    Copy-Item -LiteralPath (Join-Path $repoRoot 'Templates/Empty') -Destination (Join-Path $buildDir 'Templates/Empty') -Recurse
    foreach ($name in 'Core', 'Runtime', 'API') {
        [IO.File]::WriteAllBytes((Join-Path $buildDir "CurryEngine.$name.dll"), [byte[]]@())
    }
    $createdDir = Join-Path (Split-Path $buildDir -Parent) ((Split-Path $buildDir -Leaf) + '-created')
    & $probe --create-project $createdDir --name '新規作品' > $resultFile 2> $errorFile
    if ($LASTEXITCODE -ne 0) { throw 'Project creation integration failed.' }
    $created = Get-Content -Raw -LiteralPath $resultFile | ConvertFrom-Json
    if (-not $created.created -or -not (Test-Path -LiteralPath $created.projectFile -PathType Leaf)) { throw 'Project creation returned an invalid path.' }
    $templateIni = Join-Path $repoRoot 'Templates/Empty/imgui.ini'
    $createdIni = Join-Path $createdDir 'imgui.ini'
    if (-not (Test-Path -LiteralPath $createdIni -PathType Leaf) -or
        (Get-FileHash -LiteralPath $createdIni -Algorithm SHA256).Hash -cne (Get-FileHash -LiteralPath $templateIni -Algorithm SHA256).Hash) {
        throw 'Project creation did not copy template imgui.ini.'
    }

    # Capture rejection through Process, avoiding PowerShell treating stderr as a terminating error.
    $start = [Diagnostics.ProcessStartInfo]::new()
    $start.FileName = $probe
    $start.Arguments = '--validate-project "missing.curryproject"'
    $start.UseShellExecute = $false
    $start.CreateNoWindow = $true
    $start.RedirectStandardOutput = $true
    $start.RedirectStandardError = $true
    $process = [Diagnostics.Process]::Start($start)
    $errorText = $process.StandardError.ReadToEnd()
    $outputText = $process.StandardOutput.ReadToEnd()
    $process.WaitForExit()
    if ($process.ExitCode -ne 1 -or $outputText -ne '' -or ($errorText | ConvertFrom-Json).valid) { throw 'Invalid-project startup was not rejected.' }
    $process.Dispose()
    Write-Output 'Startup integration passed (Unicode/quoted paths, JSON output, project creation, legacy startup, invalid project).'
    Write-Output "Test outputs: $buildDir"
}
finally { Pop-Location }
