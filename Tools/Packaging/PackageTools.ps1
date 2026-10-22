#requires -Version 5.1
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
# SDK files can exceed MAX_PATH after staging and ZIP extraction. These switches
# affect this .NET Framework process only; no Windows policy/registry changes.
[AppContext]::SetSwitch('Switch.System.IO.UseLegacyPathHandling', $false)
[AppContext]::SetSwitch('Switch.System.IO.BlockLongPaths', $false)
Add-Type -AssemblyName System.IO.Compression.FileSystem

# .NET Framework (Windows PowerShell 5.1) has neither ArgumentList nor GetRelativePath.
function ConvertTo-PackageArgument([AllowEmptyString()][string]$Value) {
    '"' + ([regex]::Replace([regex]::Replace($Value, '(\\*)"', '$1$1\"'), '(\\+)$', '$1$1')) + '"'
}

function Get-PackageRelativePath([string]$Directory, [string]$Path) {
    $base = [Uri]([IO.Path]::GetFullPath($Directory).TrimEnd('\', '/') + [IO.Path]::DirectorySeparatorChar)
    $relative = $base.MakeRelativeUri([Uri][IO.Path]::GetFullPath($Path))
    if ($relative.IsAbsoluteUri) { throw "Path is on another volume: $Path" }
    [Uri]::UnescapeDataString($relative.ToString()).Replace('/', [IO.Path]::DirectorySeparatorChar)
}

function Assert-PackageVersion([string]$Version) {
    if ($Version.Length -gt 80 -or $Version -cnotmatch '^\d+\.\d+\.\d+(?:-[0-9A-Za-z]+(?:[.-][0-9A-Za-z]+)*)?$') {
        throw 'Use a version such as 0.1.0 or 0.1.0-dev.1 (at most 80 characters).'
    }
}

function Invoke-PackageTool {
    param([string]$Executable, [string[]]$Arguments, [string]$WorkingDirectory,
          [string]$LogPath, [int]$TimeoutSeconds = 1800, [hashtable]$Environment = @{})
    $info = [Diagnostics.ProcessStartInfo]::new()
    $info.FileName = $Executable
    $info.WorkingDirectory = $WorkingDirectory
    $info.UseShellExecute = $false
    $info.CreateNoWindow = $true
    $info.RedirectStandardOutput = $true
    $info.RedirectStandardError = $true
    $info.StandardOutputEncoding = [Text.Encoding]::UTF8
    $info.StandardErrorEncoding = [Text.Encoding]::UTF8
    foreach ($key in $Environment.Keys) { $info.EnvironmentVariables[$key] = $Environment[$key] }
    $info.Arguments = ($Arguments | ForEach-Object { ConvertTo-PackageArgument $_ }) -join ' '
    $process = [Diagnostics.Process]::Start($info)
    try {
        $stdout = $process.StandardOutput.ReadToEndAsync()
        $stderr = $process.StandardError.ReadToEndAsync()
        $timedOut = -not $process.WaitForExit($TimeoutSeconds * 1000)
        if ($timedOut) {
            # taskkill is included in Windows and terminates descendants on .NET Framework too.
            try { & "$env:SystemRoot/System32/taskkill.exe" /PID $process.Id /T /F 2>&1 | Out-Null }
            catch { Write-Warning "Could not terminate process tree: $_" }
            if (-not $process.HasExited) { $process.Kill() }
            $process.WaitForExit()
        }
        $output = $stdout.GetAwaiter().GetResult()
        $errors = $stderr.GetAwaiter().GetResult()
        [IO.File]::WriteAllText($LogPath, $output + "`n" + $errors, [Text.UTF8Encoding]::new($false))
        if ($timedOut) { throw "Timed out: $Executable (see $LogPath)" }
        if ($process.ExitCode -ne 0) {
            $tail = (($output + "`n" + $errors) -split '\r?\n' | Select-Object -Last 25) -join "`n"
            throw "Command failed ($($process.ExitCode)): $Executable`n$tail`nFull log: $LogPath"
        }
        return $output
    } finally { $process.Dispose() }
}

function Copy-PackageFile([string]$Source, [string]$Destination) {
    if (-not (Test-Path -LiteralPath $Source -PathType Leaf) -or (Get-Item -LiteralPath $Source).Length -eq 0) {
        throw "Required nonempty file is missing: $Source"
    }
    if ((Get-Item -LiteralPath $Source).Attributes -band [IO.FileAttributes]::ReparsePoint) {
        throw "Package inputs must be regular files: $Source"
    }
    $null = [IO.Directory]::CreateDirectory((Split-Path $Destination -Parent))
    Copy-Item -LiteralPath $Source -Destination $Destination -ErrorAction Stop
}

function Get-PackageHashes([string]$Directory) {
    $hashes = [ordered]@{}
    foreach ($file in Get-ChildItem -LiteralPath $Directory -File -Recurse | Sort-Object FullName) {
        $relative = (Get-PackageRelativePath $Directory $file.FullName).Replace('\', '/')
        $hashes[$relative] = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    return $hashes
}

function Write-PackageJson([string]$Path, $Value) {
    [IO.File]::WriteAllText($Path, ($Value | ConvertTo-Json -Depth 10) + "`n", [Text.UTF8Encoding]::new($false))
}

function Copy-EditorPayload {
    param([string]$RepoRoot, [string]$NativeDirectory, [string]$ManagedDirectory, [string]$Destination)
    if (Test-Path -LiteralPath $Destination) { throw "Package staging destination already exists: $Destination" }
    $null = [IO.Directory]::CreateDirectory($Destination)
    Copy-PackageFile "$NativeDirectory/CurryEngine.exe" "$Destination/CurryEngine.exe"
    foreach ($name in 'Core', 'Runtime', 'API') {
        Copy-PackageFile "$ManagedDirectory/bin/CurryEngine.API/release/CurryEngine.$name.dll" "$Destination/CurryEngine.$name.dll"
    }
    foreach ($name in 'deps', 'runtimeconfig') {
        Copy-PackageFile "$ManagedDirectory/bin/CurryEngine.Runtime/release/CurryEngine.Runtime.$name.json" "$Destination/CurryEngine.Runtime.$name.json"
    }
    Copy-PackageFile "$RepoRoot/x64/Release/assimp.dll" "$Destination/assimp.dll"
    Copy-PackageFile "$RepoRoot/External/DotNetHost/9.0.10/runtimes/win-x64/native/nethost.dll" "$Destination/nethost.dll"
    foreach ($font in 'NotoSansJP-Medium.ttf', 'NotoSansSC-Medium.ttf') {
        Copy-PackageFile "$RepoRoot/Data/Fonts/$font" "$Destination/Resources/Data/Fonts/$font"
    }
    foreach ($relative in 'Data/Icon', 'Data/Default') {
        $files = @(Get-ChildItem -LiteralPath "$RepoRoot/$relative" -File -Filter '*.png')
        if ($files.Count -eq 0) { throw "No PNG resources: $relative" }
        foreach ($file in $files) { Copy-PackageFile $file.FullName "$Destination/Resources/$relative/$($file.Name)" }
    }
    foreach ($required in 'Data/Icon/editorIcons.png', 'Data/Default/button.png') {
        if (-not (Test-Path -LiteralPath "$Destination/Resources/$required" -PathType Leaf)) { throw "Missing resource: $required" }
    }
    $sources = @(Get-ChildItem -LiteralPath "$RepoRoot/Shader" -File | Where-Object Extension -In '.hlsl', '.hlsli')
    if ($sources.Count -eq 0) { throw 'No shader sources.' }
    foreach ($file in $sources) { Copy-PackageFile $file.FullName "$Destination/Resources/Shader/$($file.Name)" }
    # Only current project shader outputs are accepted; never collect stale Assets/Shaders or Data/Shaders.
    [xml]$project = Get-Content -LiteralPath "$RepoRoot/CurryEngine.vcxproj" -Raw
    $shaderItems = @($project.SelectNodes("//*[local-name()='FxCompile'][@Include]"))
    if ($shaderItems.Count -eq 0) { throw 'No shaders are declared in the native project.' }
    foreach ($item in $shaderItems) {
        $name = [IO.Path]::GetFileNameWithoutExtension($item.Include) + '.cso'
        Copy-PackageFile "$NativeDirectory/Shaders/$name" "$Destination/Resources/Assets/Shaders/$name"
    }
    foreach ($name in 'geometric_primitive_vs.cso', 'geometric_primitive_ps.cso') {
        Copy-PackageFile "$NativeDirectory/Shaders/$name" "$Destination/Resources/Data/Shaders/$name"
    }
    Copy-Item -LiteralPath "$RepoRoot/Templates" -Destination "$Destination/Templates" -Recurse -Force -ErrorAction Stop
}

function Copy-PortableDependencies([string]$Destination, [string]$VisualStudio) {
    $dotnetRoot = Split-Path (Get-Command dotnet -ErrorAction Stop).Source -Parent
    $sdk = Get-ChildItem "$dotnetRoot/sdk" -Directory | Where-Object Name -Match '^9\.0\.\d+$' | Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1
    if (-not $sdk) { throw '.NET 9 SDK is required on the packaging PC.' }
    $runtimeVersion = (Get-Content "$($sdk.FullName)/dotnet.runtimeconfig.json" -Raw | ConvertFrom-Json).runtimeOptions.framework.version
    $local = Join-Path $Destination 'DotNet'
    foreach ($name in 'dotnet.exe', 'LICENSE.txt', 'ThirdPartyNotices.txt') { Copy-PackageFile "$dotnetRoot/$name" "$local/$name" }
    # Copy the SDK and matching runtime/reference packs, never machine-installed workloads.
    foreach ($relative in @("sdk/$($sdk.Name)", "host/fxr/$runtimeVersion", "shared/Microsoft.NETCore.App/$runtimeVersion",
        "packs/Microsoft.NETCore.App.Ref/$runtimeVersion", "packs/Microsoft.AspNetCore.App.Ref/$runtimeVersion",
        "packs/Microsoft.WindowsDesktop.App.Ref/$runtimeVersion")) {
        if (-not (Test-Path -LiteralPath "$dotnetRoot/$relative" -PathType Container)) { throw "Missing portable .NET component: $relative" }
        $parent = Split-Path "$local/$relative" -Parent
        $null = [IO.Directory]::CreateDirectory($parent)
        Copy-Item -LiteralPath "$dotnetRoot/$relative" -Destination $parent -Recurse
    }
    $crt = Get-ChildItem "$VisualStudio/VC/Redist/MSVC/*/x64/Microsoft.VC143.CRT" -Directory | Sort-Object FullName -Descending | Select-Object -First 1
    if (-not $crt) { throw 'Redistributable x64 Visual C++ runtime was not found.' }
    foreach ($file in Get-ChildItem -LiteralPath $crt.FullName -Filter '*.dll') {
        Copy-PackageFile $file.FullName "$Destination/$($file.Name)"
        Copy-PackageFile $file.FullName "$local/$($file.Name)"
    }
}

function Test-NativePackageDependencies {
    param([string]$Directory, [string]$Dumpbin, [string]$LogDirectory)
    $dependencies = [ordered]@{}
    foreach ($file in Get-ChildItem -LiteralPath $Directory -File | Where-Object Extension -In '.exe', '.dll' | Where-Object Name -NotLike 'CurryEngine.*.dll') {
        $binary = $file.Name
        $dump = Invoke-PackageTool $Dumpbin @('/dependents', "$Directory/$binary") $Directory "$LogDirectory/$binary.dependencies.log" 60
        $names = @([regex]::Matches($dump, '(?im)^\s+([a-z0-9_.-]+\.dll)\s*$') | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique)
        if ($names.Count -eq 0) { throw "No dependency information for $binary" }
        foreach ($name in $names) {
            if ($name -match '^(MSVCP\d.*D|VCRUNTIME\d.*D|VCOMP\d.*D|ucrtbased)\.dll$') { throw "Non-redistributable Debug dependency: $binary -> $name" }
            if (Test-Path -LiteralPath "$Directory/$name" -PathType Leaf) { continue }
            if ($name -match '^(MSVCP|VCRUNTIME|VCOMP|CONCRT|MSVCR)\d') { throw "Missing app-local runtime: $binary -> $name" }
            if ($name -match '^(api-ms-|ext-ms-)' -or
                (Test-Path -LiteralPath "$env:SystemRoot/System32/$name" -PathType Leaf)) { continue }
            throw "Unpackaged native dependency: $binary -> $name"
        }
        $dependencies[$binary] = $names
    }
    return $dependencies
}

function Test-EditorPackageCommands {
    param([string]$PackageDirectory, [string]$TestDirectory, [string]$Version)
    $null = [IO.Directory]::CreateDirectory($TestDirectory)
    $EngineDirectory = Join-Path $PackageDirectory 'Engine'
    $before = Get-PackageHashes $EngineDirectory | ConvertTo-Json -Compress
    $projectRoot = Join-Path $PackageDirectory 'CurryEngineProject'
    $exe = Join-Path $EngineDirectory 'CurryEngine.exe'
    $created = Invoke-PackageTool $exe @('--create-project', $projectRoot, '--name', '新規作品 & space') $TestDirectory "$TestDirectory/create.log" 120 | ConvertFrom-Json
    if (-not $created.created -or [IO.Path]::GetFullPath($created.projectFile) -ne "$projectRoot/Project.curryproject".Replace('/', '\')) {
        throw 'Unexpected --create-project response.'
    }
    foreach ($relative in 'Project.sln', 'Assembly-CSharp.csproj', 'imgui.ini', 'Assets/Scenes/Main.scene', 'Assets/Scripts/HelloCurry.cs', 'ProjectSettings/settings.json') {
        if (-not (Test-Path -LiteralPath "$projectRoot/$relative" -PathType Leaf)) { throw "Creation missed: $relative" }
    }
    if ((Get-FileHash -LiteralPath "$projectRoot/imgui.ini" -Algorithm SHA256).Hash -cne
        (Get-FileHash -LiteralPath "$EngineDirectory/Templates/Empty/imgui.ini" -Algorithm SHA256).Hash) {
        throw 'Template imgui.ini changed during project creation.'
    }
    $valid = Invoke-PackageTool $exe @('--validate-project', $created.projectFile) $TestDirectory "$TestDirectory/validate.log" 120 | ConvertFrom-Json
    if (-not $valid.valid -or $valid.engineVersion -cne $Version) { throw 'Project validation/version mismatch.' }
    $generated = Invoke-PackageTool $exe @('--generate-solution', $created.projectFile) $TestDirectory "$TestDirectory/generate.log" 120 | ConvertFrom-Json
    if (-not $generated.generated) { throw 'Solution generation failed.' }
    # Pin the same stable SDK selection when the chosen output directory is outside the repository.
    Write-PackageJson "$projectRoot/global.json" @{ sdk = @{ version = '9.0.100'; rollForward = 'latestFeature'; allowPrerelease = $false } }
    # No installed SDK on PATH, no global package cache and no package feed are used.
    [IO.File]::WriteAllText("$projectRoot/NuGet.Config", '<configuration><packageSources><clear /></packageSources></configuration>')
    $environment = @{
        PATH = "$env:SystemRoot/System32;$env:SystemRoot"
        DOTNET_ROOT = "$EngineDirectory/DotNet"; DOTNET_ROOT_X64 = "$EngineDirectory/DotNet"
        DOTNET_MULTILEVEL_LOOKUP = '0'; DOTNET_CLI_TELEMETRY_OPTOUT = '1'
        DOTNET_GENERATE_ASPNET_CERTIFICATE = 'false'; DOTNET_ADD_GLOBAL_TOOLS_TO_PATH = 'false'
        DOTNET_CLI_HOME = "$projectRoot/Library/DotNetHome"; NUGET_PACKAGES = "$projectRoot/Library/NuGetPackages"
        MSBuildEnableWorkloadResolver = 'false'
    }
    $null = Invoke-PackageTool "$EngineDirectory/DotNet/dotnet.exe" @('build', "$projectRoot/Project.sln", '-c', 'Release', '--nologo', '-p:NuGetAudit=false') $projectRoot "$TestDirectory/scripts-build.log" 300 $environment
    if (-not (Test-Path -LiteralPath "$projectRoot/Library/ScriptAssemblies/Assembly-CSharp.dll")) { throw 'Script assembly was not produced.' }
    if (($before) -cne (Get-PackageHashes $EngineDirectory | ConvertTo-Json -Compress)) { throw 'Smoke test changed shared engine files.' }
}

function Publish-EditorPackage {
    param([string]$PackageDirectory, [string]$OutputDirectory, [string]$Version, [string]$StagingDirectory)
    Assert-PackageVersion $Version
    $name = "CurryEngine-$Version-win-x64"
    $target = Join-Path $OutputDirectory $name
    $zipTarget = "$target.zip"
    if ((Test-Path -LiteralPath $target) -or (Test-Path -LiteralPath $zipTarget)) { throw "Version output already exists: $target" }
    $hashes = Get-PackageHashes $PackageDirectory
    Write-PackageJson "$PackageDirectory/package-files.json" $hashes
    $zip = Join-Path $StagingDirectory 'package.zip'
    [IO.Compression.ZipFile]::CreateFromDirectory($PackageDirectory, $zip, [IO.Compression.CompressionLevel]::Optimal, $false)
    $extracted = Join-Path $StagingDirectory 'Extracted Engine 日本語 & space'
    [IO.Compression.ZipFile]::ExtractToDirectory($zip, $extracted)
    foreach ($entry in $hashes.GetEnumerator()) {
        $hash = (Get-FileHash -LiteralPath (Join-Path $extracted $entry.Key) -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($hash -cne $entry.Value) { throw "ZIP content mismatch: $($entry.Key)" }
    }
    Test-EditorPackageCommands $extracted (Join-Path $StagingDirectory 'Smoke') $Version
    # Sources and destinations are resolved descendants of the selected output root; never overwrite/delete a release.
    $outputRoot = [IO.Path]::GetFullPath($OutputDirectory).TrimEnd('\') + '\'
    foreach ($path in @($PackageDirectory, $StagingDirectory, $target, $zipTarget)) {
        if (-not [IO.Path]::GetFullPath($path).StartsWith($outputRoot, [StringComparison]::OrdinalIgnoreCase)) {
            throw "Publish path is outside output root: $path"
        }
    }
    [IO.Directory]::Move($PackageDirectory, $target)
    [IO.File]::Move($zip, $zipTarget)
    $zipHash = (Get-FileHash -LiteralPath $zipTarget -Algorithm SHA256).Hash.ToLowerInvariant()
    [IO.File]::WriteAllText("$zipTarget.sha256", "$zipHash  $name.zip`n")
    return [pscustomobject]@{ PackageDirectory = $target; Zip = $zipTarget; Sha256 = $zipHash }
}
