// EngineAPI/Debug.cs
using CurryEngine.Interop;
using System.Runtime.CompilerServices;

namespace CurryEngine;

public static class Debug
{
    public enum LogLevel
    {
        Info = 0,
        Warning = 1,
        Error = 2
    }

    public static void Log(object? message,
        [CallerFilePath] string file = "",
        [CallerLineNumber] int line = 0)
        => NativeMethods.Console_CustomLog((int)LogLevel.Info, message?.ToString() ?? "", file, line);

    public static void LogWarning(object? message,
        [CallerFilePath] string file = "",
        [CallerLineNumber] int line = 0)
        => NativeMethods.Console_CustomLog((int)LogLevel.Warning, message?.ToString() ?? "", file, line);

    public static void LogError(object? message,
        [CallerFilePath] string file = "",
        [CallerLineNumber] int line = 0)
        => NativeMethods.Console_CustomLog((int)LogLevel.Error, message?.ToString() ?? "", file, line);
}