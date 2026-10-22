#requires -Version 5.1
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
. "$repoRoot/Tools/Packaging/PackageTools.ps1"
$root = Join-Path $repoRoot ('Library/Tests/Packaging/' + [Guid]::NewGuid().ToString('N'))
$null = [IO.Directory]::CreateDirectory($root)
$script:checks = 0
function Check([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw $Message }
    $script:checks++
}
function Reject([scriptblock]$Action, [string]$Message) {
    $rejected = $false
    try { & $Action } catch { $rejected = $true }
    Check $rejected $Message
}
function Fixture([string]$Path, [string]$Text = 'fixture') {
    $null = [IO.Directory]::CreateDirectory((Split-Path $Path -Parent))
    [IO.File]::WriteAllText($Path, $Text)
}
foreach ($version in '0.1.0', '1.2.3-dev.1', '1.0.0-rc-2') { Assert-PackageVersion $version; $script:checks++ }
foreach ($version in '', '../escape', '1.0', '1.0.0/evil', '1.0.0;Remove-Item', '1.0.0 ', '1.0.0..') {
    Reject { Assert-PackageVersion $version } "Invalid version accepted: $version"
}
$fixtureRepo = Join-Path $root '入力 repo & 日本語'
$native = Join-Path $root 'native'
$managed = Join-Path $root 'managed'
foreach ($path in 'x64/Release/assimp.dll', 'External/DotNetHost/9.0.10/runtimes/win-x64/native/nethost.dll',
    'Data/Fonts/NotoSansJP-Medium.ttf', 'Data/Fonts/NotoSansSC-Medium.ttf', 'Data/Icon/editorIcons.png', 'Data/Default/button.png',
    'Shader/geometric_primitive_vs.hlsl', 'Shader/geometric_primitive_ps.hlsl', 'Shader/common.hlsli',
    'Templates/Empty/imgui.ini', 'Templates/Empty/Assets/Scenes/Main.scene', 'Templates/Empty/Assets/Scripts/HelloCurry.cs',
    'Templates/Empty/ProjectSettings/settings.json', 'Templates/Empty/ProjectSettings/EditorConfig.json',
    'Templates/Empty/ProjectSettings/Physics.json', 'Templates/Empty/ProjectSettings/Input.json',
    'Assets/Scenes/UserGame.scene', 'UserScripts/Secret.cs', 'Data/Icon/unwanted.log', 'Shader/unwanted.obj',
    'x64/Debug/Assembly-CSharp.dll', 'Templates/Empty/additional.log',
    'Templates/Other/Assets/Nested/example.txt', 'Templates/README.txt') { Fixture "$fixtureRepo/$path" }
Fixture "$fixtureRepo/Templates/Other/empty.txt" ''
$null = [IO.Directory]::CreateDirectory("$fixtureRepo/Templates/Other/EmptyDirectory")
Fixture "$fixtureRepo/CurryEngine.vcxproj" '<Project><ItemGroup><FxCompile Include="Shader\geometric_primitive_vs.hlsl"/><FxCompile Include="Shader\geometric_primitive_ps.hlsl"/></ItemGroup></Project>'
foreach ($path in 'CurryEngine.exe', 'Shaders/geometric_primitive_vs.cso', 'Shaders/geometric_primitive_ps.cso', 'Shaders/stale.cso', 'CurryEngine.pdb') { Fixture "$native/$path" }
foreach ($name in 'Core', 'Runtime', 'API') { Fixture "$managed/bin/CurryEngine.API/release/CurryEngine.$name.dll" }
foreach ($name in 'deps', 'runtimeconfig') { Fixture "$managed/bin/CurryEngine.Runtime/release/CurryEngine.Runtime.$name.json" }
$payload = Join-Path $root 'payload'
Copy-EditorPayload $fixtureRepo $native $managed $payload
foreach ($path in 'CurryEngine.exe', 'CurryEngine.Runtime.runtimeconfig.json', 'Resources/Shader/common.hlsli',
    'Resources/Assets/Shaders/geometric_primitive_vs.cso', 'Resources/Data/Shaders/geometric_primitive_ps.cso',
    'Templates/Empty/imgui.ini', 'Templates/Empty/Assets/Scenes/Main.scene', 'Templates/Empty/additional.log',
    'Templates/Other/Assets/Nested/example.txt', 'Templates/Other/empty.txt',
    'Templates/Other/EmptyDirectory', 'Templates/README.txt') { Check (Test-Path -LiteralPath "$payload/$path") "Missing payload: $path" }
Check ((Get-PackageHashes "$fixtureRepo/Templates" | ConvertTo-Json -Compress) -ceq
    (Get-PackageHashes "$payload/Templates" | ConvertTo-Json -Compress)) 'Template contents changed during copying'
