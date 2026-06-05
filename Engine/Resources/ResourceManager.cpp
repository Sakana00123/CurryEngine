#include "pch.h"
#include "ResourceManager.h"
#include "Engine/Resources/Shader.h" // Shaderクラスの定義が必要
#include "Engine/Resources/Texture.h" // Textureクラスの定義が必要
#include "Engine/Editor/Console.h"
#include "Engine/Core/Time.h"

// デバッグビルドとリリースビルドでシェーダーファイルの検索方法を切り替える
static std::string AdjustShaderPath(const std::string& path) {
	std::string findPath = path;
#ifdef _DEBUG
	// csoファイルだったら、hlslに変換して検索 
	auto fsPath = std::filesystem::path(path);
	if (fsPath.extension() == ".cso") {
		std::filesystem::path hlslPath = EnginePaths::ShaderSourceDir / fsPath.filename().replace_extension(".hlsl");
		findPath = hlslPath.string();
	}
#else
	// 本番環境では、csoファイルで検索
	auto fsPath = std::filesystem::path(path);
	if (fsPath.extension() == ".hlsl") {
		std::filesystem::path csoPath = EnginePaths::ShadersDataDir / fsPath.filename().replace_extension(".cso");
		findPath = csoPath.string();
	}
#endif // _DEBUG
	return findPath;
}


void ResourceManager::Initialize()
{
	_files.clear();
	_resources.clear();
	shaderPaths.clear();

//#ifdef _DEBUG
	// 初期化時にすべてのシェーダーファイルを読み込み
	LoadAllShaders();
//#endif // _DEBUG
}

void ResourceManager::Finalize()
{
	UnloadAll();
	_files.clear();
	shaderPaths.clear();
}

void ResourceManager::Register(const std::string& filePath, std::shared_ptr<Resource> resource)
{
	resource->AddRef();
	_resources[filePath] = resource;
	_files[filePath] = std::filesystem::last_write_time(filePath);// 最終更新日時を保存
	std::filesystem::path fsPath(filePath);
#ifdef _DEBUG
	if (fsPath.extension() == ".cso") {
		// csoファイルだったら、hlslに変換して登録
		std::filesystem::path hlslPath = EnginePaths::ShaderSourceDir / fsPath.filename().replace_extension(".hlsl");

		// hlslファイルも監視対象に追加
		if (std::filesystem::exists(hlslPath))
		{
			_resources[hlslPath.string()] = resource;
			_files[hlslPath.string()] = std::filesystem::last_write_time(hlslPath);
		}
	}
#endif // _DEBUG
}

std::shared_ptr<Resource> ResourceManager::Load(const std::string& path)
{
	// すでにロードされているか確認
	std::string findPath = AdjustShaderPath(path);
	auto it = _resources.find(findPath);
	if (it != _resources.end()) {
		return it->second; // 既存のリソースを返す
	}
	// 新しいリソースを作成
	std::shared_ptr<Resource> resource;
	std::filesystem::path fsPath(findPath);

#ifdef _DEBUG
	bool isShader = (fsPath.extension() == ".hlsl" || fsPath.extension() == ".cso");
#else 
	bool isShader = (fsPath.extension() == ".cso");
#endif // _DEBUG

	if (isShader) {
#ifdef _DEBUG
		std::filesystem::path hlslPath = EnginePaths::ShaderSourceDir / fsPath.filename();
		if (!std::filesystem::exists(hlslPath)) {
			Console::LogError("Shader file not found: " + hlslPath.string());
			return nullptr;
		}
#endif // _DEBUG


		// シェーダータイプの推定
		std::string stem = fsPath.stem().string(); // ファイル名（拡張子なし）
		std::string suffix = "";
		if (stem.size() >= 2) {
		    suffix = stem.substr(stem.size() - 2, 2);
		    std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);
		}
		if (suffix == "vs") {
			resource = std::make_shared<VertexShader>();
		} else if (suffix == "ps") {
			resource = std::make_shared<PixelShader>();
		} else if (suffix == "gs") {
			resource = std::make_shared<GeometryShader>();
		} else if (suffix == "cs") {
			resource = std::make_shared<ComputeShader>();
		} else {
			Console::LogError("Unsupported shader type: " + findPath);
			return nullptr;
		}
		if (resource->LoadFromFile(findPath)) {
			Register(findPath, resource);
			return resource;
		}
	}
	else if (fsPath.extension() == ".png" || fsPath.extension() == ".jpg" || fsPath.extension() == ".jpeg") {
		resource = std::make_shared<AssetTexture>();
		if (resource->LoadFromFile(path)) {
			Register(path, resource);
			return resource;
		}
	} else {
		Console::LogError("Unsupported resource type: " + path);
	}
	return nullptr; // ロード失敗
}

