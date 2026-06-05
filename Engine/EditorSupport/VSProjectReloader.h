#pragma once
#include <string>

class VSProjectReloader
{
public:
	// 指定された .vcxproj ファイルを Visual Studio にリロードさせる
	// これにより、外部で .vcxproj を編集した際に VS 側の変更を反映させることができる
	static bool ReloadProject(const std::wstring& vcxprojPath);
};

