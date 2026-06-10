#include "Parser.h"
#include <regex>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

#include "../../External/tinygltf-release/json.hpp"
#include <windows.h>

// ============================================================
//  内部ユーティリティ（既存）
// ============================================================

static std::string ShiftJisToUtf8(const std::string& sjis)
{
    int wlen = MultiByteToWideChar(CP_ACP, 0, sjis.c_str(), -1, nullptr, 0);
    std::wstring wide(wlen, L'\0');
    MultiByteToWideChar(CP_ACP, 0, sjis.c_str(), -1, wide.data(), wlen);

    int ulen = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string utf8(ulen, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, utf8.data(), ulen, nullptr, nullptr);
    return utf8;
}

static std::string RemoveComments(const std::string& s)
{
    std::string out = s;
    size_t pos = out.find("//");
    while (pos != std::string::npos)
    {
        size_t end = out.find('\n', pos + 2);
        if (end == std::string::npos) break;
        out.erase(pos, end - pos);
        pos = out.find("//");
    }
    size_t start = out.find("/*");
    while (start != std::string::npos)
    {
        size_t end = out.find("*/", start + 2);
        if (end == std::string::npos) break;
        out.erase(start, end - start + 2);
        start = out.find("/*");
    }
    return out;
}

static void Trim(std::string& s)
{
    s.erase(0, s.find_first_not_of(" \t\n\r"));
    s.erase(s.find_last_not_of(" \t\n\r") + 1);
}

static std::string ExtractMacroArgs(const std::string& text, size_t macroPos)
{
    int depth = 0;
    size_t open = text.find('(', macroPos);
    if (open == std::string::npos) return "";
    size_t pos = open;
    while (pos < text.size())
    {
        switch (text[pos])
        {
        case '(': depth++; break;
        case ')': depth--; break;
        default: break;
        }
        if (depth == 0) break;
        pos++;
    }
    if (depth != 0) return "";
    return text.substr(open + 1, pos - open - 1);
}

static std::vector<AttributeInfo> ParseAttributes(const std::string& attrStr)
{
    std::vector<AttributeInfo> attrs;
    if (attrStr.empty()) return attrs;

    std::vector<std::string> tokens;
    int depth = 0;
    std::string cur;
    for (char c : attrStr)
    {
        if (c == '(') depth++;
        else if (c == ')') depth--;
        if (c == ',' && depth == 0)
        {
            Trim(cur);
            if (!cur.empty()) tokens.push_back(cur);
            cur.clear();
        }
        else cur += c;
    }
    Trim(cur);
    if (!cur.empty()) tokens.push_back(cur);

    std::regex attrRegex(R"(([\w:]+)(?:\((.*)\))?)");
    for (auto& token : tokens)
    {
        std::smatch m;
        if (!std::regex_search(token, m, attrRegex)) continue;
        AttributeInfo a;
        a.name = m[1].str();
        while (a.name.find("::") != std::string::npos)
            a.name = a.name.substr(a.name.find("::") + 2);

        if (m[2].matched)
        {
            std::stringstream ss(m[2].str());
            std::string arg;
            while (std::getline(ss, arg, ','))
            {
                Trim(arg);
                if (arg.size() >= 2 && arg.front() == '"' && arg.back() == '"')
                    arg = arg.substr(1, arg.size() - 2);
                a.args.push_back(arg);
            }
        }
        attrs.push_back(a);
    }
    return attrs;
}

// ============================================================
//  ParseDirectory / ParseFile
// ============================================================

std::vector<FileInfo> Parser::ParseDirectory(const std::string& dirPath)
{
    std::vector<FileInfo> result;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath))
    {
        if (!entry.is_regular_file()) continue;
        auto path = entry.path();
        if (path.extension() == ".h" || path.extension() == ".hpp")
            result.push_back(ParseFile(path.string()));
    }
    return result;
}

FileInfo Parser::ParseFile(const std::string& path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) return {};

    std::string content((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());
    content = ShiftJisToUtf8(content);

    std::string cleaned = RemoveComments(content);

    FileInfo fileInfo;
    fileInfo.path = path;
    for (auto& c : ExtractClasses(cleaned))
        fileInfo.classes.push_back(c);
    for (auto& e : ExtractEnums(cleaned))
        fileInfo.enums.push_back(e);
    for (auto& s : ExtractStructs(cleaned))
        fileInfo.structs.push_back(s);
    return fileInfo;
}

