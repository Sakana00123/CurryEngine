@echo off
setlocal DisableDelayedExpansion
rem Projects must be outside the shared engine directory.
set "project=%~dp0..\CurryEngineProject"
if not "%~1"=="" set "project=%~f1"
if exist "%project%\Project.curryproject" goto :open
"%~dp0CurryEngine.exe" --create-project "%project%" --name "CurryEngineProject"
if errorlevel 1 goto :failed
:open
"%~dp0CurryEngine.exe" --project "%project%\Project.curryproject"
if errorlevel 1 goto :failed
exit /b 0
:failed
echo Failed to create or open the project. Check the project Logs folder.
pause
exit /b 1
