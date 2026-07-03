#include "pch.h"
#include "AssetBrowser.h"
#include <string>
#include <vector>
#include <filesystem>

#include <shellapi.h> // DragAcceptFiles, DragQueryFile, DragFinish
#pragma comment(lib, "Shell32.lib")

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI
#include "Engine/Resources/Texture.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "FileOpener.h"
#include "Engine/Core/framework.h"
#include "Engine/Scenes/SceneManager.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/EditorSupport/VcxprojHelper.h"
#include <Engine\Core\EnginePaths.h>
#include <Engine\Resources\ResourceManager.h>
#include "Dialog.h"
#include "Engine/Resources/AssetDatabase.h"
#include "Engine/Resources/ImportSettings/TextureImportSettings.h"
#include "Engine/Resources/AssetTypeUtils.h"
#include "Engine/Resources/Importers/ImporterRegistry.h"
#include "Engine/Editor/ImportSettings/ImportSettingsWindow.h"
#include "Engine/Editor/ImportSettings/ImportSettingsDrawerRegistry.h"

void AssetBrowser::Initialize()
{
	// アセットブラウザの初期化処理（アイコンの読み込みなど）
	currentDirectory = s_AssetPath;
	LoadTextureFromFile(Graphics::GetDevice(), std::filesystem::path(EnginePaths::IconsDir).append("directoryIcon.png").wstring().c_str(), directoryIcon.ReleaseAndGetAddressOf(), NULL);
	LoadTextureFromFile(Graphics::GetDevice(), std::filesystem::path(EnginePaths::IconsDir).append("fileIcon.png").wstring().c_str(), fileIcon.ReleaseAndGetAddressOf(), NULL);
	Refresh();

	CurryEngine::Resources::ImporterRegistry::Initialize();
	CurryEngine::Resources::ImportSettingsDrawerRegistry::Initialize();
	CurryEngine::Resources::AssetDatabase::Initialize("./TestAssets");
}

void AssetBrowser::InitializeDropTarget(HWND hwnd)
{
	HRESULT hr = OleInitialize(nullptr); // ドロップターゲットの初期化に必要

	oleInitializedHere = (hr == S_OK); // AssetBrowserがOLEを初期化したかどうかのフラグ

	DropTargetCallbacks callbacks;
	callbacks.isOverGrid = [](const POINTL& pt)
	{
			POINT screenPt = { pt.x, pt.y };
			return PtInRect(&assetGridScreenRect, screenPt) != 0;
	};
	callbacks.setHovering = [](bool hovering)
	{
			isExternalDragHovering = hovering;
			//Console::Log("External drag hovering: " + std::string(hovering ? "true" : "false"));
	};
	callbacks.onDrop = [](const std::vector<std::filesystem::path>& paths)
	{
			isExternalDragActive = false; // ドロップ完了したのでドラッグはアクティブでない
			bool anySucceeded = false;
			for (const auto& src : paths)
			{
				fs::path dst = currentDirectory / src.filename();
				if (fs::exists(dst))
					dst = MakeUniqueFilePath(currentDirectory,
						src.stem(), src.extension().string());
				try
				{
					if (fs::is_directory(src))
						fs::copy(src, dst, fs::copy_options::recursive);
					else
						fs::copy_file(src, dst);

					//if (auto meta = CurryEngine::Resources::AssetDatabase::Import(dst))
					//{
					//	// インポート成功したらインポート設定ウィンドウを開く
					//	CurryEngine::Resources::ImportSettingsWindow::OpenForNewAsset(meta->id);
					//}

					Console::Log(u8"Imported: " + dst.u8string());
					anySucceeded = true;
				}
				catch (const fs::filesystem_error& e)
				{
					std::string errorMsg = e.what();
					std::u8string errorMsgU8(errorMsg.begin(), errorMsg.end());
					LOG_ERROR(u8"Import failed: " + errorMsgU8);
				}
			}
			if (anySucceeded) Refresh();
	};
	callbacks.onDragEnter = []()
		{
			isExternalDragActive = true; // ドラッグが開始されたのでアクティブにする
			//Console::Log("External drag entered.");
		};
	callbacks.onDragLeave = []()
		{
			isExternalDragActive = false; // ドラッグが終了したのでアクティブでない
			//Console::Log("External drag left.");
		};


	dropTarget = new AssetBrowserDropTarget(std::move(callbacks));
	RegisterDragDrop(hwnd, dropTarget);
}

void AssetBrowser::FinalizeDropTarget(HWND hwnd)
{
	CurryEngine::Resources::AssetDatabase::Finalize();

	RevokeDragDrop(hwnd);
	if (dropTarget) {
		dropTarget->Release();
		dropTarget = nullptr;
	}
	// AssetBrowserがOLEを初期化していた場合のみ、OleUninitializeを呼び出す
	if (oleInitializedHere)
	{
		OleUninitialize();
	}
}


void AssetBrowser::OnDropFiles(HWND hwnd, HDROP hDrop)
{
#if 0

	// ── ドロップ座標がアセットグリッド内か確認 ──────────
	POINT dropPoint;
	DragQueryPoint(hDrop, &dropPoint);          // クライアント座標
	ClientToScreen(hwnd, &dropPoint);           // スクリーン座標に変換

	if (!PtInRect(&assetGridScreenRect, dropPoint))
	{
		// グリッド外へのドロップは無視
		DragFinish(hDrop);
		Console::Log("Drop ignored: outside asset grid.");
		return;
	}

	// ── ファイルパス収集 ──────────────────────────────
	UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);

	std::vector<fs::path> droppedPaths;
	droppedPaths.reserve(fileCount);

	for (UINT i = 0; i < fileCount; ++i)
	{
		WCHAR buf[MAX_PATH];
		if (DragQueryFileW(hDrop, i, buf, MAX_PATH))
			droppedPaths.emplace_back(buf);
	}
	DragFinish(hDrop);

	// currentDirectory へコピー
	bool anySucceeded = false;
	for (const auto& src : droppedPaths)
	{
		fs::path dst = currentDirectory / src.filename();

		// 同名ファイルが既にある場合はユニークな名前を生成
		if (fs::exists(dst))
			dst = MakeUniqueFilePath(currentDirectory,
				src.stem().string(), src.extension().string());
		try
		{
			// ディレクトリはフォルダごと再帰コピー
			if (fs::is_directory(src))
				fs::copy(src, dst, fs::copy_options::recursive);
			else
				fs::copy_file(src, dst);

			Console::Log("Imported: " + dst.string());
			anySucceeded = true;
		}
		catch (const fs::filesystem_error& e)
		{
			Console::LogError("Import failed: " + std::string(e.what()));
		}
	}

	if (anySucceeded)
		Refresh();
#endif // 0
}

AssetType AssetBrowser::DetectAssetTypeFromFile(const fs::path& path)
{
	std::string extension = path.extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	return CurryEngine::Resources::AssetTypeUtils::DetectFromExtension(extension);
}

