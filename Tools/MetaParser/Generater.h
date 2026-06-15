#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "ParseInfo.h"
#include "GeneraterUtils.h"

class Generater
{
public:
	Generater(const std::string& outputDir);

	// クラス情報からコードを生成(ignoreClasses に指定されたクラスはcppファイルは生成しないが、ヘッダーファイルは生成する)
	void Generate(const std::vector<FileInfo>& files, const std::vector<std::string>& ignoreClasses = {});
private:
	std::unordered_set<std::string> knownEnums;
	std::unordered_map<std::string, TypeMapping> typeMap;
	std::string outputDirectory;
	std::string headerDir = "Reflection"; // ヘッダーファイルの出力サブディレクトリ
	std::string sourceDir = "Interop"; // ソースファイルの出力サブディレクトリ
	std::string enumDir = "Reflection/Enums"; // 列挙型のヘッダーファイルの出力サブディレクトリ
	std::string structDir = "Reflection/Structs"; // 構造体のヘッダーファイルの出力サブディレクトリ
	
	void GenerateHeader(const ClassInfo& info, const std::string& outPath, const std::string& includePath);
	void GenerateSource(const ClassInfo& info, const std::string& outPath, const std::string& includePath, const std::string& relativeSolutionPath = "");
	void GenerateEnum(const EnumInfo& info, const std::string& outPath, const std::string& includePath);
	void GenerateStruct(const StructInfo& info, const std::string& outPath, const std::string& includePath);
};