foreach ($path in 'Assets', 'UserScripts', 'Assembly-CSharp.dll', 'CurryEngine.pdb', 'Resources/Data/Icon/unwanted.log',
    'Resources/Shader/unwanted.obj', 'Resources/Assets/Shaders/stale.cso') {
    Check (-not (Test-Path -LiteralPath "$payload/$path")) "Unwanted payload: $path"
}
Reject { Copy-EditorPayload $fixtureRepo $native $managed $payload } 'Existing staging was overwritten'
$before = Get-PackageHashes $payload | ConvertTo-Json -Compress
Reject { Copy-PackageFile "$root/missing" "$payload/CurryEngine.exe" } 'Missing input accepted'
Check ($before -ceq (Get-PackageHashes $payload | ConvertTo-Json -Compress)) 'Missing input damaged payload'
$missingShader = "$native/Shaders/geometric_primitive_vs.cso"
[IO.File]::Move($missingShader, "$missingShader.saved")
Reject { Copy-EditorPayload $fixtureRepo $native $managed "$root/incomplete" } 'Missing current shader accepted'
[IO.File]::Move("$missingShader.saved", $missingShader)
Fixture "$root/occupied/CurryEngine-1.0.0-win-x64.zip" 'original archive'
Reject { Publish-EditorPackage $payload "$root/occupied" '1.0.0' "$root/occupied/stage" } 'Existing ZIP overwritten'
Check ((Get-Content -LiteralPath "$root/occupied/CurryEngine-1.0.0-win-x64.zip" -Raw) -ceq 'original archive') 'Existing ZIP changed'

# Exercise stream draining, Unicode arguments and timeout behavior using a real child process.
$pwsh = (Get-Process -Id $PID).Path
Fixture "$root/child.ps1" 'param([string]$Value) [Console]::OutputEncoding=[Text.Encoding]::UTF8; [Console]::Error.Write([string]::new([char]69,100000)); [Console]::Write($Value)'
$text = Invoke-PackageTool $pwsh @('-NoProfile', '-File', "$root/child.ps1", '日本語 & space') $root "$root/streams.log" 30
Check ($text -ceq '日本語 & space') 'Arguments or UTF-8 output corrupted'
Fixture "$root/arguments.ps1" 'param([string]$Value) [Console]::OutputEncoding=[Text.Encoding]::UTF8; [Console]::Write($Value)'
foreach ($value in @('', 'C:\space dir\', 'embedded"quote', 'slashes\\"quote', '%PATH% & 日本語')) {
    $text = Invoke-PackageTool $pwsh @('-NoProfile', '-File', "$root/arguments.ps1", $value) $root "$root/arguments.log" 30
    Check ([string]$text -ceq $value) "Argument changed: $value"
}
Check ((Get-PackageRelativePath $root "$root/hash # percent % 日本語/file.txt") -ceq 'hash # percent % 日本語\file.txt') 'Relative path decoded incorrectly'
$zip = "$root/roundtrip.zip"
[IO.Compression.ZipFile]::CreateFromDirectory($payload, $zip)
[IO.Compression.ZipFile]::ExtractToDirectory($zip, "$root/unzipped")
Check ((Get-PackageHashes $payload | ConvertTo-Json -Compress) -ceq (Get-PackageHashes "$root/unzipped" | ConvertTo-Json -Compress)) 'ZIP roundtrip changed payload'
$longDirectory = "$root/long" + ('/nested-directory' * 18)
Fixture "$longDirectory/SDK.txt" 'long path fixture'
[IO.Compression.ZipFile]::CreateFromDirectory("$root/long", "$root/long.zip")
[IO.Compression.ZipFile]::ExtractToDirectory("$root/long.zip", "$root/long-extracted")
Check ((Get-PackageHashes "$root/long" | ConvertTo-Json -Compress) -ceq (Get-PackageHashes "$root/long-extracted" | ConvertTo-Json -Compress)) 'Long-path ZIP roundtrip failed'
Reject { Invoke-PackageTool $pwsh @('-NoProfile', '-Command', 'exit 7') $root "$root/exit.log" 30 } 'Failed command accepted'
Reject { Invoke-PackageTool $pwsh @('-NoProfile', '-Command', 'Start-Sleep -Seconds 30') $root "$root/timeout.log" 1 } 'Timeout ignored'
Check (Test-Path -LiteralPath "$root/timeout.log") 'Timeout log was not saved'
$originalInvoke = (Get-Item Function:Invoke-PackageTool).ScriptBlock
try {
    function Invoke-PackageTool { "`n    $script:dependency`n" }
    Fixture "$root/dependencies/CurryEngine.exe"
    $script:dependency = 'KERNEL32.dll'
    $null = Test-NativePackageDependencies "$root/dependencies" 'unused' $root
    $script:checks++
    $script:dependency = 'VCRUNTIME140D.dll'
    Fixture "$root/dependencies/VCRUNTIME140D.dll"
    Reject { Test-NativePackageDependencies "$root/dependencies" 'unused' $root } 'Debug runtime accepted even when bundled'
    $script:dependency = 'VCRUNTIME140.dll'
    Reject { Test-NativePackageDependencies "$root/dependencies" 'unused' $root } 'Machine-installed CRT substituted for bundled CRT'
    Fixture "$root/dependencies/VCRUNTIME140.dll"
    $null = Test-NativePackageDependencies "$root/dependencies" 'unused' $root
    $script:checks++
} finally { Set-Item Function:Invoke-PackageTool $originalInvoke }
Write-Output "$script:checks checks passed. Fixtures: $root"
