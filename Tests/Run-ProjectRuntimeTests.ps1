$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$runId = [guid]::NewGuid().ToString('N')
$runRoot = Join-Path $repoRoot "Library/Tests/ProjectRuntime/$runId"
$utf8 = [Text.UTF8Encoding]::new($false)
function Write-Utf8($path, $text) {
    $null = New-Item -ItemType Directory -Force -Path (Split-Path $path -Parent)
    [IO.File]::WriteAllText($path, $text, $utf8)
}
function Expect-Rejection($exe, $mode, $file) {
    $start = [Diagnostics.ProcessStartInfo]::new($exe)
    $start.UseShellExecute = $false
    $start.CreateNoWindow = $true
    $start.RedirectStandardOutput = $true
    $start.RedirectStandardError = $true
    $start.ArgumentList.Add($mode)
    $start.ArgumentList.Add($file)
    $process = [Diagnostics.Process]::Start($start)
    $errorText = $process.StandardError.ReadToEnd()
    $outputText = $process.StandardOutput.ReadToEnd()
    $process.WaitForExit()
    if ($process.ExitCode -ne 1 -or $outputText -ne '' -or ($errorText | ConvertFrom-Json).valid) {
        throw 'Invalid startup was not rejected.'
    }
    $process.Dispose()
}
Push-Location $repoRoot
try {
    & dotnet build CurryEngine.API/CurryEngine.API.csproj -c Release --artifacts-path "$runRoot/managed" -p:SkipEngineCopy=true --nologo -v quiet
    if ($LASTEXITCODE -ne 0) { throw 'Managed engine build failed.' }
    $testOutput = @()
    & "$PSScriptRoot/Run-ProjectContextTests.ps1" | Tee-Object -Variable testOutput
    $probeRoot = ($testOutput | Where-Object { $_ -like 'Test outputs: *' } | Select-Object -Last 1).Substring(14)
    $engineRoot = Join-Path $runRoot 'Engine 日本語 & space'
    $null = New-Item -ItemType Directory -Path $engineRoot
    Copy-Item -LiteralPath "$probeRoot/StartupProbe.exe", "$probeRoot/engine.json" -Destination $engineRoot
    $probe = Join-Path $engineRoot 'StartupProbe.exe'
    $managedBin = Join-Path $runRoot 'managed/bin/CurryEngine.API/release'
    foreach ($name in @('Core', 'Runtime', 'API')) {
        Copy-Item -LiteralPath "$managedBin/CurryEngine.$name.dll" -Destination $engineRoot
    }
    foreach ($dir in @('Data/Icon','Data/Fonts','Shader','Assets/Shaders')) {
        $null = New-Item -ItemType Directory -Force -Path "$engineRoot/Resources/$dir"
    }
    Write-Utf8 "$engineRoot/Resources/Shader/test.hlsl" '// engine-owned test fixture'
    $before = @(Get-ChildItem $engineRoot -Recurse -File | Get-FileHash | ForEach-Object { $_.Path + ':' + $_.Hash })
    $invalidRoot = "$runRoot/Invalid"
    Write-Utf8 "$invalidRoot/Game.curryproject" '{"formatVersion":1,"name":"Game","engineVersion":"test-1"}'
    Expect-Rejection $probe '--project' "$invalidRoot/Game.curryproject"
    if (Test-Path "$invalidRoot/Library") { throw 'Rejected startup created output.' }
    Write-Utf8 "$invalidRoot/Assembly-CSharp.csproj" '<Project><!-- user-owned --></Project>'
    Expect-Rejection $probe '--generate-solution' "$invalidRoot/Game.curryproject"
    if ((Get-Content -Raw "$invalidRoot/Assembly-CSharp.csproj") -ne '<Project><!-- user-owned --></Project>') {
        throw 'A user-owned project file was overwritten.'
    }
    $projects = @("$runRoot/作品 A & space", "$runRoot/作品 B")
    foreach ($root in $projects) {
        $null = New-Item -ItemType Directory -Force -Path "$root/Assets/Scenes", "$root/ProjectSettings"
        Write-Utf8 "$root/Game.curryproject" '{"formatVersion":1,"name":"Game","engineVersion":"test-1"}'
        Write-Utf8 "$root/Assets/Scripts/ProjectProbe.cs" 'using CurryEngine; public class ProjectProbe : Behaviour { public static int Value() => 1; }'
        & $probe --generate-solution "$root/Game.curryproject"
        if ($LASTEXITCODE -ne 0) { throw 'Solution generation failed.' }
        [xml]$csproj = Get-Content -Raw "$root/Assembly-CSharp.csproj"
        if ($csproj.SelectNodes('//ProjectReference').Count -ne 0) { throw 'Generated project depends on engine sources.' }
        $openOutput = & $probe --project "$root/Game.curryproject"
        if ($LASTEXITCODE -ne 0) { throw 'Project startup failed.' }
        $opened = $openOutput | ConvertFrom-Json
        if (-not $opened.opened -or [IO.Path]::GetFullPath($opened.cwd) -ne [IO.Path]::GetFullPath($root)) { throw 'Wrong working directory.' }
        if ([IO.Path]::GetFullPath($opened.shader) -ne [IO.Path]::GetFullPath("$engineRoot/Resources/Shader/test.hlsl")) { throw 'Wrong shared resource path.' }
        if ([IO.Path]::GetFullPath($opened.cache) -ne [IO.Path]::GetFullPath("$root/Library/ShaderCache")) { throw 'Shader cache is not project-local.' }
        if ($opened.asset -ne 'Assets/Shaders/custom.hlsl') { throw 'Project shader redirected into shared resources.' }
    }
    & dotnet build "$($projects[1])/Game.sln" -c Debug --nologo -v quiet
    if ($LASTEXITCODE -ne 0) { throw 'Generated solution did not build.' }
    foreach ($root in $projects) {
        if (Test-Path "$root/Library/ScriptAssemblies/CurryEngine.Core.dll") { throw 'Shared engine DLL was copied into the project.' }
    }
    & dotnet build "$PSScriptRoot/ManagedReloadProbe.csproj" --artifacts-path "$runRoot/probe" "-p:EngineTestBin=$managedBin" --nologo -v quiet
    if ($LASTEXITCODE -ne 0) { throw 'Managed integration probe build failed.' }
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
    $vsRoot = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    $nativeBuild = @"
@echo off
call "$vsRoot/VC/Auxiliary/Build/vcvars64.bat" > nul
cl /nologo /LD /EHsc "$PSScriptRoot/NativeRuntimeStub.cpp" /Fe:NativeRuntimeStub.dll
exit /b %errorlevel%
"@
    Write-Utf8 "$runRoot/native.cmd" $nativeBuild
    Push-Location $runRoot
    try {
        & $env:ComSpec /d /c "`"$runRoot/native.cmd`""
        if ($LASTEXITCODE -ne 0) { throw 'Native integration stub build failed.' }
    } finally { Pop-Location }
    & "$runRoot/probe/bin/ManagedReloadProbe/debug/ManagedReloadProbe.exe" $projects[0] $projects[1] "$runRoot/NativeRuntimeStub.dll"
    if ($LASTEXITCODE -ne 0) { throw 'Managed integration failed.' }
    $after = @(Get-ChildItem $engineRoot -Recurse -File | Get-FileHash | ForEach-Object { $_.Path + ':' + $_.Hash })
    if (Compare-Object $before $after) { throw 'Engine files changed during project build/reload.' }
    Write-Output 'Project runtime integration passed; engine files unchanged.'
    Write-Output "Runtime test outputs: $runRoot"
} finally { Pop-Location }