void AssetBrowser::DrawGUI()
{
#ifdef USE_IMGUI
	if (isOpen)
	{
		ImGui::Begin("Asset Browser", &isOpen);
		ImGuiWindow* window = ImGui::GetCurrentWindow();
#if 1
		// currentDirectoryが空の場合はAssetsフォルダを表示する
		if (currentDirectory.empty())
		{
			currentDirectory = s_AssetPath;
		}

		//検索バー
		static char searchBuffer[128] = "";
		ImGui::InputTextWithHint("##SearchAssets", "Search assets...", searchBuffer, IM_ARRAYSIZE(searchBuffer));

		ImGui::Separator();

		if (ImGui::BeginTable("ProjectView", 2, ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("Folders", ImGuiTableColumnFlags_WidthFixed, 200.0f);
			ImGui::TableSetupColumn("Assets");
			ImGui::TableNextRow();

			//左側：フォルダ階層
			ImGui::TableSetColumnIndex(0);
			ImGui::BeginChild("FolderPanel");
			DrawFolderTree("./", currentDirectory);
			ImGui::EndChild();

			//右側：アセットグリッド
			ImGui::TableSetColumnIndex(1);
			ImGui::BeginChild("AssetPanel");
			DrawUnityPath(currentDirectory);
			DrawAssetGrid(currentDirectory, searchBuffer);
			ImGui::EndChild();

			ImGui::EndTable();
		}

#else

		if (currentDirectory != s_AssetPath)
		{
			if (ImGui::Button("<-"))
			{
				currentDirectory = currentDirectory.parent_path();
			}
		}

		static float padding = 16.0f;
		static float thumbnailSize = 64;
		float cellSize = thumbnailSize + padding;

		float panelHeight = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelHeight / cellSize);
		if (columnCount < 1)
			columnCount = 1;
		ImGui::Columns(columnCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(currentDirectory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, s_AssetPath);
			std::string filenameString = relativePath.filename().string();
			std::wstring filePathWString = path.wstring();
			ImGui::PushID(filenameString.c_str());
			bool isDirectory = directoryEntry.is_directory();

			ID3D11ShaderResourceView* iconImage = isDirectory ? directoryIcon.Get() : fileIcon.Get();
			std::wstring extension = relativePath.extension().wstring();
			if (extension == L".png" || extension == L".jpeg" || extension == L".DDS" || extension == L".dds" || extension == L".tif") {
				//見つかったら
				if (images.find(filePathWString) != images.end()) {
					iconImage = images.at(filePathWString).Get();
				}
				else {
					LoadTextureFromFile(Graphics::GetDevice(), filePathWString.c_str(), images[filePathWString].ReleaseAndGetAddressOf(), NULL);
					iconImage = images[filePathWString].Get();
				}
			}
			ImGui::ImageButton("", iconImage, { thumbnailSize,thumbnailSize });
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (isDirectory)
					currentDirectory /= path.filename();
			}
			ImGui::TextWrapped(filenameString.c_str());

			ImGui::PopID();
			ImGui::NextColumn();
		}

		ImGui::Columns(1);

		ImGui::SliderFloat("ThumbnailSize", &thumbnailSize, 32, 256);
		ImGui::SliderFloat("Padding", &padding, 0, 32);

#endif // 0

		// ImGui::End() を呼ぶ前に AssetBrowser ウィンドウ全体（子ウィンドウ含む）のフォーカス状態を取得
		bool isAssetBrowserFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

		ImGui::End();

		// -- キーボードショートカット ---------------------------------------
		// テキスト入力中・リネーム中・ウィンドウがフォーカスされてないときはショートカットを無効にする
		if ((!ImGui::GetIO().WantTextInput && !isRenaming) && isAssetBrowserFocused)
		{
			// Ctrl + Aで全選択
			if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A, false))
			{
				// 現在表示されているアセットをすべて選択する
				for (const auto& path : lastResultOrder)
					selectedAssets.insert(path);
				if (!selectedAssets.empty())
					lastClickedAsset = lastResultOrder.back(); // 最初の選択を基準点にする
			}

			if (!lastClickedAsset.empty())
			{
				// F2キーでリネーム開始
				if (ImGui::IsKeyPressed(ImGuiKey_F2, false))
				{
					StartRename(lastClickedAsset);
				}

				// Deleteキーで削除確認モーダルを表示
				if (ImGui::IsKeyPressed(ImGuiKey_Delete, false) && !selectedAssets.empty())
				{
					StartDelete(lastClickedAsset);
				}
			}
		}

		// -- モーダルの描画はウィンドウの描画とは分けて、常に最後に行う -----------------

		// スクリプト作成モーダル
		DrawScriptCreationModal();
		// HLSLシェーダー作成モーダル
		DrawHlslShaderCreationModal();
		// シーン作成モーダル
		DrawNewSceneCreationModal();
		// 削除確認モーダル
		DrawDeleteConfirmModal();
	};

	// TODO : MaterialEditorのDrawGUIをAssetBrowserのDrawGUIの中で呼ぶのは適切ではない。
	if (materialEditor)
	{
		materialEditor->DrawGUI(nullptr);
	}

#endif // USE_IMGUI

}

void AssetBrowser::Refresh()
{
	// キャッシュをクリアして再読み込みする
	lastResultOrder.clear();
	selectedAssets.clear();

	// キャッシュをクリア
	cacheSearchResults.clear();

#ifdef _DEBUG
	// ./Assets/Models以下のすべてのcereal/batchCerealファイルを削除（古いキャッシュが残っていると問題になるため）
	for (const auto& entry : fs::recursive_directory_iterator("./Assets/Models"))
	{
		if (entry.is_regular_file())
		{
			std::string ext = entry.path().extension().string();
			if (ext == ".cereal" || ext == ".batchCereal")
			{
				fs::remove(entry.path());
				LOG_INFO(u8"Deleted old cache file: " + entry.path().u8string());
			}
		}
	}
#endif // _DEBUG

}

void AssetBrowser::StartRename(const fs::path& assetPath)
{
	isRenaming = true;
	renamingJustStarted = true;
	renamingTarget = assetPath;
	strncpy_s(renameBuffer, reinterpret_cast<const char*>(assetPath.stem().u8string().c_str()), sizeof(renameBuffer));
}

void AssetBrowser::StartDelete(const fs::path& assetPath)
{
	// 複数選択中はすべてを削除対象としてモーダルを表示する
	// deleteTargetAssetはモーダルで使用するための変数なので、
	// 複数選択されている場合は特に意味を持たないが、便宜上最後にクリックされたアセットをセットしておく
	showDeleteConfirmModal = true;
	deleteTargetAsset = assetPath;
}

void AssetBrowser::ShowScriptCreationModal(const fs::path& initDir)
{
	// モーダルを表示するためのフラグと初期ディレクトリを設定
	showScriptCreationModal = true;
	scriptCreationInitDir = initDir;
}

void AssetBrowser::CreateCSharpScript(const fs::path& directory, const std::string& scriptName)
{
	fs::path newScriptPath = MakeUniqueFilePath(directory, scriptName);

	if (fs::exists(newScriptPath))
	{ // 同名のファイルが既に存在する場合はエラーを表示して処理を終了
		LOG_ERROR(u8"同じ名前のファイルがすでに存在します: " + newScriptPath.u8string());
		return;
	}
	if (!newScriptPath.has_stem() || newScriptPath.extension() != ".cs")
	{ // ファイル名が無効な場合はエラーを表示して処理を終了
		LOG_ERROR(u8"スクリプト名が不正です。ファイルの拡張子は .cs である必要があります。");
		return;
	}

	std::ofstream newScriptFile(newScriptPath);
	if (newScriptFile.is_open())
	{
		// スクリプトテンプレートの内容を新しいファイルに書き込む
		newScriptFile << "// This is a generated C# script.\n";
		newScriptFile << "using CurryEngine;\n\n";
		newScriptFile << "public class " << newScriptPath.stem().string() << " : Behaviour\n";
		newScriptFile << "{\n\n";
		newScriptFile << "    // Start is called before the first frame update\n";
		newScriptFile << "    public override void Start()\n";
		newScriptFile << "    {\n";
		newScriptFile << "        \n";
		newScriptFile << "    }\n\n";
		newScriptFile << "    // Update is called once per frame\n";
		newScriptFile << "    public override void Update()\n";
		newScriptFile << "    {\n";
		newScriptFile << "        \n";
		newScriptFile << "    }\n";
		newScriptFile << "}\n";
		newScriptFile.close();

		Console::Log("Created new script: " + newScriptPath.string());
		Refresh(); // キャッシュをクリアして新しいスクリプトがアセットブラウザに表示されるようにする
	}
	else
	{
		Console::LogError((std::string("Failed to create script file: ") + std::string(reinterpret_cast<const char*>(newScriptPath.u8string().c_str()))));
	}
}

void AssetBrowser::ShowHlslShaderCreationModal(const fs::path& initDir)
{
	showHlslShaderCreationModal = true;
	hlslShaderCreationInitDir = initDir;
}

