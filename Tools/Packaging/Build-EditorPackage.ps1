#requires -Version 5.1
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Version,
    [string]$OutputDirectory,
    [ValidateRange(1, 16)][int]$BuildWorkers = 2
)

. "$PSScriptRoot/PackageTools.ps1"
Assert-PackageVersion $Version
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
if (-not $OutputDirectory) { $OutputDirectory = Join-Path $repoRoot 'BuildOutput/Engines' }
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
$target = Join-Path $OutputDirectory "CurryEngine-$Version-win-x64"
foreach ($path in @($target, "$target.zip", "$target.zip.sha256")) {
    if (Test-Path -LiteralPath $path) { throw "Output already exists; choose another version or output directory: $path" }
}
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
if (-not (Test-Path -LiteralPath $vswhere)) { throw 'Visual Studio Installer was not found.' }
$vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw 'Visual Studio C++ tools (v143) were not found.' }
$msbuild = Join-Path $vs 'MSBuild/Current/Bin/MSBuild.exe'
$dumpbin = Get-ChildItem -Path "$vs/VC/Tools/MSVC/*/bin/Hostx64/x64/dumpbin.exe" | Sort-Object FullName -Descending | Select-Object -First 1 -ExpandProperty FullName
$null = Get-Command dotnet -ErrorAction Stop

$runId = [Guid]::NewGuid().ToString('N')
$build = Join-Path $repoRoot "Library/Packaging/$runId"
$stage = Join-Path $OutputDirectory ".curry-package-$runId"
$engine = Join-Path $stage 'Engine'
$null = [IO.Directory]::CreateDirectory($build)
$null = [IO.Directory]::CreateDirectory($stage)
$native = Join-Path $build 'native'
$managed = Join-Path $build 'managed'
$generated = Join-Path $build 'Generated'
Write-Host "Build logs: $build"
Write-Host "Staging: $stage"
try {
    Write-Host '[1/6] Building the code generator...'
    $null = Invoke-PackageTool $msbuild @('Tools/MetaParser/MetaParser.vcxproj', '/t:Build', '/p:Configuration=Release', '/p:Platform=x64',
        "/p:OutDir=$build/parser/bin/", "/p:IntDir=$build/parser/obj/", '/nologo', '/v:minimal') $repoRoot "$build/parser-build.log"
    # The parser currently accepts narrow paths. Relative ASCII build paths avoid the output parent's encoding.
    $relativeGenerated = Get-PackageRelativePath $repoRoot $generated
    $null = Invoke-PackageTool "$build/parser/bin/MetaParser.exe" @($relativeGenerated, 'Engine') $repoRoot "$build/generate.log"
    if (-not (Test-Path -LiteralPath "$generated/ReflectionGenerated.h")) { throw 'Reflection code generation failed.' }

    Write-Host '[2/6] Building the portable editor and shaders (Release x64)...'
    $null = Invoke-PackageTool $msbuild @('External/DirectXTex/DirectXTex_Desktop_2022.vcxproj', '/t:Build', '/p:Configuration=Release', '/p:Platform=x64',
        "/p:OutDir=$build/directxtex/", "/p:IntDir=$build/directxtex-obj/", '/nologo', '/v:minimal') $repoRoot "$build/directxtex-build.log"
    $null = Invoke-PackageTool $msbuild @('CurryEngine.vcxproj', '/t:Build', '/p:Configuration=Release', '/p:Platform=x64',
        '/p:PreBuildEventUseInBuild=false', '/p:PostBuildEventUseInBuild=false', '/p:BuildProjectReferences=false',
        "/p:CL_MPCount=$BuildWorkers", "/p:SolutionDir=$repoRoot/", "/p:GeneratedRoot=$generated", "/p:PackageDirectXTex=$build/directxtex",
        "/p:ForceImportBeforeCppTargets=$PSScriptRoot/EditorPackage.targets", "/p:IntDir=$build/native-obj/", "/p:OutDir=$native/",
        '/nologo', '/v:minimal') $repoRoot "$build/native-build.log"

    Write-Host '[3/6] Building the managed engine...'
    $null = Invoke-PackageTool 'dotnet' @('build', 'CurryEngine.API/CurryEngine.API.csproj', '-c', 'Release',
        '--artifacts-path', $managed, '-p:SkipEngineCopy=true', "-p:GeneratedRoot=$generated", '--nologo') $repoRoot "$build/managed-build.log"

    Write-Host '[4/6] Collecting editor resources, templates and dependencies...'
    Copy-EditorPayload $repoRoot $native $managed $engine
    Copy-PortableDependencies $engine $vs
    $dependencies = Test-NativePackageDependencies $engine $dumpbin $build
    Write-PackageJson "$engine/engine.json" ([ordered]@{
        formatVersion = 1; version = $Version; editor = $true; projectCommandsVersion = 1
        platform = 'win-x64'; configuration = 'Release'; packageKind = 'portable-editor'
        requirements = @{
            os = 'Windows 10/11 x64'; graphics = 'Direct3D 11.1'; dotnetSdk = 'bundled'; visualStudioCppDebugRuntime = $false
        }
        nativeDependencies = $dependencies
    })
    [IO.File]::WriteAllText("$engine/version.txt", "$Version`n", [Text.UTF8Encoding]::new($false))
    Copy-PackageFile "$PSScriptRoot/PackageREADME.txt" "$engine/README.txt"

    Write-Host '[5/6] Creating ZIP, checking its contents and testing the extracted engine...'
    $result = Publish-EditorPackage $engine $OutputDirectory $Version $stage
    Write-Host '[6/6] Package completed.'
    Write-Host "Engine: $($result.EngineDirectory)/CurryEngine.exe"
    Write-Host "ZIP: $($result.Zip)"
    Write-Host "SHA256: $($result.Sha256)"
    Write-Host 'Validation: create project, validate project, generate solution, build C# scripts; shared engine unchanged.'
    Write-Host 'Rendering/editor play and clean-machine setup are not exercised by this command.'
    $result
} catch {
    Write-Host "Packaging failed. Build logs: $build"
    Write-Host "Diagnostic staging files are retained: $stage"
    throw
}
