#pragma once
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include "ParseInfo.h"

// ============================================================
//  型マッピングエントリ
// ============================================================

enum class TypeSource
{
    Primitive, // ulong など C# プリミティブに直接マップ
    Existing,  // C# 側に既存定義がある (Vector3 など)
    Generate,  // このツールで生成する (enum/struct)
};

struct TypeMapping
{
    std::string csType;   // C# での型名
    TypeSource  source = TypeSource::Primitive;
    std::string marshalAttr; // [MarshalAs(...)] が必要な場合
};

// FileInfo リストから C_ENUM で収集された enum 名のセットを構築
inline std::unordered_set<std::string> BuildKnownEnums(const std::vector<FileInfo>& files)
{
    std::unordered_set<std::string> result;
    for (const auto& file : files)
        for (const auto& e : file.enums)
            result.insert(e.name);
    return result;
}

// enum かどうかを判定
// knownEnums: パーサーが収集した C_ENUM 付き enum 名
// typeMap   : type_map.json で明示登録されたもの（外部ライブラリの enum など）
inline bool IsEnumType(
    const std::string& cppType,
    const std::unordered_set<std::string>& knownEnums,
    const std::unordered_map<std::string, TypeMapping>& typeMap)
{
    if (knownEnums.count(cppType)) return true;
    auto it = typeMap.find(cppType);
    return (it != typeMap.end() && it->second.source == TypeSource::Generate);
}