std::shared_ptr<Resource> ResourceManager::Get(const std::string& path)
{
	// すでにロードされているか確認
	auto it = _resources.find(AdjustShaderPath(path));
	if (it != _resources.end()) {
		return it->second;
	}
	return nullptr; // 見つからなかった場合
}

std::vector<std::string> ResourceManager::GetShaderPaths()
{
	return shaderPaths;
}

void ResourceManager::Reload(const std::string& path)
{
	auto it = _resources.find(path);
	if (it != _resources.end()) {
		it->second->Reload();
	}
}

void ResourceManager::Unload(const std::string& path)
{
	auto it = _resources.find(path);
	if (it != _resources.end()) {
		it->second->ReleaseRef();
		if (it->second->RefCount() <= 0) {
			_resources.erase(it);
		}
	}
}

void ResourceManager::UnloadAll()
{
	for (auto& [path, resource] : _resources) {
		resource->ReleaseRef();
	}
	_resources.clear();
}

void ResourceManager::Update()
{
#ifdef _DEBUG
	//static float timeAccumulator = 0.0f; // 時間の累積
	//static constexpr float hotReloadInterval = 1.0f; // ホットリロードの間隔（秒）
	//timeAccumulator += Time::UnscaledDeltaTime();
	//if (timeAccumulator >= hotReloadInterval)
	//{
	//	timeAccumulator = 0.0f;
	//	UpdateHotReload();
	//}
#endif // _DEBUG
}

void ResourceManager::UpdateHotReload()
{
#ifdef _DEBUG
	// ホットリロードの実装例（ファイルの変更を監視してリロードするなど）
	for (auto& [path, resource] : _resources)
	{
		// ファイルの存在チェック
		if (!std::filesystem::exists(path)) continue;

		// 最終更新日時の取得
		auto& lastTime = _files[path];

		// 例えば、ファイルのタイムスタンプをチェックして変更があればリロード
		auto now = std::filesystem::last_write_time(path);

#ifdef _DEBUG
		// 追加: csoファイルの場合は対応するhlslファイルの更新もチェック
		std::filesystem::path fsPath(path);
		if (fsPath.extension() == ".cso") {
			// csoファイルだったら、hlslに変換して登録
			std::filesystem::path hlslPath = EnginePaths::ShaderSourceDir / fsPath.filename().replace_extension(".hlsl");
			
			// hlslファイルも監視対象に追加
			if (std::filesystem::exists(hlslPath))
			{
				lastTime = _files[hlslPath.string()];
				now = std::filesystem::last_write_time(hlslPath);
			}
		}
#endif // _DEBUG

		if (now != lastTime) {
			lastTime = now;
			resource->Reload();

			// 特定の型に対する追加処理
			if (std::dynamic_pointer_cast<Shader>(resource)) {
				UpdateShaderNames();
			}
		}
	}
#endif // _DEBUG
}

