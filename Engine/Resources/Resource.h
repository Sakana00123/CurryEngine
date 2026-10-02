#pragma once
#include <string>
#include <memory>
#include <atomic>
#include "AssetId.h"

/** @brief Resource を表すクラスです。 */
class Resource {
public:
	/**
	 * @brief Resource を構築します。
	 */
	Resource() = default;
    /**
     * @brief Resource を破棄します。
     */
    virtual ~Resource() = default;

    // ファイルからロード
    /**
     * @brief LoadFromFile に対応する値を取得します。
     * @return 処理結果を返します。
     */
    virtual bool LoadFromFile(const std::string& path) = 0;

	// アセットIDからロード
    /**
     * @brief Load に対応する値を取得します。
     * @return 処理結果を返します。
     */
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
