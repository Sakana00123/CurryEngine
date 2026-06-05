#pragma once
#include <string>
/**
 * @brief 指定したファイルを関連付けられたアプリケーションで開く。
 * @param filePath 開くファイルのパス。
 */
void OpenFileWithDefaultApplication(const std::wstring& filePath);

/**
 * @brief 指定した実行ファイルを非表示で実行し、終了を待機する。
 * @param exePath 実行ファイルのパス。
 * @param args コマンドライン引数。
 * @param currentDirectory プロセスの作業ディレクトリ。
 * @param outExitCode プロセスの終了コードを格納する変数への参照。
 * @param timeoutMs プロセスの終了を待つタイムアウト時間（ミリ秒）。デフォルトは無限待機。
 * @return プロセスの起動に成功した場合は true、失敗した場合は false を返す。
 */
bool RunHiddenAndWait(const std::wstring& exePath, const std::wstring& args, const std::wstring& currentDirectory, DWORD& outExitCode, DWORD timeoutMs = INFINITE);


/**
 * @brief Visual Studio で指定したソリューションファイルを開き、特定のファイルと行にジャンプする。
 * @param slnPath ソリューションファイルのパス。
 * @param filePath ジャンプ先のファイルのパス。
 * @param lineNumber ジャンプ先の行番号（省略可能、デフォルトは -1 で行番号なし）。
 * @return ジャンプに成功した場合は true、失敗した場合は false を返す。
 */
bool OpenFileInVisualStudio(const std::wstring& slnPath, const std::wstring& filePath, int lineNumber = -1);