// ============================================================
//  ExtractClasses
// ============================================================

std::vector<ClassInfo> Parser::ExtractClasses(const std::string& text)
{
    std::vector<ClassInfo> classes;

    std::regex classRegex(
        R"((class)\s+(\w+)\s*((?:\s*:\s*(?:public|protected|private)\s+\w+(?:\s*,\s*(?:public|protected|private)\s+\w+)*)?)?\s*(?://[^\n]*)?\s*\{)"
    );
    // struct は ExtractStructs が担当するため class のみにマッチ

    auto begin = std::sregex_iterator(text.begin(), text.end(), classRegex);
    auto end = std::sregex_iterator();

    for (auto it = begin; it != end; ++it)
    {
        ClassInfo info;
        info.name = (*it)[2].str();

        std::string inheritStr = (*it)[3].str();
        std::regex baseRegex(R"((?:public|protected|private)\s+(\w+))");
        auto bIt = std::sregex_iterator(inheritStr.begin(), inheritStr.end(), baseRegex);
        for (; bIt != std::sregex_iterator(); ++bIt)
            info.bases.push_back((*bIt)[1].str());

        size_t classPos = it->position(0);
        auto [blockStart, blockEnd] = FindClassBlock(text, classPos);

        info.reflect = ContainsReflectMacro(text, blockStart, blockEnd, "C_REFLECT");
        if (!info.reflect)
        {
            std::cout << "Skipping class (no C_REFLECT): " << info.name << "\n";
            continue;
        }

        std::cout << "Found class: " << info.name << "\n";
        ExtractFields(text, classPos, info);
        ExtractMethods(text, classPos, info);

        WriteJson(info, outputDirectory + "/" + info.name + ".json");
        classes.push_back(info);
    }
    return classes;
}

void Parser::ExtractFields(const std::string& text, size_t classPos, ClassInfo& info)
{
    auto [blockStart, blockEnd] = FindClassBlock(text, classPos);
    size_t pos = blockStart;

    while (true)
    {
        pos = text.find("C_PROPERTY", pos);
        if (pos == std::string::npos || pos > blockEnd) break;

        std::string attrStr = ExtractMacroArgs(text, pos);
        auto attributes = ParseAttributes(attrStr);

        size_t lineStart = text.find('\n', pos);
        if (lineStart == std::string::npos) break;
        lineStart++;
        size_t lineEnd = text.find(';', lineStart);
        if (lineEnd == std::string::npos) break;

        std::string line = text.substr(lineStart, lineEnd - lineStart);
        Trim(line);

        std::regex fieldRegex(R"(([A-Za-z0-9_:<>]+)\s+([A-Za-z0-9_]+)\s*)");
        std::smatch m;
        if (!std::regex_search(line, m, fieldRegex))
        {
            std::cout << "  Warning: Could not parse field line: " << line << "\n";
            pos = lineEnd;
            continue;
        }

        FieldInfo field;
        field.type = m[1].str();
        field.name = m[2].str();
        field.attributes = attributes;

		// Getter/Setter属性があれば customGetter/customSetter にメソッド名をセット
        for (const auto& attr : attributes)
        {
            if (attr.name == "Getter" && !attr.args.empty())
                field.customGetter = attr.args[0];
            else if (attr.name == "Setter" && !attr.args.empty())
                field.customSetter = attr.args[0];
		}

        info.fields.push_back(field);
        std::cout << "  Field: " << field.type << " " << field.name << "\n";

        pos = lineEnd;
    }
}

