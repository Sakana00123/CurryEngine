#pragma once
#include <unordered_map>
#include "Engine/Core/ObjectId.h"

class ObjectManager;

/**
 * @file
 * @brief シーン読み込み時に旧形式のIDを新形式に正規化するクラス。
 * @details Scene::Deserialize() の末尾から呼び出すことを想定しています。
 *          旧IDの判定は ObjectId::IsLegacy() に委譲するため、
 *          判定ロジックが変わっても本クラスの修正は不要です。
 */
class SceneMigrator
{
public:
    /** old → new の対応表。*/
    using RemapTable = std::unordered_map<ObjectId, ObjectId>;

    /**
     * @brief ObjectManager 内の全オブジェクトを走査し、
     *        旧IDを新IDに差し替えます。
     * @param objectManager 対象の ObjectManager。
     * @return 実際に差し替えが発生した場合は true。
     * @details 旧IDが一件もなければ即リターンします。
     */
    static bool Migrate(ObjectManager* objectManager);

private:
    /**
     * @brief 旧IDを持つオブジェクトを検出し、
     *        新IDを割り当てながら変換テーブルを構築します。
     *        この段階でオブジェクト本体の id も新IDに更新します。
     */
    static RemapTable BuildRemapTable(ObjectManager* objectManager);

    /**
     * @brief 変換テーブルに従って parentId / pendingParentID を書き換えます。
     */
    static void ApplyRemap(ObjectManager* objectManager,
                           const RemapTable& remap);
};