void ResourceManager::LoadAllShaders()
{
	namespace fs = std::filesystem;
	const std::string shaderDir = EnginePaths::ShadersDataDir; // シェーダーファイルの配置先ディレクトリ
	if (fs::exists(shaderDir) && fs::is_directory(shaderDir)) {
		for (const auto& entry : fs::recursive_directory_iterator(shaderDir)) {
			if (entry.is_regular_file()) {
				const auto& path = entry.path();
				// シェーダーファイルの拡張子をチェック（.cso）
				if (path.extension() == ".cso") 
				{
					std::string pathStr = path.string();

					// ファイル名からシェーダーターゲットを推定
					std::string stem = path.stem().string(); // ファイル名（拡張子なし）
					if (stem.size() < 2) continue; // 最低でも末尾2文字が必要
					std::string suffix = stem.substr(stem.size() - 2, 2); // ファイル名の末尾2文字を取得
					std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower); // 小文字に変換
					std::shared_ptr<Shader> shader;
					if (suffix == "vs") {
						// 頂点シェーダーのロード
						shader = Load<VertexShader>(pathStr);
					} else if (suffix == "ps") {
						// ピクセルシェーダーのロード
						shader = Load<PixelShader>(pathStr);
					} else if (suffix == "gs") {
						// ジオメトリシェーダーのロード
						shader = Load<GeometryShader>(pathStr);
					} else if (suffix == "cs") {
						// コンピュートシェーダーのロード
						shader = Load<ComputeShader>(pathStr);
					} else if (suffix == "hs") {
						// ハルシェーダーのロード
						shader = Load<HullShader>(pathStr);
					} else if (suffix == "ds") {
						// ドメインシェーダーのロード
						shader = Load<DomainShader>(pathStr);
					} else {
						continue; // 不明なタイプはスキップ
					}

					if (shader) {
						Console::Log("Loaded shader: " + path.string());
					} else {
						Console::LogWarning("Failed to load shader: " + path.string());
					}
				}
			}
		}
	}
	// シェーダーパスのキャッシュを更新
	UpdateShaderNames();
}

void ResourceManager::LoadAllTextures()
{
	// テクスチャフォルダ内のすべてのテクスチャファイルを読み込み
	namespace fs = std::filesystem;
	const std::string textureDir = EnginePaths::ImagesDataDir; // テクスチャファイルの配置先ディレクトリ
	if (fs::exists(textureDir) && fs::is_directory(textureDir)) {
		for (const auto& entry : fs::recursive_directory_iterator(textureDir)) {
			if (entry.is_regular_file()) {
				const auto& path = entry.path();
				// テクスチャファイルの拡張子をチェック（例: .png, .jpg, .jpeg, .tga, .dds）
				std::string ext = path.extension().string();
				std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower); // 小文字に変換
				if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".dds") {
					std::string pathStr = path.string();
					auto texture = Load<AssetTexture>(pathStr);
					if (texture) {
						Console::Log("Loaded texture: " + path.string());
					} else {
						Console::LogWarning("Failed to load texture: " + path.string());
					}
				}
			}
		}
	}
}

void ResourceManager::UpdateShaderNames()
{
#ifdef _DEBUG
	// 既存のキャッシュをクリア
	shaderPaths.clear();
	// シェーダーリソースを走査して名前を更新
	for (const auto& [path, resource] : _resources) {
		if (std::dynamic_pointer_cast<Shader>(resource)) {
			std::filesystem::path fsPath(path);

			// hlslファイルだったら登録
			if (fsPath.extension() == ".hlsl")
			{
				// 指定のシェーダーディレクトリ内の対応するファイルを探す
				std::filesystem::path fileName = fsPath.filename();
				fsPath = std::filesystem::path(EnginePaths::ShaderSourceDir) / fileName;
				// 存在するならそれを登録
				if (std::filesystem::exists(fsPath))
				{
					shaderPaths.push_back(fsPath.string());
					continue; // すでに追加したので次へ
				}
			}
		}
	}
#endif // _DEBUG

}