void Parser::ExtractMethods(const std::string& text, size_t classPos, ClassInfo& info)
{
    auto [blockStart, blockEnd] = FindClassBlock(text, classPos);
    size_t pos = blockStart;

    while (true)
    {
        pos = text.find("C_FUNCTION", pos);
        if (pos == std::string::npos || pos > blockEnd) break;

        size_t lineStart = text.find('\n', pos);
        if (lineStart == std::string::npos) break;
        lineStart++;

		// ; か { のどちらか早い方を行末とみなす（関数宣言だけでなく定義もあるため）
		size_t semicolonPos = text.find(';', lineStart);
		size_t bracePos = text.find('{', lineStart);

        size_t lineEnd;
        bool isInlineImpl = false;

        if (bracePos != std::string::npos && bracePos < semicolonPos)
        {
            lineEnd = bracePos;
            isInlineImpl = true;
        }
        else if (semicolonPos != std::string::npos)
        {
            lineEnd = semicolonPos;
        }
        else break;

        std::string line = text.substr(lineStart, lineEnd - lineStart);
        Trim(line);

        // const メソッドかどうかを判定
        std::regex constMethodRegex(R"(\)\s*const\s*$)");
        bool isConst = std::regex_search(line, constMethodRegex);

        std::regex methodRegex(R"(([A-Za-z0-9_:<>]+)\s+([A-Za-z0-9_]+)\s*\((.*)\))");
        std::smatch m;
        if (!std::regex_search(line, m, methodRegex))
        {
            std::cout << "  Warning: Could not parse method line: " << line << "\n";
            pos = lineEnd;
            continue;
        }

        MethodInfo method;
        method.returnType = m[1].str();
        method.name = m[2].str();
		method.isConst = isConst;

        std::string argsStr = m[3].str();
        // 例: "ForceMode mode = ForceMode::Force"  →  type="ForceMode", name="mode", default="ForceMode::Force"
        // 例: "float value = 0.0f"                 →  type="float",     name="value", default="0.0f"
		// 例: "const Vector3& position"            →  type="const Vector3&", name="position", default=""
        //std::regex argRegex (R"(([A-Za-z0-9_:<>]+(?:\s*[*&])?)\s+([A-Za-z0-9_]+)\s*(?:=\s*([^,]+))?)");
        std::regex argRegex (R"(((?:const\s+)?[A-Za-z0-9_:<>]+\s*[&*]*)\s+([A-Za-z0-9_]+)\s*(?:=\s*([^,)]+))?)");
        auto aIt = std::sregex_iterator(argsStr.begin(), argsStr.end(), argRegex);
        for (; aIt != std::sregex_iterator(); ++aIt)
        {
            //method.parameters.emplace_back((*aIt)[1].str(), (*aIt)[2].str());
			ParameterInfo param;
			param.type = (*aIt)[1].str();
			param.name = (*aIt)[2].str();
			if ((*aIt)[3].matched)
            {
                param.defaultValue = (*aIt)[3].str();
				Trim(param.defaultValue);
            }
            method.parameters.push_back(param);
        }

        info.methods.push_back(method);
        std::cout << "  Method: " << method.returnType << " " << method.name << "\n";

        if (isInlineImpl)
        {
			auto [implStart, implEnd] = FindClassBlock(text, bracePos - 1);
			pos = (implEnd != std::string::npos) ? implEnd + 1 : lineEnd;
        }
        else
        {
            pos = lineEnd;
        }
    }
}

// ============================================================
//  ExtractEnums
//  C_ENUM() マクロの直後にある enum / enum class を抽出する
// ============================================================

