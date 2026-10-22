CurryEngine portable editor (Windows x64)

Extract the entire ZIP into a writable folder, then double-click StartEditor.bat.
It creates/opens CurryEngineProject next to the engine folder.
To choose a project folder: StartEditor.bat "D:\MyProject"
The engine and project folders must be separate (neither inside the other).
Alternatively, register CurryEngine.exe in CurryHub's Engine Versions > Add.

Requirements: Windows 10/11 x64, Direct3D 11.1 compatible GPU and driver.
The .NET 9 SDK/runtime and redistributable C++ runtime are included.
No PowerShell 7, Visual Studio or separate .NET installation is required for
normal editor use. Keep DotNet and the other payload folders intact.
Edit C# files with a text editor such as Notepad; saving triggers compilation.
Building standalone games and Visual Studio debugging require development tools.

engine.json/version.txt identify this engine version. package-files.json records
payload SHA256 hashes. The accompanying .zip.sha256 verifies the ZIP.
The packaging command checks extracted project commands and bundled-SDK builds.
Rendering, editing and play must also be verified on a clean Windows machine.
Startup diagnostics are in the project's Logs/startup.log.