void AssetBrowser::CreateHlslShader(const fs::path& directory, const std::string& shaderName, const std::string& extension)
{
	fs::path newShaderPath = MakeUniqueFilePath(directory, shaderName, extension);
	if (fs::exists(newShaderPath))
	{ // 同名のファイルが既に存在する場合はエラーを表示して処理を終了
		Console::LogError("A file with the same name already exists: " + newShaderPath.string());
		return;
	}
	if (!newShaderPath.has_stem())
	{ // ファイル名が無効な場合はエラーを表示して処理を終了
		Console::LogError("Invalid shader name. The file extension must be .hlsl");
		return;
	}
	std::ofstream newShaderFile(newShaderPath);
	if (newShaderFile.is_open())
	{
		char shaderType[3] = {}; // 頂点シェーダーなら "vs", ピクセルシェーダーなら "ps"、ジオメトリシェーダーなら "gs"、コンピュートシェーダーなら "cs" を shaderType にセットする
		std::string lowerStem = newShaderPath.stem().string();
		std::transform(lowerStem.begin(), lowerStem.end(), lowerStem.begin(), ::tolower);
		// stemの末尾2文字を見て、shaderTypeを決定する
		if (lowerStem.size() >= 2)
		{
			std::string suffix = lowerStem.substr(lowerStem.size() - 2);
			if (suffix == "vs")
				strcpy_s(shaderType, "vs");
			else if (suffix == "ps")
				strcpy_s(shaderType, "ps");
			else if (suffix == "gs")
				strcpy_s(shaderType, "gs");
			else if (suffix == "cs")
				strcpy_s(shaderType, "cs");
		}

		// シェーダーテンプレートの内容を新しいファイルに書き込む(shaderTypeに応じて内容を変える)
		switch (shaderType[0])
		{
			case 'v': // 頂点シェーダー
				newShaderFile << "// Vertex Shader Template\n";
				newShaderFile << "struct VSInput\n";
				newShaderFile << "{\n";
				newShaderFile << "    float3 position : POSITION;\n";
				newShaderFile << "    float3 normal : NORMAL;\n";
				newShaderFile << "    float2 uv : TEXCOORD0;\n";
				newShaderFile << "};\n\n";
				newShaderFile << "struct VSOutput\n";
				newShaderFile << "{\n";
				newShaderFile << "    float4 position : SV_POSITION;\n";
				newShaderFile << "    float3 normal : NORMAL;\n";
				newShaderFile << "    float2 uv : TEXCOORD0;\n";
				newShaderFile << "};\n\n";
				newShaderFile << "VSOutput main(VSInput input)\n";
				newShaderFile << "{\n";
				newShaderFile << "    VSOutput output;\n";
				newShaderFile << "    // TODO: Implement vertex shader logic here\n";
				newShaderFile << "    return output;\n";
				newShaderFile << "}\n";
				break;
			case 'p': // ピクセルシェーダー
				newShaderFile << "// Pixel Shader Template\n";
				newShaderFile << "struct PSInput\n";
				newShaderFile << "{\n";
				newShaderFile << "    float4 position : SV_POSITION;\n";
				newShaderFile << "    float3 normal : NORMAL;\n";
				newShaderFile << "    float2 uv : TEXCOORD0;\n";
				newShaderFile << "};\n\n";
				newShaderFile << "float4 main(PSInput input) : SV_TARGET\n";
				newShaderFile << "{\n";
				newShaderFile << "    // TODO: Implement pixel shader logic here\n";
				newShaderFile << "    return float4(1, 0, 1, 1); // Magenta for debugging\n";
				newShaderFile << "}\n";
				break;
			case 'g': // ジオメトリシェーダー
				newShaderFile << "// Geometry Shader Template\n";
				newShaderFile << "struct GSInput\n";
				newShaderFile << "{\n";
				newShaderFile << "    float4 position : SV_POSITION;\n";
				newShaderFile << "    float3 normal : NORMAL;\n";
				newShaderFile << "    float2 uv : TEXCOORD0;\n";
				newShaderFile << "};\n\n";
				newShaderFile << "[maxvertexcount(3)]\n";
				newShaderFile << "void main(triangle GSInput input[3], inout TriangleStream<GSOutput> triStream)\n";
				newShaderFile << "{\n";
				newShaderFile << "    // TODO: Implement geometry shader logic here\n";
				newShaderFile << "}\n";
				break;
			case 'c': // コンピュートシェーダー
				newShaderFile << "// Compute Shader Template\n";
				newShaderFile << "[numthreads(8, 8, 1)]\n";
				newShaderFile << "void main(uint3 DTid : SV_DispatchThreadID)\n";
				newShaderFile << "{\n";
				newShaderFile << "    // TODO: Implement compute shader logic here\n";
				newShaderFile << "}\n";
				break;
			default: // 不明なタイプ
				newShaderFile << "// HLSL Shader Template\n";
				newShaderFile << "// TODO: Implement shader logic here\n";
				break;
		};

		newShaderFile.close();

		Console::Log("Created new shader: " + newShaderPath.string());

		if (extension == ".hlsl")
		{
			// 新しいシェーダーをリソースマネージャーにロードして登録する
			ResourceManager::Load(newShaderPath.string());
			ResourceManager::LoadAllShaders();
		}

		// 新しいシェーダーをプロジェクトに登録するためのキューに追加
		VcxprojHelper::EnqueueShaderRegistration(newShaderPath);

		Refresh(); // キャッシュをクリアして新しいシェーダーがアセットブラウザに表示されるようにする
	}
	else
	{
		LOG_ERROR(u8"シェーダーファイルの作成に失敗しました: " + newShaderPath.u8string());
	}
}

void AssetBrowser::ShowNewSceneCreationModal(const fs::path& initDir)
{
	showNewSceneCreationModal = true;
	sceneCreationInitDir = initDir;
}

void AssetBrowser::CreateNewScene(const fs::path& templateScenePath, const fs::path& newScenePath)
{
	if (fs::exists(newScenePath))
	{ // 同名のファイルが既に存在する場合はエラーを表示して処理を終了
		LOG_ERROR(u8"同じ名前のファイルがすでに存在します: " + newScenePath.u8string());
		return;
	}
	if (!newScenePath.has_stem() || newScenePath.extension() != ".scene")
	{ // ファイル名が無効な場合はエラーを表示して処理を終了
		LOG_ERROR(u8"シーン名が不正です。ファイル拡張子は .scene である必要があります。");
		return;
	}
	json j;
	JsonFileHandler::LoadJsonFromFile(j, templateScenePath.string(), JsonIOFormat::Text); // テンプレートシーンのJSONを読み込む
	std::string sceneName = newScenePath.stem().string();
	j["name"] = sceneName; // シーン名を新しいものに変更
	JsonFileHandler::SaveJsonToFile(j, newScenePath.string()); // JSONファイルに保存
	//SceneManager::Register(newScenePath.string()); // シーンマネージャーに登録
	SceneManager::ChangeScene(sceneName); // シーンマネージャーで切り替え
	Refresh(); // キャッシュをクリアして新しいシーンがアセットブラウザに表示されるようにする
}

void AssetBrowser::OpenAsset(const fs::path& assetPath)
{
	auto relativePath = std::filesystem::relative(assetPath, s_AssetPath);
	auto absolutePath = std::filesystem::absolute(assetPath);
	AssetType type = DetectAssetTypeFromFile(assetPath);
	if (type == AssetType::Scene)
	{
		// シーンファイルならシーンを開く
		SceneManager::ChangeScene(assetPath.stem().string());
	}
	else if (type == AssetType::Prefab)
	{
		// プレハブファイルならプレハブエディタで開く（未実装）
		//Console::Log("Opening prefab editor for: " + path.string());
		Console::Log("Prefab editing is not implemented yet.");
	}
	else if (type == AssetType::Script)
	{
		// スクリプトファイルならVisual Studioで開く
		std::wstring slnPath = std::filesystem::path(EnginePaths::SolutionFile).wstring(); // ソリューションファイルのパス
		std::wstring filePath = absolutePath.wstring();
		//OpenFileInVisualStudio(slnPath, filePath); // ソリューションファイルパスがハードコードされているため、Visual Studioが正しく開けない可能性がある
		OpenFileWithDefaultApplication(absolutePath.wstring()); // とりあえず関連付けされたアプリで開く（Visual Studioが関連付けされていればそちらで開く）
	}
	else if (type == AssetType::Material)
	{
		// マテリアルファイルならマテリアルエディタで開く
		if (auto meta = CurryEngine::Resources::AssetDatabase::FindByPath(assetPath))
		{
			materialEditor = std::make_unique<CurryEngine::Editor::MaterialEditor>(meta->id);
		}
		else
		{
			LOG_ERROR(u8"Failed to open material editor: Material asset not found in database for path: " + assetPath.u8string());
		}
	}
	else
	{
		// それ以外のファイルなら既定のアプリで開く
		// ファイルパスを絶対パスに変換してから開く（相対パスだと失敗することがあるため）
		OpenFileWithDefaultApplication(absolutePath.wstring());
	}
}

void AssetBrowser::SearchAssets(const fs::path& root, const std::u8string& keyword, std::vector<fs::directory_entry>& results)
{
	//キャッシュされてるか確認
	if (cacheSearchResults.find(root) != cacheSearchResults.end())
	{
		if (cacheSearchResults.at(root).find(keyword) != cacheSearchResults.at(root).end())
		{
			//キャッシュされた結果を返す
			results = cacheSearchResults.at(root).at(keyword);
			return;
		}
	}

	if (keyword.empty())
	{
		for (const auto& entry : filesystem::directory_iterator(root))
		{
			results.push_back(entry);
		}
	}
	else
	{
		for (const auto& entry : filesystem::recursive_directory_iterator(root))
		{
			std::u8string lowerName = entry.path().u8string();
			std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
			if (lowerName.find(keyword) != std::u8string::npos) {
				results.push_back(entry);
			}
		}
	}

	//結果をキャッシュ
	cacheSearchResults[root][keyword] = results;
}

