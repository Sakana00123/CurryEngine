
using System.Runtime.CompilerServices;
using System.IO;

namespace CurryEngine;

/// <summary>
/// デバッグのための静的クラスです。現状はログ出力のみを提供します。将来的には、デバッグ用のユーティリティ関数が追加される予定です。
/// </summary>
public static class Debug
{
    public enum LogLevel
    {
        Info = 0,
        Warning = 1,
        Error = 2
    }

    /// <summary>
    /// C++側のログ出力関数を呼び出すためのデリゲート。
    /// </summary>
    internal static Action<int, string, string, int>? LogNativeHandler { get; set; }

    /// <summary>
    /// C++側のログ出力関数を呼び出す内部メソッド。
    /// </summary>
    /// <param name="level"> ログレベル</param>
    /// <param name="message"> ログメッセージ</param>
    /// <param name="file"> 呼び出し元のファイルパス</param>
    /// <param name="line"> 呼び出し元の行番号</param>
    internal static void LogInternal(LogLevel level, object? message,
        [CallerFilePath] string file = "",
        [CallerLineNumber] int line = 0)
    {
        // テキストファイルにログを出力
        string logMessage = $"[{level}] {message?.ToString() ?? ""} (File: {file}, Line: {line}){Environment.NewLine}";
        File.AppendAllText("debug_log.txt", logMessage);
        // C++側のログ出力関数が設定されている場合に呼び出す
        LogNativeHandler?.Invoke((int)level, message?.ToString() ?? "", file, line);
    }


    /// <summary>
    /// デバッグログを出力します。
    /// </summary>
    /// <param name="message"> ログメッセージ</param>
    /// <param name="file"> 呼び出し元のファイルパス</param>
    /// <param name="line"> 呼び出し元の行番号</param>
    public static void Log(object? message,
        [CallerFilePath] string file = "",
        [CallerLineNumber] int line = 0)
        => LogInternal(LogLevel.Info, message, file, line);

    /// <summary>
    /// 警告ログを出力します。
    /// </summary>
    /// <param name="message"> ログメッセージ</param> 
    /// <param name="file"> 呼び出し元のファイルパス</param>
    /// <param name="line"> 呼び出し元の行番号</param>
    public static void LogWarning(object? message,
        [CallerFilePath] string file = "",
        [CallerLineNumber] int line = 0)
        => LogInternal(LogLevel.Warning, message, file, line);

    /// <summary>
    /// エラーログを出力します。
    /// </summary>
    /// <param name="message"> ログメッセージ</param>
    /// <param name="file"> 呼び出し元のファイルパス</param>
    /// <param name="line"> 呼び出し元の行番号</param>
    public static void LogError(object? message,
        [CallerFilePath] string file = "",
        [CallerLineNumber] int line = 0)
        => LogInternal(LogLevel.Error, message, file, line);
}
