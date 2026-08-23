#pragma once
#include <string>
#include <memory>
#include <atomic>
#include "AssetId.h"

class Resource {
public:
	Resource() = default;
    virtual ~Resource() = default;

    // ファイルからロード
    virtual bool LoadFromFile(const std::string& path) = 0;

	// アセットIDからロード
    virtual bool Load(const CurryEngine::Resources::AssetId& assetId);

    // リロード用（ホットリロード対応）
    virtual bool Reload() { return LoadFromFile(_path); }

    // リソースのパス取得
    const std::string& GetPath() const { return _path; }

    // 参照カウント
    void AddRef() { ++_refCount; }
    void ReleaseRef() { --_refCount; }
    int RefCount() const { return _refCount; }

protected:
    std::string _path;

private:
    std::atomic<int> _refCount{ 0 };
};
