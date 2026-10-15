CurryEngine development editor (Windows x64)

1. Extract the entire ZIP to its own engine folder.
2. In CurryHub, open Engine Versions > Add and select CurryEngine.exe.
3. Use New Project and choose a location outside the engine folder.

Current build: Debug x64 editor, with Release .NET engine assemblies.
Required on the target PC:
- Windows x64 with a compatible Direct3D 11 environment.
- .NET 9 SDK for compiling project scripts.
- Visual Studio 2022, C# development tools and the C++ development workload
  supplying the Visual C++ Debug runtime used by this editor.

The package does not install these prerequisites or bundle Visual C++ Debug
runtime DLLs. It is currently intended for development PCs. An optimized editor
build independent of the Debug runtime is a separate remaining task.

engine.json and version.txt come from the same version argument.
package-files.json records SHA256 hashes of payload files (excluding itself).
The accompanying .zip.sha256 verifies the ZIP.

The packaging command tests project creation, validation, solution generation
and script compilation using the extracted ZIP in another location. Rendering,
editor play and a clean-machine installation require separate verification.
