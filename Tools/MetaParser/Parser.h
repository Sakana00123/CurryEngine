#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include "ParseInfo.h"


class Parser
{
public:
	Parser(const std::string& outputDir = "./") : outputDirectory(outputDir) {}

	// --- ディレクトリ / ファイル単位のエントリポイント ---
	std::vector<FileInfo> ParseDirectory(const std::string& dirPath);
	FileInfo              ParseFile(const std::string& filePath);

	// --- class 抽出 ---
	std::vector<ClassInfo> ExtractClasses(const std::string& text);
	void ExtractFields(const std::string& text, size_t classPos, ClassInfo& info);
	void ExtractMethods(const std::string& text, size_t classPos, ClassInfo& info);

	// --- enum / struct 抽出 ---
	std::vector<EnumInfo>   ExtractEnums(const std::string& text);
	std::vector<StructInfo> ExtractStructs(const std::string& text);

	// --- ユーティリティ ---
	std::pair<size_t, size_t> FindClassBlock(const std::string& text, size_t classPos);
	bool ContainsReflectMacro(const std::string& text, size_t begin, size_t end, const std::string& macro = "C_REFLECT");

	// --- JSON 出力 ---
	void WriteJson(const ClassInfo& info, const std::string& outPath);
	void WriteJson(const EnumInfo& info, const std::string& outPath);
	void WriteJson(const StructInfo& info, const std::string& outPath);

private:
	std::string outputDirectory;

	// struct のフィールド抽出（class 用 ExtractFields を流用）
	void ExtractStructFields(const std::string& text, size_t blockStart, size_t blockEnd, StructInfo& info);
};