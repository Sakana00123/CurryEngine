@echo off
setlocal DisableDelayedExpansion

rem Usage: build_editor_package.bat [version]
set "packageVersion=0.1.0-portable.1"
if not "%~1"=="" set "packageVersion=%~1"

pushd "%~dp0"
if errorlevel 1 (
    echo [ERROR] Could not open the CurryEngine directory.
    set "packageExitCode=1"
    goto :finish
)

"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoLogo -NoProfile -ExecutionPolicy Bypass -File ".\Tools\Packaging\Build-EditorPackage.ps1" -Version "%packageVersion%"
set "packageExitCode=%ERRORLEVEL%"
popd

if not "%packageExitCode%"=="0" echo [ERROR] Package creation failed. See the output above.

:finish
rem Keep the result visible when launched without arguments, including by double-click.
if "%~1"=="" pause
exit /b %packageExitCode%
