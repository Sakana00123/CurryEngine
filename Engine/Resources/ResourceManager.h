#pragma once
#include "Resource.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <filesystem>
#include "Engine/Core/EnginePaths.h"

class ResourceManager
{
public:
	// 初期化
	static void Initialize();

	// 終了処理
	static void Finalize();

	// リソースの監視登録（ホットリロード用）
	static void Register(const std::string& filePath, std::shared_ptr<Resource> resource);

	// 指定した型でリソースをロード
	template<typename T>
	static std::shared_ptr<T> Load(const std::string& path)
	{
		// 新しいリソースを作成
		std::shared_ptr<Resource> resource = std::make_shared<T>();

		std::filesystem::path filePath(path);
#ifndef _DEBUG
		// リリースビルドではCSOファイルのみ読み込む
		if (filePath.extension() == ".hlsl")
		{
			// CSOファイルのパスを生成
			filePath = std::filesystem::path(EnginePaths::ShadersDataDir) / (filePath.stem().string() + ".cso");
		}
#endif // _DEBUG

		if (resource->LoadFromFile(filePath.string())) {
			// ロード成功したら管理に追加
			Register(filePath.string(), resource);
			return std::dynamic_pointer_cast<T>(resource);
		}
		return nullptr; // ロード失敗
	}

	// 指定した型でリソースをロード（すでにロードされている場合は既存のリソースを返す）
	template<typename T>
	static std::shared_ptr<T> GetOrLoad(const std::string& path)
	{
		// すでにロードされているか確認
		auto it = _resources.find(path);
		if (it != _resources.end()) {
			return std::dynamic_pointer_cast<T>(it->second); // 既存のリソースを返す
		}
		// 新しいリソースを作成
		std::shared_ptr<Resource> resource = std::make_shared<T>();

		std::filesystem::path filePath(path);
#ifndef _DEBUG
		// リリースビルドではCSOファイルのみ読み込む
		if (filePath.extension() == ".hlsl")
		{
			// CSOファイルのパスを生成
			filePath = std::filesystem::path(EnginePaths::ShadersDataDir) / (filePath.stem().string() + ".cso");
		}
#endif // _DEBUG

		if (resource->LoadFromFile(filePath.string())) {
			// ロード成功したら管理に追加
			Register(filePath.string(), resource);
			return std::dynamic_pointer_cast<T>(resource);
		}
		return nullptr; // ロード失敗
	}

	// リソースのロード（すでにロードされている場合は既存のリソースを返す）
	static std::shared_ptr<Resource> Load(const std::string& path);

	// リソースの取得（ロードされていない場合は nullptr を返す）
	static std::shared_ptr<Resource> Get(const std::string& path);

	// 指定した型でリソースを取得（ロードされていない場合は nullptr を返す）
	template<typename T>
	static std::shared_ptr<T> GetAs(const std::string& path) {
		return std::dynamic_pointer_cast<T>(Get(path));
	}

	// シェーダーのロード
	template<typename T>
	static std::shared_ptr<T> LoadShader(const std::string& name) {
#ifdef _DEBUG
		std::filesystem::path filePath = std::filesystem::path(EnginePaths::ShaderSourceDir) / (name + ".hlsl");
#else
		std::filesystem::path filePath = std::filesystem::path(EnginePaths::ShadersDataDir) / (name + ".cso");
#endif // _DEBUG
		return Load<T>(filePath.string());
	}

	// シェーダーのロード（すでにロードされている場合は既存のリソースを返す）
	template<typename T>
	static std::shared_ptr<T> GetOrLoadShader(const std::string& name) {
#ifdef _DEBUG
		std::filesystem::path filePath = std::filesystem::path(EnginePaths::ShaderSourceDir) / (name + ".hlsl");
#else
		std::filesystem::path filePath = std::filesystem::path(EnginePaths::ShadersDataDir) / (name + ".cso");
#endif // _DEBUG
		return GetOrLoad<T>(filePath.string());
	}

	// シェーダーの取得（ロードされていない場合は nullptr を返す）
	template<typename T>
	static std::shared_ptr<T> GetShader(const std::string& name) {
#ifdef _DEBUG
		std::filesystem::path filePath = std::filesystem::path(EnginePaths::ShaderSourceDir) / (name + ".hlsl");
#else
		std::filesystem::path filePath = std::filesystem::path(EnginePaths::ShadersDataDir) / (name + ".cso");
#endif // _DEBUG
		return GetAs<T>(filePath.string());
	}


	// 指定した型のすべてのリソースを取得
	template<typename T>
	static std::vector<std::shared_ptr<T>> GetResourcesOfType() {
		std::vector<std::shared_ptr<T>> result;
		for (const auto& [path, resource] : _resources) {
			if (auto casted = std::dynamic_pointer_cast<T>(resource)) {
				result.push_back(casted);
			}
		}
		return result;
	}

	// ロードされているシェーダーパスの一覧を取得
	static std::vector<std::string> GetShaderPaths();

	// リソースのリロード
	static void Reload(const std::string& path);

	// リソースのアンロード
	static void Unload(const std::string& path);

	// 全リソースのアンロード
	static void UnloadAll();

	// ホットリロード用の更新処理
	static void Update();

	// ホットリロード用の更新処理
	static void UpdateHotReload();

	// シェーダーフォルダ内のすべてのシェーダーファイルを読み込み
	static void LoadAllShaders();

	// テクスチャフォルダ内のすべてのテクスチャファイルを読み込み
	static void LoadAllTextures();

	// シェーダー名のキャッシュを更新
	static void UpdateShaderNames();
private:
	// ファイルの最終更新日時を管理（ホットリロード用）
	static inline std::unordered_map<std::string, std::filesystem::file_time_type> _files;

	// パスをキーにしてリソースを管理
	static inline std::unordered_map<std::string, std::shared_ptr<Resource>> _resources;

	static inline std::vector<std::string> shaderPaths; // ロードされているシェーダーパスのキャッシュ
};