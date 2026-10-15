$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$preview = Join-Path $repoRoot 'Library/HubPreview'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$msbuild = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild/Current/Bin/MSBuild.exe'
if (-not $msbuild) { throw 'MSBuild was not found.' }
Push-Location $repoRoot
try {
    & $msbuild CurryEngine.vcxproj /t:Build /p:PreBuildEventUseInBuild=false /p:PostBuildEventUseInBuild=false /p:BuildProjectReferences=false /p:Configuration=Debug /p:Platform=x64 /p:CL_MPCount=2 "/p:SolutionDir=$repoRoot\" '/p:IntDir=Library\HubPreview\native\' '/p:OutDir=Library\HubPreview\Engine\' /nologo /v:minimal
    if ($LASTEXITCODE -ne 0) { throw 'Native editor build failed.' }
    if (-not (Test-Path -LiteralPath "$preview/Engine/CurryEngine.exe")) { throw 'The editor executable was not produced.' }
    & dotnet build CurryEngine.API/CurryEngine.API.csproj -c Release --artifacts-path "$preview/managed" -p:SkipEngineCopy=true --nologo -v minimal
    if ($LASTEXITCODE -ne 0) { throw 'Managed engine build failed.' }
    $engine = Join-Path $preview 'Engine'
    $managedBin = Join-Path $preview 'managed/bin/CurryEngine.API/release'
    foreach ($name in @('Core','Runtime','API')) {
        Copy-Item -LiteralPath "$managedBin/CurryEngine.$name.dll" -Destination $engine
    }
    foreach ($name in @('deps','runtimeconfig')) {
        Copy-Item -LiteralPath "$preview/managed/bin/CurryEngine.Runtime/release/CurryEngine.Runtime.$name.json" -Destination $engine
    }
    foreach ($name in @('assimp.dll','nethost.dll')) {
        Copy-Item -LiteralPath "$repoRoot/x64/Debug/$name" -Destination $engine
    }
    foreach ($relative in @('Data/Icon','Data/Default','Data/Shaders','Shader','Assets/Shaders')) {
        $destination = Join-Path "$engine/Resources" $relative
        $null = New-Item -ItemType Directory -Force -Path $destination
        Copy-Item -Path "$repoRoot/$relative/*" -Destination $destination -Recurse -Force
    }
    $null = New-Item -ItemType Directory -Force -Path "$engine/Resources/Data/Fonts"
    foreach ($font in @('NotoSansJP-Medium.ttf','NotoSansSC-Medium.ttf')) {
        Copy-Item -LiteralPath "$repoRoot/Data/Fonts/$font" -Destination "$engine/Resources/Data/Fonts"
    }
    $null = New-Item -ItemType Directory -Force -Path "$engine/Templates"
    Copy-Item -LiteralPath "$repoRoot/Templates/Empty" -Destination "$engine/Templates" -Recurse -Force
    [IO.File]::WriteAllText("$engine/engine.json", '{"formatVersion":1,"version":"0.1.0-dev-hub","editor":true,"projectCommandsVersion":1}', [Text.UTF8Encoding]::new($false))
    Write-Output "Preview engine: $engine/CurryEngine.exe"
} finally { Pop-Location }