void AssetBrowser::OnAssetDeleted(const fs::path& assetPath)
{
	// アセットが削除された後の処理

	// 削除されたアセットがシェーダーだった場合、VcxprojHelperに削除されたシェーダーのパスを通知してプロジェクトファイルからも削除する
	if (DetectAssetTypeFromFile(assetPath) == AssetType::Shader)
	{
		VcxprojHelper::EnqueueShaderUnregistration(assetPath);
	}

	// アセットデータベースからも削除
	//CurryEngine::Resources::AssetDatabase::RemoveByPath(assetPath);

}

#ifdef USE_IMGUI
void AssetBrowser::DrawFolderTree(const std::filesystem::path& root, std::filesystem::path& selectedFolder)
{
	for (const auto& entry : std::filesystem::directory_iterator(root))
	{
		//ディレクトリなら
		if (entry.is_directory())
		{
			const auto& name = entry.path().filename().u8string();
			ImGuiTreeNodeFlags flags = (selectedFolder.compare(entry) == 0) ? ImGuiTreeNodeFlags_Selected : 0;

			bool open = ImGui::TreeNodeEx(reinterpret_cast<const char*>(name.c_str()), flags);

			//フォルダをドロップターゲットに
			if (settings.acceptDropToFolderTree)
			{
				if (HandleDropTargetForFolder(entry.path()))
				{
					selectedFolder = entry.path();
				}
			}


			if (ImGui::IsItemClicked()) 
			{
				selectedFolder = entry.path();
			}

			if (open)
			{
				DrawFolderTree(entry.path(), selectedFolder);
				ImGui::TreePop();
			}
		}
	}
}
#endif // USE_IMGUI

