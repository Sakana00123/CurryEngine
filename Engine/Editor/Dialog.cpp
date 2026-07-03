#include "pch.h"
#include "Dialog.h"

#include <shlwapi.h>
#include <ShlObj.h>
#pragma comment(lib, "shlwapi.lib")

static char pathBuffer[MAX_PATH];

// [ファイルを開く]ダイアログボックスを表示
DialogResult Dialog::OpenFileName(char* filepath, int size, const char* filter, const char* title, HWND hWnd, bool multiSelect)
{
	// 初期パス設定
	char dirname[MAX_PATH];
	if (filepath[0] != '0')
	{
		// ディレクトリパス取得
		::_splitpath_s(filepath, nullptr, 0, dirname, MAX_PATH, nullptr, 0, nullptr, 0);
	}
	else
	{
		filepath[0] = dirname[0] = '\0';
	}
	if ((dirname[0] == '\0'))
	{
		strcpy_s(dirname, MAX_PATH, pathBuffer);
	}
	// lpstrInitialDir は \ でないと受け付けない
	for (char* p = dirname; *p != '\0'; p++)
	{
		if (*p == '/')
			* p = '\\';
	}

	if (filter == nullptr)
	{
		filter = "All Files\0*.*\0\0";
	}

	// 構造体セット
	OPENFILENAMEA	ofn;
	memset(&ofn, 0, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = filepath;
	ofn.nMaxFile = size;
	ofn.lpstrTitle = title;
	ofn.lpstrInitialDir = (dirname[0] != '\0') ? dirname : nullptr;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	if (multiSelect)
	{
		ofn.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	}

	// カレントディレクトリ取得
	char currentDir[MAX_PATH];
	if (!::GetCurrentDirectoryA(MAX_PATH, currentDir))
	{
		currentDir[0] = '\0';
	}

	// ダイアログオープン
	if (::GetOpenFileNameA(&ofn) == FALSE)
	{
		return DialogResult::Cancel;
	}

	// カレントディレクトリ復帰
	if (currentDir[0] != '\0')
	{
		::SetCurrentDirectoryA(currentDir);
	}

	// 最終パスを記憶
	strcpy_s(pathBuffer, MAX_PATH, filepath);

	//カレントディレクトリに対する相対パスに変換
	char relativePath[MAX_PATH];
	char baseDir[MAX_PATH];
	if (::GetCurrentDirectoryA(MAX_PATH, baseDir)) {
		if (::PathRelativePathToA(
			relativePath,
			baseDir,
			FILE_ATTRIBUTE_DIRECTORY,
			filepath,
			FILE_ATTRIBUTE_NORMAL)) {
			//相対パスへの変換が成功したらfilepathを上書き
			strcpy_s(filepath, size, relativePath);
		}
	}
	return DialogResult::OK;
}

// [ファイルを保存]ダイアログボックスを表示
DialogResult Dialog::SaveFileName(char* filepath, int size, const char* filter, const char* title, const char* ext, HWND hWnd)
{
	// 初期パス設定
	char dirname[MAX_PATH];
	if (filepath[0] != '0')
	{
		// ディレクトリパス取得
		::_splitpath_s(filepath, nullptr, 0, dirname, MAX_PATH, nullptr, 0, nullptr, 0);
	}
	else
	{
		filepath[0] = dirname[0] = '\0';
	}
	if ((dirname[0] == '\0'))
	{
		strcpy_s(dirname, MAX_PATH, pathBuffer);
	}
	// lpstrInitialDir は \ でないと受け付けない
	for (char* p = dirname; *p != '\0'; p++)
	{
		if (*p == '/')
			* p = '\\';
	}

	if (filter == nullptr)
	{
		filter = "All Files\0*.*\0\0";
	}

	// 構造体セット
	OPENFILENAMEA	ofn;
	memset(&ofn, 0, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = filepath;
	ofn.nMaxFile = size;
	ofn.lpstrTitle = title;
	ofn.lpstrInitialDir = (dirname[0] != '\0') ? dirname : nullptr;
	ofn.lpstrDefExt = ext;
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

	// カレントディレクトリ取得
	char current_dir[MAX_PATH];
	if (!::GetCurrentDirectoryA(MAX_PATH, current_dir))
	{
		current_dir[0] = '\0';
	}

	// ダイアログオープン
	if (::GetSaveFileNameA(&ofn) == FALSE)
	{
		return DialogResult::Cancel;
	}

	// カレントディレクトリ復帰
	if (current_dir[0] != '\0')
	{
		::SetCurrentDirectoryA(current_dir);
	}

	// 最終パスを記憶
	strcpy_s(pathBuffer, MAX_PATH, filepath);

	//カレントディレクトリに対する相対パスに変換
	char relativePath[MAX_PATH];
	char baseDir[MAX_PATH];
	if (::GetCurrentDirectoryA(MAX_PATH, baseDir)) {
		if (::PathRelativePathToA(
			relativePath,
			baseDir,
			FILE_ATTRIBUTE_DIRECTORY,
			filepath,
			FILE_ATTRIBUTE_NORMAL)) {
			//相対パスへの変換が成功したらfilepathを上書き
			strcpy_s(filepath, size, relativePath);
		}
	}

	return DialogResult::OK;
}

static std::string OpenFolderDialog(HWND hWnd, const char* title)
{
	std::string folderPath;
	// IFileDialog を使用してフォルダ選択ダイアログを表示
	IFileDialog* pFileDialog = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
	if (FAILED(hr)) {
		return folderPath;
	}
	// フォルダ選択モードを有効にする
	DWORD dwOptions = 0;
	pFileDialog->GetOptions(&dwOptions);
	pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS);

	if (title) {
		pFileDialog->SetTitle(std::wstring(title, title + strlen(title)).c_str());
	}

	if (SUCCEEDED(pFileDialog->Show(hWnd))) {
		IShellItem* pItem = nullptr;
		if (SUCCEEDED(pFileDialog->GetResult(&pItem))) {
			PWSTR pszFolderPath = nullptr;
			if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath))) {
				// wstring から string (UTF-8) に変換
				int len = WideCharToMultiByte(CP_UTF8, 0, pszFolderPath, -1,
					nullptr, 0, nullptr, nullptr);
				folderPath.resize(static_cast<size_t>(len - 1)); // null terminator を除く
				WideCharToMultiByte(CP_UTF8, 0, pszFolderPath, -1,
					folderPath.data(), len, nullptr, nullptr);

				CoTaskMemFree(pszFolderPath);
			}
			pItem->Release();
		}
	}

	pFileDialog->Release();
	return folderPath;
}



