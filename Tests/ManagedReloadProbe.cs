using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using CurryEngine.Runtime;

static class Probe
{
    static void Check(bool condition, string message)
    {
        if (!condition) throw new Exception(message);
    }

    static int Value() => (int)ScriptRegistry.Resolve("ProjectProbe").GetMethod("Value").Invoke(null, null);

    static int Build(string root)
    {
        var start = new ProcessStartInfo("dotnet") { WorkingDirectory = root, UseShellExecute = false };
        foreach (var arg in new[] { "build", "Assembly-CSharp.csproj", "-c", "Debug", "--nologo", "-v", "quiet" })
            start.ArgumentList.Add(arg);
        using var process = Process.Start(start);
        process.WaitForExit();
        return process.ExitCode;
    }

    public static unsafe int Main(string[] args)
    {
        var rootA = Path.GetFullPath(args[0]);
        var rootB = Path.GetFullPath(args[1]);
        var stub = Path.GetFullPath(args[2]);
        Directory.SetCurrentDirectory(rootA);
        NativeLibrary.SetDllImportResolver(typeof(EngineRuntime).Assembly, (_, _, _) => NativeLibrary.Load(stub));
        var dllA = Path.Combine(rootA, "Library/ScriptAssemblies/Assembly-CSharp.dll");
        var dllB = Path.Combine(rootB, "Library/ScriptAssemblies/Assembly-CSharp.dll");
        var bytes = Encoding.UTF8.GetBytes(dllA + "\0");
        delegate* unmanaged[Stdcall]<byte*, int> initialize = &EngineRuntime.EngineInitialize;
        delegate* unmanaged[Stdcall]<byte*, int> reload = &EngineRuntime.ReloadScripts;
        fixed (byte* path = bytes)
        {
            Check(!File.Exists(dllA), "Fixture A should have no compiled scripts initially.");
            Check(initialize(path) == 1, "Initialization without an initial DLL failed.");
            Check(!ScriptRegistry.IsRegistered("ProjectProbe"), "Another project's scripts were loaded.");
            Check(Build(rootA) == 0, "Initial project build failed.");
            Check(reload(path) == 1, "First reload failed.");
            Check(Value() == 1, "First load did not execute project A.");
            Check(ScriptRegistry.Resolve("ProjectProbe").BaseType == typeof(CurryEngine.Behaviour), "Shared Core type identity was lost.");
            File.WriteAllText(Path.Combine(rootA, "Assets/Scripts/ProjectProbe.cs"),
                "using CurryEngine; public class ProjectProbe : Behaviour { public static int Value() => 2; }");
            Check(Build(rootA) == 0, "Rebuilding the loaded DLL failed (file lock).");
            Check(reload(path) == 1, "Second reload failed.");
            Check(Value() == 2, "Hot reload did not pick up the edit.");
            File.WriteAllText(Path.Combine(rootA, "Assets/Scripts/ProjectProbe.cs"), "invalid C# fixture");
            Check(Build(rootA) != 0, "Invalid C# unexpectedly built.");
            Check(Value() == 2, "Failed compilation changed the working scripts.");
            var otherBytes = Encoding.UTF8.GetBytes(dllB + "\0");
            fixed (byte* other = otherBytes) Check(reload(other) == 0, "Cross-project reload reported success.");
            Check(Value() == 2, "Reload accepted another project's DLL.");
            var goodDll = File.ReadAllBytes(dllA);
            File.WriteAllText(dllA, "invalid DLL fixture");
            Check(reload(path) == 0 && Value() == 2, "Bad DLL replaced the last successful assembly.");
            File.WriteAllBytes(dllA, goodDll);
            File.WriteAllText(Path.Combine(rootA, "Assets/Scripts/ProjectProbe.cs"),
                "using CurryEngine; public class RenamedProbe : Behaviour { }");
            Check(Build(rootA) == 0 && reload(path) == 1, "Class rename reload failed.");
            Check(!ScriptRegistry.IsRegistered("ProjectProbe") && ScriptRegistry.Resolve("ProjectProbe") == null &&
                ScriptRegistry.IsRegistered("RenamedProbe"), "Deleted script type remained registered.");
        }
        Check(!Directory.Exists(Path.Combine(AppContext.BaseDirectory, "Temp")), "Hot reload wrote beside the engine.");
        Console.WriteLine("Managed integration passed: initial DLL absent, compile/load, shared types, rebuild/reload, failed build, cross-project rejection, bad DLL, class rename.");
        return 0;
    }
}