std::vector<EnumInfo> Parser::ExtractEnums(const std::string& text)
{
    std::vector<EnumInfo> enums;
    size_t pos = 0;

    while (true)
    {
        pos = text.find("C_ENUM", pos);
        if (pos == std::string::npos) break;

        // C_ENUM(...) の閉じ括弧の次から enum を探す
        size_t afterMacro = text.find(')', pos);
        if (afterMacro == std::string::npos) break;
        afterMacro++;

        // 空白・改行をスキップして enum / enum class を探す
        // 正規表現: enum (class)? Name (: underlyingType)? {
        std::string sub = text.substr(afterMacro);
        std::regex enumRegex(
            R"(\benum\s+(class\s+)?(\w+)\s*(?::\s*(\w+))?\s*\{)"
        );
        std::smatch m;
        if (!std::regex_search(sub, m, enumRegex))
        {
            std::cout << "Warning: C_ENUM found but no enum follows at pos " << pos << "\n";
            pos = afterMacro;
            continue;
        }

        EnumInfo info;
        info.isClass = m[1].matched;
        info.name = m[2].str();
        info.underlyingType = m[3].matched ? m[3].str() : "int";

        std::cout << "Found enum: " << info.name
            << (info.isClass ? " (class)" : "")
            << " : " << info.underlyingType << "\n";

        // ブロック内を取得してメンバーを列挙
        size_t enumStart = afterMacro + m.position(0);
        auto [blockStart, blockEnd] = FindClassBlock(text, enumStart);
        if (blockStart == std::string::npos)
        {
            pos = afterMacro;
            continue;
        }

        std::string body = text.substr(blockStart + 1, blockEnd - blockStart - 1);

        // "Name = Value," または "Name," をパース
        std::regex valueRegex(R"(\b([A-Za-z_]\w*)\s*(?:=\s*(-?\d+))?\s*[,}])");
        int nextValue = 0;
        auto vIt = std::sregex_iterator(body.begin(), body.end(), valueRegex);
        for (; vIt != std::sregex_iterator(); ++vIt)
        {
            EnumValueInfo v;
            v.name = (*vIt)[1].str();
            if ((*vIt)[2].matched)
            {
                v.value = std::stoi((*vIt)[2].str());
                v.hasExplicitValue = true;
                nextValue = v.value + 1;
            }
            else
            {
                v.value = nextValue++;
            }
            std::cout << "  EnumValue: " << v.name << " = " << v.value << "\n";
            info.values.push_back(v);
        }

        WriteJson(info, outputDirectory + "/" + info.name + ".json");
        enums.push_back(info);

        pos = blockEnd + 1;
    }
    return enums;
}

// ============================================================
//  ExtractStructs
//  C_STRUCT() マクロの直後にある struct を抽出する
// ============================================================

std::vector<StructInfo> Parser::ExtractStructs(const std::string& text)
{
    std::vector<StructInfo> structs;
    size_t pos = 0;

    while (true)
    {
        pos = text.find("C_STRUCT", pos);
        if (pos == std::string::npos) break;

        size_t afterMacro = text.find(')', pos);
        if (afterMacro == std::string::npos) break;
        afterMacro++;

        std::string sub = text.substr(afterMacro);
        std::regex structRegex(R"(\bstruct\s+(\w+)\s*\{)");
        std::smatch m;
        if (!std::regex_search(sub, m, structRegex))
        {
            std::cout << "Warning: C_STRUCT found but no struct follows at pos " << pos << "\n";
            pos = afterMacro;
            continue;
        }

        StructInfo info;
        info.name = m[1].str();
        std::cout << "Found struct: " << info.name << "\n";

        size_t structStart = afterMacro + m.position(0);
        auto [blockStart, blockEnd] = FindClassBlock(text, structStart);
        if (blockStart == std::string::npos)
        {
            pos = afterMacro;
            continue;
        }

        ExtractStructFields(text, blockStart, blockEnd, info);

        WriteJson(info, outputDirectory + "/" + info.name + ".json");
        structs.push_back(info);

        pos = blockEnd + 1;
    }
    return structs;
}

void Parser::ExtractStructFields(const std::string& text, size_t blockStart, size_t blockEnd, StructInfo& info)
{
    std::string body = text.substr(blockStart + 1, blockEnd - blockStart - 1);

    // 1行ずつ処理
    std::istringstream stream(body);
    std::string line;
    while (std::getline(stream, line))
    {
        Trim(line);
        if (line.empty()) continue;

        // メソッド・演算子・コンストラクタ・デストラクタを除外
        if (line.find('(') != std::string::npos) continue;
        if (line.find('~') != std::string::npos) continue;
        if (line.find("operator") != std::string::npos) continue;

        // セミコロンで終わる行のみ対象
        if (line.back() != ';') continue;
        line.pop_back(); // セミコロン除去
        Trim(line);

        // ポインタ・参照修飾子を除去（型名の正規化）
        // "float* x" → "float x" / "const Vector3& v" → "Vector3 v"
        // const も除去
        std::string normalized = line;
        // const を除去
        std::regex constRegex(R"(\bconst\b)");
        normalized = std::regex_replace(normalized, constRegex, "");
        // * & を除去
        for (char& c : normalized)
            if (c == '*' || c == '&') c = ' ';
        Trim(normalized);
        // 連続スペースを1つに
        normalized = std::regex_replace(normalized, std::regex(R"(\s+)"), " ");

        // "Type Name" または "Type Name = defaultValue" をパース
        // デフォルト値部分は無視
        std::regex fieldRegex(R"(^([A-Za-z0-9_:<>]+)\s+([A-Za-z0-9_]+)(?:\s*=.*)?$)");
        std::smatch m;
        if (!std::regex_match(normalized, m, fieldRegex)) continue;

        std::string type = m[1].str();
        std::string name = m[2].str();

        // アクセス指定子・キーワードを除外
        static const std::vector<std::string> keywords = {
            "public", "private", "protected", "static", "virtual",
            "inline", "explicit", "friend", "typedef", "using",
            "return", "void"
        };
        bool skip = false;
        for (const auto& kw : keywords)
            if (type == kw || name == kw) { skip = true; break; }
        if (skip) continue;

        FieldInfo f;
        f.type = type;
        f.name = name;
        info.fields.push_back(f);
        std::cout << "  StructField: " << f.type << " " << f.name << "\n";
    }
}

