@echo off
echo [PROGRESS_TOTAL 5]
setlocal EnableDelayedExpansion
chcp 65001 > nul

REM ===== build_settings.json をパース =====
echo [PROGRESS 1] Parsing build_settings.json ...
set "SETTINGS=%~dp0BuildSetting\build_settings.json"

for /f "delims=" %%i in ('powershell -NoProfile -Command ^
    "$s=Get-Content '%SETTINGS%' | ConvertFrom-Json; $s.appName"') do set "APP_NAME=%%i"

for /f "delims=" %%i in ('powershell -NoProfile -Command ^
    "$s=Get-Content '%SETTINGS%' | ConvertFrom-Json; $s.iconPath"') do set "ICON_PATH=%%i"

for /f "delims=" %%i in ('powershell -NoProfile -Command ^
    "$s=Get-Content '%SETTINGS%' | ConvertFrom-Json; $s.outputDir"') do set "OUTPUT_DIR=%%i"

for /f "delims=" %%i in ('powershell -NoProfile -Command ^
    "$s=Get-Content '%SETTINGS%' | ConvertFrom-Json; $s.zipToolPath"') do set "ZIP_EXE=%%i"

echo [Build] App      : %APP_NAME%
echo [Build] Icon     : %ICON_PATH%
echo [Build] Output   : %OUTPUT_DIR%

REM ===== 絶対パスに変換 =====
pushd "%~dp0"
for %%I in ("%OUTPUT_DIR%") do set "OUTPUT_FULL=%%~fI"
popd

REM ===== AppName サブフォルダを挟む
set "OUTPUT_FULL=%OUTPUT_FULL%\%APP_NAME%"

REM ===== icon.rc 生成 =====
echo [Build] Patching icon.rc ...
powershell -NoProfile -Command ^
    "(Get-Content '%~dp0BuildSetting\icon.rc.template') -replace '{{ICON_PATH}}','%ICON_PATH%' | Set-Content '%~dp0BuildSetting\icon.rc'"

REM ===== MSBuild =====
echo [PROGRESS 2] Searching for MSBuild ...
set VSWHERE="C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "delims=" %%i in ('%VSWHERE% -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe') do set "MSBUILD=%%i"

if not defined MSBUILD (
    echo [ERROR] MSBuild not found.
    exit /b 1
)

echo [PROGRESS 3] Building C++ project ...
"%MSBUILD%" "%~dp0CurryEngine.sln" /p:Configuration=Release /p:Platform=x64 /nologo /v:m
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Build failed.
    exit /b 1
)

REM ===== 出力先準備 =====
if not exist "%OUTPUT_FULL%" mkdir "%OUTPUT_FULL%"

REM ===== exe コピー＆リネーム =====
echo [Build] Copying executable ...
copy /Y "%~dp0x64\Release\CurryEngine.exe" "%OUTPUT_FULL%\" > nul
if exist "%OUTPUT_FULL%\CurryEngine.exe" (
    del /f /q "%OUTPUT_FULL%\%APP_NAME%.exe" 2> nul
    move /Y "%OUTPUT_FULL%\CurryEngine.exe" "%OUTPUT_FULL%\%APP_NAME%.exe" > nul
)

echo [PROGRESS 4] Copying additional files ...
REM ===== copyItems をループ =====
powershell -NoProfile -Command ^
    "$settings='%SETTINGS%'.Replace('/', '\');" ^
    "$out='%OUTPUT_FULL%';" ^
    "$s=Get-Content $settings | ConvertFrom-Json;" ^
    "$s.copyItems | ForEach-Object {" ^
    "  $type=$_.type;" ^
    "  $src=$_.src.Replace('/', '\');" ^
    "  $exclude=@();" ^
    "  if ($_.PSObject.Properties['exclude']) { $exclude=$_.exclude };" ^
    "  Write-Host \"[Build] Copy [$type] $src\";" ^
    "  if ($type -eq 'file') {" ^
    "    Copy-Item $src $out -Force" ^
    "  }" ^
    "  if ($type -eq 'folder') {" ^
    "    $name=Split-Path $src -Leaf;" ^
    "    $dest=$out+'\'+$name;" ^
    "    if (Test-Path $dest) { Remove-Item $dest -Recurse -Force }" ^
    "    Copy-Item $src $dest -Recurse -Force;" ^
    "    foreach ($pat in $exclude) {" ^
    "      Get-ChildItem -Path $dest -Recurse | Where-Object { $_.Name -like $pat } | ForEach-Object {" ^
    "        if (Test-Path $_.FullName) {" ^
    "          Write-Host \"[Build] Exclude $($_.FullName)\";" ^
    "          Remove-Item $_.FullName -Recurse -Force" ^
    "        }" ^
    "      }" ^
    "    }" ^
    "  }" ^
    "  if ($type -eq 'glob') {" ^
    "    $dir=Split-Path $src -Parent;" ^
    "    $pattern=Split-Path $src -Leaf;" ^
    "    Get-ChildItem -Path $dir -Filter $pattern | ForEach-Object { Copy-Item $_.FullName $out -Force }" ^
    "  }" ^
    "}"

REM ===== ZIP 圧縮 =====
echo [PROGRESS 5] Compressing output ...
set "ZIP_FILE=%OUTPUT_FULL%.zip"
echo [Build] Compressing to %ZIP_FILE% ...

if not exist "%ZIP_EXE%" (
    echo [WARN] 7z.exe not found, skipping compression.
    goto :done
)

if exist "%ZIP_FILE%" del /f /q "%ZIP_FILE%"
"%ZIP_EXE%" a "%ZIP_FILE%" "%OUTPUT_FULL%\*" -r -y > nul
echo [Build] ZIP created: %ZIP_FILE%

:done
echo [OK] Build complete ^-^> %OUTPUT_FULL%\%APP_NAME%.exe

:: エクスプローラーで出力ディレクトリを開く
::explorer "%OUTPUT_FULL%"

:: exe をハイライトした状態で開く
explorer /select,"%OUTPUT_FULL%\%APP_NAME%.exe"

exit /b 0