#ifdef USE_IMGUI
void AssetBrowser::DrawAssetGrid(const std::filesystem::path& folderPath, const char* filter)
{
	std::u8string query = filter ? std::u8string(reinterpret_cast<const char8_t*>(filter)) : u8"";
	std::transform(query.begin(), query.end(), query.begin(), ::tolower);


	// サムネイルサイズスライダー
	ImGui::TextDisabled("Size");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.f);
	ImGui::SliderFloat("##ThumbnailSize", &thumbnailSize, 32.0f, 128.0f, "%.0f");
	ImGui::Separator();

	// ── グリッドの列数計算 ──
	constexpr float padding = 10.0f;
	float totalWidth = ImGui::GetContentRegionAvail().x;
	int columns = max(1, int(totalWidth / (thumbnailSize + padding)));
	float textHeight = ImGui::GetTextLineHeightWithSpacing();

	// ── コンテキストメニュー（ホバー対象を記録しておく） ──
	// ホバー中のアイテムパスを先にここで受け取る構造にする
	static fs::path pendingContextPath; // 右クリックでコンテキストメニューを開く対象のパスを一時的に保存する変数
	static bool pendingContextOpen = false; // コンテキストメニューを開くべきかどうかのフラグ

	// 今フレームでアイテム上でクリックされたか追跡するフラグ
	bool clickedOnItem = false;
	
	// アセットグリッドパネルの画面座標を毎フレーム記録しておく（ドラッグ＆ドロップのターゲット判定に使用）
	{
		ImVec2 panelMin = ImGui::GetCursorScreenPos();
		ImVec2 panelSize = ImGui::GetContentRegionAvail();
		ImVec2 panelMax = { panelMin.x + panelSize.x, panelMin.y + panelSize.y };

		HWND hwnd = Graphics::GetHwnd();
		POINT ptMin = { LONG(panelMin.x), LONG(panelMin.y) };
		POINT ptMax = { LONG(panelMax.x), LONG(panelMax.y) };
		ClientToScreen(hwnd, &ptMin);
		ClientToScreen(hwnd, &ptMax);

		assetGridScreenRect = { ptMin.x, ptMin.y, ptMax.x, ptMax.y };
	}

	// ── アセットグリッドの描画 ──
	if (ImGui::BeginTable("AssetGrid", columns, ImGuiTableFlags_NoBordersInBody))
	{
		//for (const auto& entry : std::filesystem::directory_iterator(folderPath))
		std::vector<fs::directory_entry> results;
		SearchAssets(folderPath, query, results);

		// 前回の検索結果の順序を保存しておく（Shift範囲選択のため）
		lastResultOrder.clear();
		for (const auto& entry : results)
		{
			lastResultOrder.push_back(entry.path());
		}

		// ── アイテムごとの描画ループ ──
		int i = 0;
		for (const auto& entry : results)
		{
			ImGui::TableNextColumn();

			// 描画する前に、アイテムがビューポート内にあるかどうかを確認して、ビューポート外のアイテムは描画しないようにする
			ImVec2 cursorPos = ImGui::GetCursorScreenPos();
			if (!ImGui::IsRectVisible(cursorPos, ImVec2(cursorPos.x + thumbnailSize, cursorPos.y + thumbnailSize + (padding * 2.0f))))
			{
				// ビューポート外のアイテムは描画せずに、ダミーのスペースだけ確保して次のアイテムへ
				ImGui::Dummy(ImVec2(thumbnailSize, thumbnailSize + (padding * 2.0f) + textHeight)); // アイコンとテキストのスペースを確保
				continue;
			}

			ImGui::BeginGroup();

			const auto& path = entry.path();
			auto relativePath = std::filesystem::relative(path, s_AssetPath);
			std::u8string filePathU8String = path.u8string();
			std::u8string filenameString = relativePath.filename().u8string();
			std::wstring filePathWString = path.wstring();
			const char* pathStr = reinterpret_cast<const char*>(filePathU8String.c_str());
			const char* filenameStr = reinterpret_cast<const char*>(filenameString.c_str());

			ImGui::PushID(i++);
			bool isDirectory = entry.is_directory();
			AssetType type = DetectAssetTypeFromFile(path);

			// -- アイコン画像の解決 -------------------------------
			ID3D11ShaderResourceView* iconImage = isDirectory ? directoryIcon.Get() : fileIcon.Get();
			//アセットタイプがテクスチャなら
			if (type == AssetType::Texture) {
				//見つかったら
				if (images.find(filePathWString) != images.end()) {
					iconImage = images.at(filePathWString).Get();
				}
				else {
					LoadTextureFromFile(Graphics::GetDevice(), filePathWString.c_str(),
						images[filePathWString].ReleaseAndGetAddressOf(), NULL);
					iconImage = images[filePathWString].Get();
				}
			}

			// -- 選択ハイライト ----------------------------------
			bool isSelected = selectedAssets.contains(path);
			if (isSelected)
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

			// -- アイコンの描画とインタラクション ----------------
			ImGui::ImageButton("##img", (ImTextureRef)iconImage,
				{ thumbnailSize,thumbnailSize });

			if (isSelected)
				ImGui::PopStyleColor();

			// -- タイプバッジ (ディレクトリ以外) ----------------------
			if (!isDirectory)
			{
				const char* badge = nullptr;
				ImVec4 badgeColor = { 0.5f, 0.5f, 0.5f, 1.0f };
				switch (type)
				{
				case AssetType::Texture:   badge = "TEX";    badgeColor = { 0.2f, 0.6f, 0.9f, 1.0f }; break;
				case AssetType::Model: badge = "3D";     badgeColor = { 0.4f, 0.8f, 0.4f, 1.0f }; break;
				case AssetType::Sound:     badge = "WAV";    badgeColor = { 0.9f, 0.7f, 0.2f, 1.0f }; break;
				case AssetType::Scene:     badge = "SCN";    badgeColor = { 0.8f, 0.3f, 0.8f, 1.0f }; break;
				case AssetType::Prefab:    badge = "PFB";    badgeColor = { 0.9f, 0.5f, 0.1f, 1.0f }; break;
				case AssetType::Script:    badge = "C#";     badgeColor = { 0.2f, 0.8f, 0.6f, 1.0f }; break;
				case AssetType::Material:  badge = "MAT";    badgeColor = { 0.6f, 0.4f, 0.9f, 1.0f }; break;
				default: break;
				}
				if (badge)
				{
					// ボタン左上にオーバーレイ (テキスト幅を考慮して配置)
					int textCount = static_cast<int>(strlen(badge));
					ImVec2 btnMin = ImGui::GetItemRectMin();
					ImVec2 btnMax = ImGui::GetItemRectMax();
					ImDrawList* dl = ImGui::GetWindowDrawList();
					ImVec2 badgeMax = { btnMin.x + 8.0f + textCount * 7.0f, btnMin.y + 14.0f }; // バッジの幅はテキスト長に応じて変動
					ImVec2 badgeMin = { btnMin.x, btnMin.y - 4.0f }; // アイコンの上に少し重なるように配置
					dl->AddRectFilled(badgeMin, badgeMax, ImGui::ColorConvertFloat4ToU32(badgeColor), 3.0f);
					dl->AddText({ badgeMin.x + 3.5f, badgeMin.y - 1.f }, IM_COL32(255, 255, 255, 255), badge);
				}
			}

			// -- ホバー・クリックの処理 -------------------------------
			if (ImGui::IsItemHovered())
			{
				// 左ダブルクリックで開く
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (isDirectory) // ディレクトリならその中へ移動
						currentDirectory /= path.filename();
					else // ファイルなら開く
						OpenAsset(path);
				}
				// シングルクリックで選択 (リネーム中は除く)
				else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !isRenaming)
				{
					clickedOnItem = true;
					bool ctrl = ImGui::GetIO().KeyCtrl;
					bool shift = ImGui::GetIO().KeyShift;

					if (shift && !lastClickedAsset.empty())
					{
						// -- Shift範囲選択 ----------------
						// Ctrlを同時押ししていなければ既存選択をクリア
						if (!ctrl) selectedAssets.clear();

						// lastResultOrder から基点と今回の間を選択する
						auto itAnchor = std::find(lastResultOrder.begin(), lastResultOrder.end(), lastClickedAsset);
						auto itCurrent = std::find(lastResultOrder.begin(), lastResultOrder.end(), path);
						if (itAnchor != lastResultOrder.end() && itCurrent != lastResultOrder.end())
						{
							if (itAnchor > itCurrent) std::swap(itAnchor, itCurrent);
							for (auto it = itAnchor; it <= itCurrent; ++it)
								selectedAssets.insert(*it);
						}
						// lastClickedAsset は Shift中は更新しない（基点を維持）
					}
					else if (ctrl)
					{
						// -- Ctrlで複数選択のトグル ----------------
						if (isSelected)
							selectedAssets.erase(path);
						else
							selectedAssets.insert(path);
						lastClickedAsset = path;
					}
					else
					{
						// -- 単一選択 ----------------
						selectedAssets.clear();
						selectedAssets.insert(path);
						lastClickedAsset = path;
					}
				}
				// 右クリック -> 選択してコンテキストメニューを予約
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					// 右クリックしたアイテムが選択済みでない場合は、選択をリセットしてそのアイテムだけを選択状態にする
					if (!isSelected)
					{
						selectedAssets.clear();
						selectedAssets.insert(path);
						lastClickedAsset = path;
					}
					pendingContextPath = path; // コンテキストメニューを開く対象のパスを保存
					pendingContextOpen = true; // コンテキストメニューを開くべきフラグを立てる
					clickedOnItem = true; // アイテム上でクリックされたフラグを立てる
				}
			}

			// -- ドロップターゲット(フォルダのみ) -------------------------------
			if (settings.acceptDropToAssetGrid && isDirectory)
			{
				HandleDropTargetForFolder(path);
			}

			// -- ドラッグソース ----------------------------------
			if (ImGui::BeginDragDropSource())
			{
				const char* payloadType = isDirectory ? "FOLDER_PATH" : "ASSET_PATH";
				ImGui::SetDragDropPayload(payloadType, 
					pathStr, path.u8string().size() + 1);
				// ドラッグ中プレビューラベル
				ImGui::TextUnformatted(filenameStr);
				ImGui::EndDragDropSource();
			}
			
			// -- ファイル名 / リネーム入力 -------------------------------
			bool renamingThis = (isRenaming && renamingTarget == path);
			if (renamingThis)
			{
				// リネーム開始直後はテキスト入力にフォーカスをセット
				if (renamingJustStarted)
				{
					ImGui::SetKeyboardFocusHere(0); // フォーカスを入力テキストにセット
					renamingJustStarted = false;
				}

				// 入力フィールドを表示
				ImGui::SetNextItemWidth(thumbnailSize);
				ImGuiInputTextFlags renameFlags =
					ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
				bool confirmed = ImGui::InputText("##Rename", renameBuffer, IM_ARRAYSIZE(renameBuffer), renameFlags);

				// フォーカスを失う or キャンセル (Escキー) でリネームモード終了
				bool lostFocus = ImGui::IsItemDeactivated() && !confirmed; // Enterで確定した場合は非アクティブ化されてもリネームモードを終了しない
				
				// 確定 or キャンセル or フォーカスを失う でリネームモード終了
				if (confirmed && renameBuffer[0] != '\0')
				{
					// ファイルの場合は元の拡張子を強制維持
					std::u8string newName = reinterpret_cast<const char8_t*>(renameBuffer);
					if (!isDirectory)
					{
						fs::path tmp(newName);
						if (tmp.extension().empty())
							newName += path.extension().u8string();
					}
					fs::path newPath = path.parent_path() / newName;
					if (!fs::exists(newPath))
					{
						std::error_code ec;
						fs::rename(path, newPath, ec);
						if (!ec)
						{
							//CurryEngine::Resources::AssetDatabase::Rename(path, newPath); // アセットデータベースのパスも更新
							lastClickedAsset = newPath; // クリックされたアセットのパスを新しいものに更新
							Refresh();
						}
						else
							LOG_ERROR("Rename failed: " + ec.message());
					}
					else
						LOG_ERROR("A file with that name already exists.");
					isRenaming = false;
				}
				else if (ImGui::IsItemDeactivated() && !ImGui::IsItemDeactivatedAfterEdit())
				{
					isRenaming = false;
				}
			}
			else
			{
				// 通常表示（長いファイル名はサムネイル幅でクリップ）
				float textWidth = ImGui::CalcTextSize(filenameStr).x;
				if (textWidth > thumbnailSize)
				{
					// クリップする場合は末尾を "..." に置き換える
					float ectWidth = ImGui::CalcTextSize("...").x; // これで "..." の幅を計算しておく
					int clipCount = 0;
					while (textWidth > thumbnailSize && clipCount < filenameString.size())
					{
						clipCount++;
						std::u8string clipped = filenameString.substr(0, filenameString.size() - clipCount) + u8"...";
						textWidth = ImGui::CalcTextSize(reinterpret_cast<const char*>(clipped.c_str())).x;
					}
					filenameString = filenameString.substr(0, filenameString.size() - clipCount) + u8"...";
				}
				ImGui::TextUnformatted(reinterpret_cast<const char*>(filenameString.c_str()));
			}

			ImGui::PopID();
			ImGui::EndGroup();
		}

		// 空白エリアの右クリック
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)
			&& ImGui::IsMouseClicked(ImGuiMouseButton_Right)
			&& !pendingContextOpen) // すでにアイテム上の右クリックで開くべきコンテキストが予約されていない場合のみ
		{
			pendingContextPath = fs::path(); // 空のパスをセットして、コンテキストメニューが空白エリア用であることを示す
			pendingContextOpen = true; // コンテキストメニューを開くべきフラグを立てる
		}

		ImGui::EndTable();

		// -- 空白エリアのクリックで選択解除 -------------------------------
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)
			&& ImGui::IsMouseClicked(ImGuiMouseButton_Left)
			&& !clickedOnItem // アイテム上でクリックされたフラグが立っていない場合のみ
			&& !ImGui::GetIO().KeyCtrl // Ctrlキーが押されていない場合のみ
			&& !ImGui::GetIO().KeyShift // Shiftキーが押されていない場合のみ
			&& !ImGui::IsPopupOpen("AssetGridContextMenu")) // コンテキストメニューが開いている最中は誤操作を防ぐため選択解除しない
		{
			selectedAssets.clear();
			lastClickedAsset.clear();
			if (isRenaming) {
				isRenaming = false; // リネームモードも終了
			}
		}

		// -- ドロップターゲットのビジュアルフィードバック -------------------------------
		if (isExternalDragHovering)
		{
			ImDrawList* dl = ImGui::GetWindowDrawList();
			ImVec2 panelMin = ImGui::GetCursorScreenPos();
			ImVec2 panelSize = ImGui::GetContentRegionAvail();
			ImVec2 panelMax = { panelMin.x + panelSize.x, panelMin.y + panelSize.y };
			
			// 半透明の青いオーバーレイ
			dl->AddRectFilled(panelMin, panelMax, IM_COL32(100, 150, 255, 100));

			//青い枠線
			dl->AddRect(panelMin, panelMax, IM_COL32(100, 150, 255, 200), 0.0f, ImDrawFlags_None, 3.0f);

			// ドロップ可能であることを示すテキスト
			const char* msg = reinterpret_cast<const char*>(u8"ここにドロップしてインポート");
			ImVec2 textSize = ImGui::CalcTextSize(msg);
			ImVec2 textPos = { 
				panelMin.x + (panelSize.x - textSize.x) * 0.5f,
				panelMin.y + (panelSize.y - textSize.y) * 0.5f 
			};
			// テキストの影 (読みやすくするための黒いテキストを少しオフセットして描画)
			dl->AddText({ textPos.x + 1, textPos.y + 1 }, IM_COL32(0, 0, 0, 150), msg);
			// テキスト本体
			dl->AddText(textPos, IM_COL32(255, 255, 255, 255), msg);

		}

		// -- コンテキストメニューの表示 -------------------------------
		if (pendingContextOpen)
		{
			ImGui::OpenPopup("AssetGridContextMenu");
			pendingContextOpen = false; // フラグをリセット
		}
		ShowContextMenu(pendingContextPath);
	}
}
#endif // USE_IMGUI