// ============================================================
//  FindClassBlock / ContainsReflectMacro
// ============================================================

std::pair<size_t, size_t> Parser::FindClassBlock(const std::string& text, size_t classPos)
{
    size_t braceOpenPos = text.find('{', classPos);
    if (braceOpenPos == std::string::npos) return { std::string::npos, std::string::npos };

    size_t braceClosePos = braceOpenPos;
    int braceCount = 1;
    while (braceCount > 0)
    {
        braceClosePos++;
        if (braceClosePos >= text.size()) return { braceOpenPos, std::string::npos };
        if (text[braceClosePos] == '{') braceCount++;
        else if (text[braceClosePos] == '}') braceCount--;
    }
    return { braceOpenPos, braceClosePos };
}

bool Parser::ContainsReflectMacro(const std::string& text, size_t begin, size_t end, const std::string& macro)
{
    size_t pos = text.find(macro, begin);
    return (pos != std::string::npos && pos < end);
}

// ============================================================
//  WriteJson overloads
// ============================================================

void Parser::WriteJson(const ClassInfo& info, const std::string& outPath)
{
    nlohmann::json j;
    j["kind"] = "class";
    j["name"] = info.name;
    j["bases"] = info.bases;

    for (const auto& f : info.fields)
    {
        auto& fj = j["fields"].emplace_back();
        fj["type"] = f.type;
        fj["name"] = f.name;
        nlohmann::json attrArr = nlohmann::json::array();
        for (const auto& attr : f.attributes)
        {
            auto& aj = attrArr.emplace_back();
            aj["name"] = attr.name;
            aj["args"] = attr.args;
        }
        fj["attributes"] = attrArr;
    }

    for (const auto& m : info.methods)
    {
        auto& mj = j["methods"].emplace_back();
        mj["returnType"] = m.returnType;
        mj["name"] = m.name;
        nlohmann::json paramArr = nlohmann::json::array();
        for (const auto& p : m.parameters)
        {
            auto& pj = paramArr.emplace_back();
            pj["type"] = p.type;
            pj["name"] = p.name;
			pj["defaultValue"] = p.defaultValue;
        }
        mj["parameters"] = paramArr;
    }

    std::cout << "Writing JSON: " << outPath << "\n";
    std::ofstream(outPath) << j.dump(2);
}

void Parser::WriteJson(const EnumInfo& info, const std::string& outPath)
{
    nlohmann::json j;
    j["kind"] = "enum";
    j["name"] = info.name;
    j["underlyingType"] = info.underlyingType;
    j["isClass"] = info.isClass;

    for (const auto& v : info.values)
    {
        auto& vj = j["values"].emplace_back();
        vj["name"] = v.name;
        vj["value"] = v.value;
        vj["hasExplicitValue"] = v.hasExplicitValue;
    }

    std::cout << "Writing JSON: " << outPath << "\n";
    std::ofstream(outPath) << j.dump(2);
}

void Parser::WriteJson(const StructInfo& info, const std::string& outPath)
{
    nlohmann::json j;
    j["kind"] = "struct";
    j["name"] = info.name;

    for (const auto& f : info.fields)
    {
        auto& fj = j["fields"].emplace_back();
        fj["type"] = f.type;
        fj["name"] = f.name;
    }

    std::cout << "Writing JSON: " << outPath << "\n";
    std::ofstream(outPath) << j.dump(2);
}