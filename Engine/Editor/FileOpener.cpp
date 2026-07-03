#include "pch.h"
#include "FileOpener.h"

#include <filesystem>
#include <Windows.h>
#include <shellapi.h>

void OpenFileWithDefaultApplication(const std::wstring& filePath)
{
	// ShellExecute関数を使用して、指定したファイルを関連付けられたアプリケーションで開く
	HINSTANCE result = ShellExecuteW(
		NULL,               // 親ウィンドウのハンドル（NULLの場合はデスクトップ）
		L"open",           // 実行する操作（"open"はファイルを開く）
		filePath.c_str(),  // 開くファイルのパス
		NULL,              // コマンドライン引数（不要な場合はNULL）
		NULL,              // 作業ディレクトリ（不要な場合はNULL）
		SW_SHOWNORMAL      // ウィンドウの表示方法
	);
	// エラーチェック
	if ((INT_PTR)result <= 32) {
		// エラー処理（必要に応じてログ出力やメッセージボックス表示などを行う）
		wchar_t msg[256];
		swprintf_s(msg, L"ファイルを開くことができませんでした。エラーコード: %lld\nパス: %s", (INT_PTR)result, filePath.c_str());
		MessageBoxW(NULL, msg, L"エラー", MB_OK | MB_ICONERROR);
	}
}

bool RunHiddenAndWait(const std::wstring& exePath, const std::wstring& args, const std::wstring& currentDirectory, DWORD& outExitCode, DWORD timeoutMs)
{
    // コマンドラインは CreateProcess に渡すとき書き換えられる可能性があるので可変バッファにする
    std::wstring commandLine = L"\"" + exePath + L"\"";
    if (!args.empty()) {
        commandLine += L" " + args;
    }
    std::vector<wchar_t> cmdBuf(commandLine.begin(), commandLine.end());
    cmdBuf.push_back(L'\0');

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    // ウィンドウ表示指定を使う
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // 非表示

    ZeroMemory(&pi, sizeof(pi));

    // CREATE_NO_WINDOW を付けるとコンソールは生成されない（コンソールアプリ向け）
    DWORD creationFlags = CREATE_NO_WINDOW;

    BOOL ok = CreateProcessW(
        nullptr,                    // lpApplicationName（nullptrにするとコマンドラインから exe を探す）
        cmdBuf.data(),              // lpCommandLine（mutable wchar_t* が必要）
        nullptr, nullptr,           // lpProcessAttributes, lpThreadAttributes
        FALSE,                      // bInheritHandles
        creationFlags,              // dwCreationFlags
        nullptr,                    // lpEnvironment
        currentDirectory.c_str(),   // lpCurrentDirectory
        &si,
        &pi
    );

    if (!ok) {
        OutputDebugStringA(("CreateProcess failed. Error: " + std::to_string(GetLastError()) + "\n").c_str());
        return false;
    }

    // プロセス終了を待つ
    DWORD wait = WaitForSingleObject(pi.hProcess, timeoutMs);
    if (wait == WAIT_OBJECT_0) {
        if (!GetExitCodeProcess(pi.hProcess, &outExitCode)) {
            outExitCode = STILL_ACTIVE; // 取得失敗時のフォールバック
        }
    }
    else {
        // タイムアウトなど
        outExitCode = STILL_ACTIVE;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}


std::wstring GetDevenvPath(const std::wstring& preferredVersion = L"")
{
    // vswhere.exeはVS Installerと一緒に必ずインストールされる
    const std::wstring vswhere =
        L"C:\\Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere.exe";

    if (!std::filesystem::exists(vswhere)) return L"";

    // vswhere -version [16,17) -property installationPath
    // preferredVersionが空なら最新、指定があればそのバージョン
    std::wstring args = L" -latest -property installationPath";
    if (!preferredVersion.empty())
        args = L" -version \"" + preferredVersion + L"\" -property installationPath";

    // パイプで出力を取得
    std::wstring cmd = L"\"" + vswhere + L"\"" + args;

    SECURITY_ATTRIBUTES sa = { sizeof(sa), nullptr, TRUE };
    HANDLE hRead, hWrite;
    CreatePipe(&hRead, &hWrite, &sa, 0);

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;
    si.dwFlags = STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi = {};
    std::vector<wchar_t> cmdBuf(cmd.begin(), cmd.end());
    cmdBuf.push_back(L'\0');

    if (!CreateProcessW(nullptr, cmdBuf.data(), nullptr, nullptr,
        TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
    {
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return L"";
    }

    CloseHandle(hWrite);
    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // 出力を読み取り
    char buf[MAX_PATH] = {};
    DWORD read = 0;
    ReadFile(hRead, buf, sizeof(buf) - 1, &read, nullptr);
    CloseHandle(hRead);

    // 改行除去してdevenv.exeのパスを組み立て
    std::string installPath(buf, read);
    while (!installPath.empty() && (installPath.back() == '\n' || installPath.back() == '\r'))
        installPath.pop_back();

    if (installPath.empty()) return L"";

    std::wstring wInstallPath(installPath.begin(), installPath.end());
    return wInstallPath + L"\\Common7\\IDE\\devenv.exe";
}


bool OpenFileInVisualStudio(const std::wstring& slnPath, const std::wstring& filePath, int lineNumber)
{
    // GetDevenvPathの戻り値からクォートを除去
    auto stripQuotes = [](std::wstring s) -> std::wstring {
        s.erase(std::remove(s.begin(), s.end(), L'"'), s.end());
        return s;
        };

    // Visual Studio の devenv.exe を探す

    // [17,18] = VS2022のみ
    std::wstring devenv = stripQuotes(GetDevenvPath(L"[17,18)"));

    // 空にすれば最新版（現在の挙動と同じ）
    //std::wstring devenv = stripQuotes(GetDevenvPath());

    if (devenv.empty())
    {
        MessageBoxW(NULL, L"Visual Studioが見つかりませんでした。", L"エラー", MB_OK | MB_ICONERROR);
        return false;
    }

    // 相対パスを絶対パスに変換
    wchar_t absSlnPath[MAX_PATH];
    wchar_t absFilePath[MAX_PATH];
    _wfullpath(absSlnPath, slnPath.c_str(), MAX_PATH);
    _wfullpath(absFilePath, filePath.c_str(), MAX_PATH);

    // コマンドライン組み立て
    std::wstring cmdLine =
        L"\"" + devenv + L"\" " + // devenv.exe のパス
        L"\"" + absSlnPath + L"\" " + // ソリューションファイルのパス
		L"/Edit \"" + absFilePath + L"\""; // 開くファイルのパス

    // CreateProcessWは書き換え可能バッファが必要
    std::vector<wchar_t> cmdBuf(cmdLine.begin(), cmdLine.end());
    cmdBuf.push_back(L'\0');

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    BOOL ok = CreateProcessW(
        nullptr,
        cmdBuf.data(),  // ← vector<wchar_t>のdata()
        nullptr, nullptr,
        FALSE,
        DETACHED_PROCESS,
        nullptr, nullptr,
        &si, &pi
    );

    if (ok)
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else
    {
        DWORD err = GetLastError();
        wchar_t msg[512];
        swprintf_s(msg, L"CreateProcessW失敗\nエラーコード: %lu\nコマンド: %s", err, cmdBuf.data());
        MessageBoxW(NULL, msg, L"エラー", MB_OK | MB_ICONERROR);
    }

    return ok != FALSE;
}