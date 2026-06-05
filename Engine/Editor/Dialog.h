#pragma once

#include <Windows.h>

// ダイアログリザルト
enum class DialogResult
{
	OK,
	Cancel
};

// ダイアログ
class Dialog
{
public:
	// [ファイルを開く]ダイアログボックスを表示
	static DialogResult OpenFileName(char* filepath, int size, const char* filter = nullptr, const char* title = nullptr, HWND hWnd = NULL, bool multiSelect = false);

	// [ファイルを保存]ダイアログボックスを表示
	static DialogResult SaveFileName(char* filepath, int size, const char* filter = nullptr, const char* title = nullptr, const char* ext = nullptr, HWND hWnd = NULL);

	// [ディレクトリを選択]ダイアログボックスを表示
	static DialogResult SelectDirectoryName(char* directoryPath, int size, const char* title = nullptr, HWND hWnd = NULL);
};

// ファイルオープンダイアログを表示し、選択されたファイルパスを返す（内部でDialogクラスを使用）
char* OpenFileDialog(const char* filter = nullptr, const char* title = nullptr, HWND hWnd = NULL, bool multiSelect = false);

// ファイルセーブダイアログを表示し、選択されたファイルパスを返す（内部でDialogクラスを使用）
char* SaveFileDialog(const char* filter = nullptr, const char* title = nullptr, const char* ext = nullptr, HWND hWnd = NULL);

// ファイル選択ダイアログを表示し、選択されたファイルパスを返す（内部でDialogクラスを使用）
char* SelectFileDialog(const char* title = nullptr, const char* filter = nullptr, HWND hWnd = NULL, bool multiSelect = false);

// ディレクトリ選択ダイアログを表示し、選択されたディレクトリパスを返す（内部でDialogクラスを使用）
char* SelectDirectoryDialog(const char* title = nullptr, HWND hWnd = NULL);