#pragma once
#include <memory>
#include <vector>
#include "GameObject.h"

class PersistentObjectManager {
public:
    static void Register(const std::shared_ptr<GameObject>& obj) {
        persistentObjects.emplace_back(obj);
    }
    static void Unregister(const std::shared_ptr<GameObject>& obj) {
        erases.emplace_back(obj);
    }
    // アプリ終了時などに呼ぶ
    static void Clear() {
        persistentObjects.clear();
    }
    static const std::vector<std::shared_ptr<GameObject>>& GetObjects(bool sortAndErase = false) {
        if (sortAndErase) {
            if (!erases.empty()) {
                persistentObjects.erase(std::remove_if(persistentObjects.begin(), persistentObjects.end(),
                    [&](const auto& obj) {
                        return std::find(erases.begin(), erases.end(), obj) != erases.end();
                    }),
                    persistentObjects.end());
                erases.clear();
            }
            //優先度でソート
            std::sort(persistentObjects.begin(), persistentObjects.end(),
                [](const std::shared_ptr<GameObject>& a, const std::shared_ptr<GameObject>& b) {
                    return a->priority < b->priority;
                });
        }
        return persistentObjects;
    }

private:
    static inline std::vector<std::shared_ptr<GameObject>> persistentObjects;
    static inline std::vector<std::shared_ptr<GameObject>> erases;
};