#pragma once
#include <string>
#include <vector>

struct AttributeInfo
{
	std::string name;
	std::vector<std::string> args;
};

struct FieldInfo
{
	std::string type;
	std::string name;
	std::vector<AttributeInfo> attributes;
};

struct ParameterInfo
{
	std::string type;
	std::string name;
	std::string defaultValue; // デフォルト引数の値（なければ空文字列）
};

struct MethodInfo
{
	std::string returnType;
	std::string name;
	std::vector<ParameterInfo> parameters;
	std::vector<AttributeInfo> attributes;
};

struct ClassInfo
{
	std::string name;
	std::vector<std::string> bases;
	std::vector<FieldInfo> fields;
	std::vector<MethodInfo> methods;
	bool reflect = false;
};

struct EnumValueInfo
{
	std::string name;
	int         value = 0;
	bool        hasExplicitValue = false; // 値が明示されているか
};

struct EnumInfo
{
	std::string name;
	std::string underlyingType = "int"; // enum class : uint など
	bool        isClass = false; // enum class かどうか
	std::vector<EnumValueInfo> values;
};

struct StructInfo
{
	std::string name;
	std::vector<FieldInfo> fields;
};

struct FileInfo
{
	std::string path;
	std::vector<ClassInfo> classes;
	std::vector<EnumInfo> enums;
	std::vector<StructInfo> structs;
};