#ifdef USE_IMGUI
void AssetBrowser::DrawUnityPath(const fs::path& path) {
	fs::path result = path.lexically_normal().make_preferred(); // 正規化してOSに合わせたパス区切り文字に変換

	// Windows用の \ を / に統一
	//std::replace(result.begin(), result.end(), '\\', '/');

	// スラッシュで分割し、" > " で連結
	std::u8string resultStr = result.u8string();
	std::stringstream ss(std::string(resultStr.begin(), resultStr.end()));
	std::string segment;
	std::vector<std::u8string> segments;

	while (std::getline(ss, segment, '\\')) {
		if (!segment.empty() && segment != ".") {
			std::u8string segmentU8(segment.begin(), segment.end());
			segments.push_back(segmentU8);
		}
	}

	std::u8string dir = u8".\\";
	for (size_t i = 0; i < segments.size(); i++)
	{
		dir += segments[i] + u8"\\";
		if (ImGui::Button(reinterpret_cast<const char*>(segments[i].c_str())))
		{
			currentDirectory = dir;
		}
		//フォルダをドロップターゲットに
		if (settings.acceptDropToCurrentPath)
		{
			HandleDropTargetForFolder(dir);
		}
		if (i < segments.size() - 1)
		{
			ImGui::SameLine();
			ImGui::Text(">");
			ImGui::SameLine();
		}

	}
}
#endif // USE_IMGUI

#ifdef USE_IMGUI

