#include "pch.h"
#include "SceneMigrator.h"

#include "Engine/Core/ObjectManager.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Editor/Console.h"

// -----------------------------------------------------------------------
// public
// -----------------------------------------------------------------------

bool SceneMigrator::Migrate(ObjectManager* objectManager)
{
    // ① 旧IDを検出し、オブジェクト本体のIDを新IDに更新しながら変換表を作る
    RemapTable remap = BuildRemapTable(objectManager);

    if (remap.empty())
        return false; // 旧IDが一件もなければ何もしない

    // ② 変換表を使って親子参照を書き換える
    ApplyRemap(objectManager, remap);

    LOG_INFO("[SceneMigrator] Migrated " +
                 std::to_string(remap.size()) + " legacy ID(s).");
    return true;
}

// -----------------------------------------------------------------------
// private
// -----------------------------------------------------------------------

SceneMigrator::RemapTable
SceneMigrator::BuildRemapTable(ObjectManager* objectManager)
{
    RemapTable remap;

    for (auto& obj : objectManager->objects)
    {
        if (!obj) continue;

        if (obj->id.IsLegacy())
        {
            ObjectId newId = ObjectId::Generate();
            remap[obj->id] = newId;  // 旧ID → 新ID を記録
            obj->id = newId;         // Object::id は public なので直接書き換え
        }
    }

    return remap;
}

void SceneMigrator::ApplyRemap(ObjectManager* objectManager,
                                const RemapTable& remap)
{
    for (auto& obj : objectManager->objects)
    {
        if (!obj) continue;

        // --- parentId の差し替え ---
        if (obj->parentId.IsValid())
        {
            auto it = remap.find(obj->parentId);
            if (it != remap.end())
                obj->parentId = it->second;
        }

        // --- pendingParentID の差し替え ---
        // デシリアライズ直後で親がまだ解決されていないケースをカバーする
        if (obj->pendingParentID.IsValid())
        {
            auto it = remap.find(obj->pendingParentID);
            if (it != remap.end())
                obj->pendingParentID = it->second;
        }
    }
}
