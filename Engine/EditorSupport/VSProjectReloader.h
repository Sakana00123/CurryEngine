#pragma once
#include <string>

/** @brief VSProjectReloader を表すクラスです。 */
class VSProjectReloader
{
public:
	// 指定された .vcxproj ファイルを Visual Studio にリロードさせる
	// これにより、外部で .vcxproj を編集した際に VS 側の変更を反映させることができる
	/**
	 * @brief ReloadProject の処理を行います。
	 */
	static bool ReloadProject(const std::wstring& vcxprojPath);
};

