#pragma once

#include <Windows.h>

// ダイアログリザルト
/** @brief DialogResult を表す列挙型です。 */
enum class DialogResult
{
	OK,
	Cancel
};

// ダイアログ
/** @brief Dialog を表すクラスです。 */
class Dialog
{
public:
	// [ファイルを開く]ダイアログボックスを表示
	/**
	 * @brief OpenFileName の処理を行います。
	 */
	static DialogResult OpenFileName(char* filepath, int size, const char* filter = nullptr, const char* title = nullptr, HWND hWnd = NULL, bool multiSelect = false);

	// [ファイルを保存]ダイアログボックスを表示
	/**
	 * @brief SaveFileName の処理を行います。
	 */
	static DialogResult SaveFileName(char* filepath, int size, const char* filter = nullptr, const char* title = nullptr, const char* ext = nullptr, HWND hWnd = NULL);

	// [ディレクトリを選択]ダイアログボックスを表示
	/**
	 * @brief SelectDirectoryName の処理を行います。
	 */
	static DialogResult SelectDirectoryName(char* directoryPath, int size, const char* title = nullptr, HWND hWnd = NULL);
};

// ファイルオープンダイアログを表示し、選択されたファイルパスを返す（内部でDialogクラスを使用）
/**
 * @brief OpenFileDialog の処理を行います。
 */
char* OpenFileDialog(const char* filter = nullptr, const char* title = nullptr, HWND hWnd = NULL, bool multiSelect = false);

// ファイルセーブダイアログを表示し、選択されたファイルパスを返す（内部でDialogクラスを使用）
/**
 * @brief SaveFileDialog の処理を行います。
 */
char* SaveFileDialog(const char* filter = nullptr, const char* title = nullptr, const char* ext = nullptr, HWND hWnd = NULL);

// ファイル選択ダイアログを表示し、選択されたファイルパスを返す（内部でDialogクラスを使用）
/**
 * @brief SelectFileDialog の処理を行います。
 */
char* SelectFileDialog(const char* title = nullptr, const char* filter = nullptr, HWND hWnd = NULL, bool multiSelect = false);

// ディレクトリ選択ダイアログを表示し、選択されたディレクトリパスを返す（内部でDialogクラスを使用）
/**
 * @brief SelectDirectoryDialog の処理を行います。
 */
char* SelectDirectoryDialog(const char* title = nullptr, HWND hWnd = NULL);