void AssetBrowser::DrawScriptCreationModal()
{
	// モーダルが開くフラグが立ったらOpenPopupを呼び出す
	if (showScriptCreationModal)
	{
		ImGui::OpenPopup("Create New Script");
		strncpy_s(scriptNameBuffer, NewScriptName, IM_ARRAYSIZE(scriptNameBuffer)); // バッファに初期値をセット
		showScriptCreationModal = false; // フラグをリセット
	}
	else if (!ImGui::IsPopupOpen("Create New Script"))
	{
		return; // モーダルが開いていない場合は何もしない
	}

	// モーダルの内容
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(360, 130), ImGuiCond_Appearing);

	if (ImGui::BeginPopupModal("Create New Script", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings))
	{
		// スクリプト名入力フィールド
		ImGui::Text(reinterpret_cast<const char*>(u8"名前:"));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-1); // ウィンドウ幅いっぱいに入力フィールドを広げる

		// Enterで確定、全選択、文字フィルタ（英数字とアンダースコアのみ）
		ImGuiInputFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackCharFilter;
		static auto CharFilter = [](ImGuiInputTextCallbackData* data) -> int {
			const char* allowedChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
			if (data->EventChar == '\n' || data->EventChar == '\r') // Enterキー
				return 0; // 許可
			if (data->EventChar == '\b') // バックスペース
				return 0; // 許可
			if (data->EventChar == '\0') // 終端文字
				return 0; // 許可
			if (strchr(allowedChars, data->EventChar) != nullptr) // 許可された文字
				return 0; // 許可
			return 1; // 不許可
			};

		// 入力フィールドを表示
		bool confirm = ImGui::InputText("##ScriptName", scriptNameBuffer, IM_ARRAYSIZE(scriptNameBuffer), inputFlags, CharFilter);

		// 入力されたスクリプト名の検証
		bool isValidPath = true;
		fs::path newScriptPath = scriptCreationInitDir / (std::string(scriptNameBuffer) + ".cs");
		const char8_t* message;
		if (scriptNameBuffer[0] == '\0')
		{
			message = u8"名前を入力してください。";
			isValidPath = false;
		}
		else if (fs::exists(newScriptPath))
		{
			message = u8"同名のファイルが既に存在します。";
			isValidPath = false;
		}
		else
		{
			message = u8"";
		}
		// エラーメッセージを表示
		ImGui::TextWrapped(reinterpret_cast<const char*>(message));
		
		ImGui::Spacing(); // スペースを空ける

		ImVec2 buttonSize(160, 0);

		// 作成ボタン
		ImGui::BeginDisabled(!isValidPath);
		if (ImGui::Button(reinterpret_cast<const char*>(u8"作成"), buttonSize) || (confirm && isValidPath))
		{
			CreateCSharpScript(scriptCreationInitDir, scriptNameBuffer);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();

		// キャンセルボタン
		ImGui::SameLine();
		if (ImGui::Button(reinterpret_cast<const char*>(u8"キャンセル"), buttonSize) || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

}

// 新規HLSLシェーダー作成モーダルの内容
void AssetBrowser::DrawHlslShaderCreationModal()
{
	// モーダルが開くフラグが立ったらOpenPopupを呼び出す
	if (showHlslShaderCreationModal)
	{
		ImGui::OpenPopup("Create New Shader");
		strncpy_s(hlslShaderNameBuffer, NewHlslShaderName, IM_ARRAYSIZE(hlslShaderNameBuffer)); // バッファに初期値をセット
		showHlslShaderCreationModal = false; // フラグをリセット
	}
	else if (!ImGui::IsPopupOpen("Create New Shader"))
	{
		return; // モーダルが開いていない場合は何もしない
	}
	// モーダルの内容
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always); // サイズは内容に合わせて自動調整
	if (ImGui::BeginPopupModal("Create New Shader", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings))
	{
		// シェーダーテンプレートの選択
		static int selectedTemplate = 0;
		//static const char* templates[] = {
		//	"Unlit Shader",
		//	"Lit Shader",
		//	"Custom Template",
		//};
		//static const char8_t* templateDescriptions[] = {
		//	u8"Unlit Shader: ライティングなしのシンプルなシェーダー。",
		//	u8"Lit Shader: ライティングに対応した基本的なシェーダー。",
		//	u8"Custom Template: ユーザーが作成したシェーダーテンプレートを使用します。"
		//};
		//static fs::path templateShaderPaths[] = {
		//	"./Assets/Shaders/UnlitTemplate.shader",
		//	"./Assets/Shaders/LitTemplate.shader",
		//	"./Assets/Shaders/"
		//};

		static const char* templates[] = {
			"HLSL Header",
			"Vertex Shader",
			"Pixel Shader",
			"Compute Shader"
		};
		static const char8_t* templateDescriptions[] = {
			u8"HLSL Header: 共通の定数バッファや構造体を定義するためのテンプレート。",
			u8"Vertex Shader: 頂点ごとに実行されるシェーダー。頂点の位置や法線などを処理します。",
			u8"Pixel Shader: ピクセルごとに実行されるシェーダー。最終的なピクセルの色を決定します。",
			u8"Compute Shader: 汎用計算シェーダー。グラフィックス以外の計算にも使用できます。"
		};
		static std::string templateExtensions[] = {
			".hlsli",
			".vs.hlsl",
			".ps.hlsl",
			".cs.hlsl"
		};
		//static fs::path templateShaderPaths[] = {
		//	"./Assets/Shaders/VertexShaderTemplate.shader",
		//	"./Assets/Shaders/PixelShaderTemplate.shader",
		//	"./Assets/Shaders/ComputeShaderTemplate.shader"
		//};
		ImGui::Text(reinterpret_cast<const char*>(u8"テンプレート:"));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-1); // ウィンドウ幅いっぱいにコンボボックスを広げる
		ImGui::Combo("##ShaderTemplate", &selectedTemplate, templates, IM_ARRAYSIZE(templates));
		ImGui::TextWrapped(reinterpret_cast<const char*>(templateDescriptions[selectedTemplate]));
		ImGui::Spacing(); // スペースを空ける

		// シェーダー名入力フィールド
		ImGui::Text(reinterpret_cast<const char*>(u8"名前:"));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-1); // ウィンドウ幅いっぱいに入力フィールドを広げる
		// Enterで確定、全選択、文字フィルタ（英数字とアンダースコアのみ）
		ImGuiInputFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackCharFilter;
		static auto CharFilter = [](ImGuiInputTextCallbackData* data) -> int {
			const char* allowedChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
			if (data->EventChar == '\n' || data->EventChar == '\r') // Enterキー
				return 0; // 許可
			if (data->EventChar == '\b') // バックスペース
				return 0; // 許可
			if (data->EventChar == '\0') // 終端文字
				return 0; // 許可
			if (strchr(allowedChars, data->EventChar) != nullptr) // 許可された文字
				return 0; // 許可
			return 1; // 不許可
			};
		// 入力フィールドを表示
		bool confirm = ImGui::InputText("##ShaderName", hlslShaderNameBuffer, IM_ARRAYSIZE(hlslShaderNameBuffer), inputFlags, CharFilter);
		// 入力されたシェーダー名の検証
		bool isValidPath = true;
		fs::path newShaderPath = hlslShaderCreationInitDir / (std::string(hlslShaderNameBuffer) + templateExtensions[selectedTemplate]);
		const char8_t* message;
		if (hlslShaderNameBuffer[0] == '\0')
		{
			message = u8"名前を入力してください。";
			isValidPath = false;
		}
		else if (fs::exists(newShaderPath))
		{
			message = u8"同名のファイルが既に存在します。";
			isValidPath = false;
		}
		else
		{
			message = u8"";
		}
		// 作成されるファイルパスを表示
		ImGui::TextWrapped(reinterpret_cast<const char*>(u8"作成されるファイル: %s"), newShaderPath.string().c_str());
		// エラーメッセージを表示
		ImGui::TextWrapped(reinterpret_cast<const char*>(message));
		ImGui::Spacing(); // スペースを空ける
		ImVec2 buttonSize(160, 0);
		// 作成ボタン
		ImGui::BeginDisabled(!isValidPath);
		if (ImGui::Button(reinterpret_cast<const char*>(u8"作成"), buttonSize) || (confirm && isValidPath))
		{
			CreateHlslShader(hlslShaderCreationInitDir, hlslShaderNameBuffer, templateExtensions[selectedTemplate]);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		// キャンセルボタン
		ImGui::SameLine();
		if (ImGui::Button(reinterpret_cast<const char*>(u8"キャンセル"), buttonSize) || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

// 新規シーン作成モーダルの内容
void AssetBrowser::DrawNewSceneCreationModal()
{
	if (showNewSceneCreationModal)
	{
		ImGui::OpenPopup("Create New Scene");
		strncpy_s(sceneNameBuffer, NewSceneName, IM_ARRAYSIZE(sceneNameBuffer)); // バッファに初期値をセット
		showNewSceneCreationModal = false; // フラグをリセット
	}
	else if (!ImGui::IsPopupOpen("Create New Scene"))
	{
		return; // モーダルが開いていない場合は何もしない
	}

	// モーダルの内容
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always); // サイズは内容に合わせて自動調整

	if (ImGui::BeginPopupModal("Create New Scene", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings))
	{
		// シーンテンプレートの選択
		static int selectedTemplate = 0;
		static const char* templates[] = {
			"Empty Scene",
			"Default Scene",
			"Custom Template",
		};
		static const char8_t* templateDescriptions[] = {
			u8"空のシーン。何も配置されていません。",
			u8"デフォルトのシーン。カメラとライトが配置された状態です。",
			u8"カスタムテンプレート。ユーザーが作成したシーンテンプレートを使用します。"
		};
		static fs::path templateScenePaths[] = {
			"./Assets/Scenes/EmptyScene.scene",
			"./Assets/Scenes/DefaultScene.scene",
			"./Assets/Scenes/"
		};
		ImGui::Text(reinterpret_cast<const char*>(u8"テンプレート:"));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-1);
		if (ImGui::BeginCombo("##SceneTemplate", templates[selectedTemplate]))
		{
			for (int i = 0; i < IM_ARRAYSIZE(templates); i++)
			{
				bool isSelected = (selectedTemplate == i);
				if (ImGui::Selectable(templates[i], isSelected))
				{
					selectedTemplate = i;
				}
				if (ImGui::IsItemHovered())
				{
					// ツールチップでテンプレートの説明を表示
					ImGui::SetTooltip("%s", templateDescriptions[i]);
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if (selectedTemplate == 2) // カスタムテンプレートが選択された場合は、テンプレートディレクトリを選択するUIを表示
		{
			ImGui::Text(reinterpret_cast<const char*>(u8"テンプレートシーン:"));
			ImGui::SameLine();
			ImGui::SetNextItemWidth(-1);
			if (ImGui::Button(reinterpret_cast<const char*>(u8"選択")))
			{
				// ファイルダイアログを開いてシーンファイルを選択させる
				char* resultPath = OpenFileDialog(
					"Scene Files (*.scene)\0*.scene\0All Files (*.*)\0*.*\0"
					"Select Scene Template"
				);
				std::string selectedPath = resultPath ? std::string(resultPath) : "";
				if (!selectedPath.empty())
				{
					templateScenePaths[2] = selectedPath; // カスタムテンプレートのパスを更新
				}
			}
			if (templateScenePaths[2].has_filename())
			{
				ImGui::TextWrapped(reinterpret_cast<const char*>(u8"選択されたテンプレート: %s"), templateScenePaths[2].string().c_str());
			}
			else
			{
				ImGui::TextWrapped(reinterpret_cast<const char*>(u8"シーンテンプレートとして使用するシーンファイルを選択してください。"));
			}
		}


		// テンプレート選択とシーン名入力の間にスペースを空ける
		ImGui::Spacing();
		// シーン名入力フィールド
		ImGui::Text(reinterpret_cast<const char*>(u8"シーン名:"));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-1); // ウィンドウ幅いっぱいに入力フィールドを広げる
		ImGuiInputFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_ElideLeft;
		bool confirm = ImGui::InputText("##SceneName", sceneNameBuffer, IM_ARRAYSIZE(sceneNameBuffer), inputFlags);
		const char8_t* message;
		bool isValid = true;
		fs::path newScenePath = sceneCreationInitDir / (std::string(sceneNameBuffer) + ".scene");
		if (selectedTemplate == 2 && (!templateScenePaths[2].has_filename() || templateScenePaths[2].extension() != ".scene"))
		{
			message = u8"有効なシーンテンプレートを選択してください。";
			isValid = false;
		}
		else if (sceneNameBuffer[0] == '\0')
		{
			message = u8"シーン名を入力してください。";
			isValid = false;
		}
		else if (fs::exists(newScenePath))
		{
			message = u8"同名のファイルが既に存在します。";
			isValid = false;
		}
		else if (!newScenePath.has_stem() || newScenePath.extension() != ".scene")
		{
			message = u8"無効なシーン名です。拡張子は .scene にしてください。";
			isValid = false;
		}
		else
		{
			message = u8"";
		}
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImGui::TextWrapped(reinterpret_cast<const char*>(message));
		ImGui::PopStyleColor();

		ImGui::Spacing();
		ImVec2 buttonSize(160, 0);
		ImGui::BeginDisabled(!isValid);
		if (ImGui::Button(reinterpret_cast<const char*>(u8"作成"), buttonSize) || (confirm && isValid))
		{
			CreateNewScene(templateScenePaths[selectedTemplate], newScenePath);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();

		if (ImGui::Button(reinterpret_cast<const char*>(u8"キャンセル"), buttonSize) || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

}

// 削除確認モーダルの内容
void AssetBrowser::DrawDeleteConfirmModal()
{
	if (showDeleteConfirmModal)
	{
		ImGui::OpenPopup("Delete Confirmation");
		showDeleteConfirmModal = false; // フラグをリセット
	}
	else if (!ImGui::IsPopupOpen("Delete Confirmation"))
	{
		return; // モーダルが開いていない場合は何もしない
	}

	// モーダルの内容
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_Appearing); // サイズは内容に合わせて自動調整

	if (ImGui::BeginPopupModal("Delete Confirmation", nullptr,
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted(reinterpret_cast<const char*>(u8"以下のアイテムを削除しますか？"));
		ImGui::Spacing();
		if (selectedAssets.size() == 1)
		{
			// 単一選択の場合は対象のファイル/フォルダ名を表示
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f)); // 警告色
			ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
			ImGui::TextUnformatted(reinterpret_cast<const char*>(deleteTargetAsset.u8string().c_str()));
			ImGui::PopTextWrapPos();
			ImGui::PopStyleColor();

			// ディレクトリの場合は追加警告
			if (fs::is_directory(deleteTargetAsset))
			{
				ImGui::Spacing();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
				ImGui::TextUnformatted(reinterpret_cast<const char*>(u8"フォルダごと中身もすべて削除されます。"));
				ImGui::PopStyleColor();
			}
		}
		else
		{
			// 複数選択の場合は、件数と先頭数件をプレビュー表示
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f)); // 警告色
			ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
			ImGui::Text(reinterpret_cast<const char*>(u8"%zu 個のアイテム"), selectedAssets.size());
			ImGui::PopTextWrapPos();
			ImGui::PopStyleColor();

			ImGui::Spacing();
			int preview = 0;
			for (const auto& asset : selectedAssets)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));
				ImGui::TextUnformatted(("  " + std::string(reinterpret_cast<const char*>(asset.filename().u8string().c_str()))).c_str());
				ImGui::PopStyleColor();
				if (++preview >= 5 && selectedAssets.size() > 5)
				{
					ImGui::TextDisabled(reinterpret_cast<const char*>(u8"  ... 他 %zu 個"), selectedAssets.size() - 5);
					break;
				}
			}
			// フォルダが含まれている場合は追加警告
			bool hasDir = std::any_of(selectedAssets.begin(), selectedAssets.end(),
				[](const fs::path& p) { return fs::is_directory(p); });
			if (hasDir)
			{
				ImGui::Spacing();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
				ImGui::TextUnformatted(reinterpret_cast<const char*>(u8"フォルダが含まれます。中身もすべて削除されます。"));
				ImGui::PopStyleColor();
			}
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImVec2 buttonSize(190, 0);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.18f, 0.18f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.28f, 0.28f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.60f, 0.10f, 0.10f, 1.0f));
		if (ImGui::Button(reinterpret_cast<const char*>(u8"削除する"), buttonSize)
			|| ImGui::IsKeyPressed(ImGuiKey_Enter)) // Enterでも確定
		{
			// 選択されたアセットを削除する処理
			std::unordered_set<fs::path> assets(selectedAssets.begin(), selectedAssets.end()); // 削除対象をコピーしておく（削除中に selectedAssets が変化する可能性があるため）
			for (const auto& asset : assets)
			{
				std::error_code ec;

				if (fs::is_directory(asset))
				{
					//CurryEngine::Resources::AssetDatabase::RemoveByPathPrefix(asset.string());
					fs::remove_all(asset, ec);
				}
				else
				{
					// ファイルの場合も同様に ec を渡す
					fs::remove(asset, ec);
					if (!ec) {
						OnAssetDeleted(asset);
					}
				}

				// 最終的な結果をログに出力
				if (ec) {
					LOG_ERROR("Failed to delete " + asset.string() + ": " + ec.message() + " (Code: " + std::to_string(ec.value()) + ")");
				}
				else {
					LOG_INFO(u8"Deleted: " + asset.u8string());
				}
			}
			lastClickedAsset.clear();
			Refresh();
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();

		if (ImGui::Button(reinterpret_cast<const char*>(u8"キャンセル"), buttonSize)
			|| ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

}

void AssetBrowser::ShowContextMenu(const fs::path& assetPath)
{
	// 右クリックされたアセットに応じたコンテキストメニューを表示する処理をここに実装
	// 例: ファイルをエクスプローラーで開く、削除、名前変更など
	bool isSelected = !assetPath.empty(); // assetPathが空でない場合は何かが選択されているとみなす
	fs::path folderPath = assetPath.empty() ? currentDirectory : assetPath.parent_path();

	if (ImGui::BeginPopup("AssetGridContextMenu"))
	{
		// -- 選択されたアセットがある場合に表示されるメニュー項目 ----------------------
		if (isSelected)
		{
			auto meta = CurryEngine::Resources::AssetDatabase::FindByPath(assetPath); // アセットのパスからアセットデータを取得する処理
			if (meta && CurryEngine::Resources::ImporterRegistry::Find(meta->type))
			{
				if (ImGui::MenuItem("Open Import Settings"))
				{
					CurryEngine::Resources::ImportSettingsWindow::Open(meta->id); // インポート設定ウィンドウを開く処理
				}
			}

			if (ImGui::MenuItem("Open"))
			{
				OpenAsset(assetPath); // アセットを開く処理
			}
			//if (ImGui::MenuItem("Show in Explorer"))
			//{
			//	ShowInExplorer(assetPath); // エクスプローラーで表示する処理
			//}
			if (ImGui::MenuItem("Rename"))
			{
				StartRename(assetPath); // リネーム処理
			}
			if (ImGui::MenuItem("Delete"))
			{
				StartDelete(assetPath); // 削除処理
			}

			ImGui::Separator(); // 選択されたアセットとフォルダ両方に共通のメニュー項目を分けるためのセパレーター
		}

		// -- 常に表示されるメニュー項目（フォルダの作成、シーンの作成、スクリプトの作成など） ----------------------
		if (ImGui::MenuItem("Create Folder"))
		{
			fs::create_directory(folderPath / "New Folder");
			Refresh();
		}
		if (ImGui::MenuItem("Create Scene"))
		{
			ShowNewSceneCreationModal(folderPath);
		}
		if (ImGui::MenuItem("Create C# Script"))
		{
			ShowScriptCreationModal(folderPath);
		}
		if (ImGui::MenuItem("Create HLSL Shader"))
		{
			ShowHlslShaderCreationModal(folderPath);
		}
		if (ImGui::MenuItem("Create Material"))
		{
			// 新しいマテリアルを作成する処理
			fs::path newMaterialPath = MakeUniqueFilePath(folderPath, "New Material", ".mat");
			std::ofstream ofs(newMaterialPath);
			ofs << "{}"; // 空のマテリアルデータをJSON形式で書き込む
			ofs.close();
			Refresh();
		}
		ImGui::EndPopup();
	}

}

#endif // DEBUG

fs::path AssetBrowser::MakeUniqueFilePath(const fs::path& dir, const fs::path& stem, const std::string& extension)
{
	fs::path newPath = dir / stem;
	newPath += extension;
	int suffix = 1;
	while (fs::exists(newPath))
	{
		newPath = dir / stem;
		newPath += "_" + std::to_string(suffix);
		suffix++;
	}
	return newPath;
}

fs::path AssetBrowser::ToUnityStylePath(const fs::path& path) {
	fs::path result = path;

	// Windows用の \ を / に統一
	//std::replace(result.begin(), result.end(), '\\', '/');

	// スラッシュで分割し、" > " で連結
	std::wstringstream ws(result.wstring());
	std::wstring segment;
	std::vector<std::wstring> segments;

	while (std::getline(ws, segment, L'\\')) {
		if (!segment.empty() && segment != L".") {
			segments.push_back(segment);
		}
	}

	result.clear();
	for (size_t i = 0; i < segments.size() - 1; ++i)
	{
		result += segments[i] + L">" + segments[i + 1];
	}
	return result;
}

bool AssetBrowser::MoveAssetToFolder(const fs::path& srcPath, const fs::path& dstFolderPath)
{
	std::filesystem::path src(srcPath);
	std::filesystem::path dst = dstFolderPath;
	dst /= src.filename(); // 元のファイル名を維持

	bool success = false;
	try {
		std::filesystem::rename(src, dst);
		Refresh(); // ブラウザを更新して変更を反映
		success = true;
	}
	catch (std::filesystem::filesystem_error& e) {
		// ログやエラーメッセージを表示
		std::cerr << "移動失敗: " << e.what() << std::endl;
		success = false;
	}

	// 移動に成功した場合は、アセットデータベースも更新
	/*if (success) {
		CurryEngine::Resources::AssetDatabase::Rename(src, dst);
	}*/

	return success;
}

bool AssetBrowser::MoveFolderToFolder(const fs::path& source, const fs::path& destinationParent)
{
	try {
		if (!fs::exists(source) || !fs::is_directory(source)) return false;
		if (!fs::exists(destinationParent) || !fs::is_directory(destinationParent)) return false;

		// フォルダ名を取得して移動先パスを作る
		fs::path sourcePath(source);
		fs::path destPath = fs::path(destinationParent) / sourcePath.filename();

		fs::rename(sourcePath, destPath);
		Refresh(); // ブラウザを更新して変更を反映
		//CurryEngine::Resources::AssetDatabase::RemapPathPrefix(sourcePath, destPath);
		return true;
	}
	catch (const std::exception& e) {
		// ログ出力など
		std::cerr << "MoveFolder error: " << e.what() << std::endl;
		LOG_ERROR(std::format("MoveFolder error: {}", e.what()));
		return false;
	}
}

#ifdef USE_IMGUI
bool AssetBrowser::HandleDropTargetForFolder(const fs::path& targetFolderPath)
{
	bool result = ImGui::BeginDragDropTarget();

	if (result)
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH"))
		{
			const char* assetPath = (const char*)payload->Data;
			MoveAssetToFolder(assetPath, targetFolderPath);
		}
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FOLDER_PATH"))
		{
			const char* folderPath = (const char*)payload->Data;
			MoveFolderToFolder(folderPath, targetFolderPath);
		}
		ImGui::EndDragDropTarget();
	}
	return result;

}
#endif // USE_IMGUI