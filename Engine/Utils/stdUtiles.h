#pragma once
#include <Windows.h>
#include <string>
/**
 * @brief ワイド文字列を UTF-8 文字列へ変換します。
 * @param wstr 変換するワイド文字列。
 * @return UTF-8 へ変換した文字列。
 */
inline std::string WstringToString(const std::wstring& wstr) {
	int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	std::string str(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], sizeNeeded, NULL, NULL);
	return str;
}
/**
 * @brief UTF-8 文字列をワイド文字列へ変換します。
 * @param str 変換する UTF-8 文字列。
 * @return ワイド文字列へ変換した結果。
 */
inline std::wstring StringToWstring(const std::string& str) {
	int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	std::wstring wstr(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], sizeNeeded);
	return wstr;
}
