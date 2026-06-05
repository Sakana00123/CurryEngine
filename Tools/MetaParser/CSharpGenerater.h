#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "ParseInfo.h"
#include "GeneraterUtils.h"

// ============================================================
//  CSharpGenerater
// ============================================================

class CSharpGenerater
{
public:
    // outputDir  : 生成ファイルの出力先
    // typeMapPath: type_map.json のパス
    // nmNamespace: NativeMethods の名前空間 (例: "CurryEngine.Interop")
    // csNamespace: ラッパークラスの名前空間 (例: "CurryEngine")
    CSharpGenerater(
        const std::string& outputDir,
        const std::string& typeMapPath,
        const std::string& nmNamespace = "CurryEngine.Interop",
        const std::string& csNamespace = "CurryEngine"
    );

    // FileInfo リストから全ファイルを生成
    void Generate(const std::vector<FileInfo>& files);

private:
    std::string outputDirectory;
    std::string nmNamespace; // NativeMethods の名前空間
    std::string csNamespace; // ラッパーの名前空間
    std::string dllConstant; // NativeMethods.Dll 参照文字列

	// 既知の enum 名のセット (C_ENUM で収集されたもの)
    std::unordered_set<std::string> knownEnums;
    // C++ 型名 → TypeMapping
    std::unordered_map<std::string, TypeMapping> typeMap;

    // C++ のデフォルト値表現を C# に変換するテーブル
    // 例: "ForceMode::Force" → "ForceMode.Force"
    //     "true" → "true"（そのまま）
    //     "nullptr" → "null"
    std::unordered_map<std::string, std::string> defaultValueMap = {
        { "nullptr", "null"    },
        { "true",    "true"    },
        { "false",   "false"   },
    };

    // --- 型マップ読み込み ---
    void LoadTypeMap(const std::string& path);

    // --- 型解決 ---
    // C++ 型名を C# 型名に変換。未登録なら警告して型名をそのまま返す
    std::string ResolveCsType(const std::string& cppType) const;

    // bool など MarshalAs が必要な型かチェックして属性文字列を返す ("" なら不要)
    std::string ResolveMarshalAttr(const std::string& cppType) const;

	// デフォルト引数の C++ 値を C# の値に変換 (未登録の値はそのまま返す)
	std::string ConvertDefaultValue(const std::string& cppDefault, const std::string& cppType) const;

    // --- 生成メソッド ---
    // NativeMethods.Xxx.g.cs (partial LibraryImport)
    void GenerateNativeMethods(const ClassInfo& info);

    // XxxComponent.g.cs (public sealed partial class)
    void GenerateWrapper(const ClassInfo& info);

    // XxxEnum.g.cs
    void GenerateEnum(const EnumInfo& info);

    // XxxStruct.g.cs
    void GenerateStruct(const StructInfo& info);

    // --- ヘルパー ---
    // メソッドの LibraryImport シグネチャ行を生成
    std::string BuildLibraryImportLine(const std::string& className, const MethodInfo& m, int indent) const;

    // フィールドの getter/setter 用 LibraryImport シグネチャを生成
    std::string BuildFieldImportLines(const std::string& className, const FieldInfo& f, int indent) const;

    // プロパティ実装（C# ラッパー側）
    std::string BuildPropertyImpl(const std::string& className, const FieldInfo& f, int indent) const;

    // メソッド実装（C# ラッパー側）
    std::string BuildMethodImpl(const std::string& className, const MethodInfo& m, int indent) const;

    std::string Indent(int n) const { return std::string(n * 4, ' '); }
};