// [ディレクトリを選択]ダイアログボックスを表示
DialogResult Dialog::SelectDirectoryName(char* directoryPath, int size, const char* title, HWND hWnd)
{
	std::string folderPath = OpenFolderDialog(hWnd, title);
	if (!folderPath.empty()) {
		strncpy_s(directoryPath, size, folderPath.c_str(), _TRUNCATE);
		return DialogResult::OK;
	}
	return DialogResult::Cancel;
}

// ファイルオープンダイアログを表示し、選択されたファイルパスを返す（内部でDialogクラスを使用）
char* OpenFileDialog(const char* filter, const char* title, HWND hWnd, bool multiSelect)
{
	static char filepath[MAX_PATH] = "";
	if (Dialog::OpenFileName(filepath, MAX_PATH, filter, title, hWnd, multiSelect) == DialogResult::OK) {
		return filepath;
	}
	return nullptr;
}

// ファイルセーブダイアログを表示し、選択されたファイルパスを返す（内部でDialogクラスを使用）
char* SaveFileDialog(const char* filter, const char* title, const char* ext, HWND hWnd)
{
	static char filepath[MAX_PATH] = "";
	if (Dialog::SaveFileName(filepath, MAX_PATH, filter, title, ext, hWnd) == DialogResult::OK) {
		return filepath;
	}
	return nullptr;
}

// ファイル選択ダイアログを表示し、選択されたファイルパスを返す（内部でDialogクラスを使用）
char* SelectFileDialog(const char* title, const char* filter, HWND hWnd, bool multiSelect)
{
	static char filepath[MAX_PATH] = "";
	if (Dialog::OpenFileName(filepath, MAX_PATH, filter, title, hWnd, multiSelect) == DialogResult::OK) {
		return filepath;
	}
	return nullptr;
}

// ディレクトリ選択ダイアログを表示し、選択されたディレクトリパスを返す（内部でDialogクラスを使用）
char* SelectDirectoryDialog(const char* title, HWND hWnd)
{
	static char directoryPath[MAX_PATH] = "";
	if (Dialog::SelectDirectoryName(directoryPath, MAX_PATH, title, hWnd) == DialogResult::OK) {
		return directoryPath;
	}
	return nullptr;
}