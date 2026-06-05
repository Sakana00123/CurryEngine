#pragma once
#include <filesystem>
#include <wrl.h>
#include <d3d11.h>
#include "Engine/Core/Misc.h"
#include <unordered_map>
#include <unordered_set>
#include "AssetBrowserDropTarget.h"
#pragma comment(lib, "Ole32.lib") // IDropTargetの実装に必要

namespace fs = std::filesystem;

enum class AssetType
{
	Unknown,
	Texture,
	GltfModel,
	Sound,
	Scene,
	Prefab,
	Script,
	Shader,
};

// fs::pathをキーにしたunordered_mapやunordered_setで必要なハッシュ関数
struct FsPathHash
{
	std::size_t operator()(const fs::path& path) const
	{
		return fs::hash_value(path);
	}
};


/**
 * @brief アセットブラウザクラス
 * - アセットの表示、選択、操作（リネーム、削除、ドラッグ＆ドロップなど）を管理
 * - アセットの種類をファイル拡張子から判別
 * - アセットのサムネイル表示（テクスチャは内容を、その他はアイコンを表示）
 * - アセットのダブルクリックでのオープン（シーンならシーンを、プレハブならプレハブエディタを開く）
 * - アセットの右クリックでのコンテキストメニュー（リネーム、削除、コピー、移動など）
 * - アセットのドラッグ＆ドロップでの移動（フォルダ間の移動や、外部からのファイルのインポート）
 * - アセットの検索機能（キーワードでフィルタリング）
 * - アセットのキャッシュ管理（検索結果のキャッシュや、サムネイルのキャッシュなど）
 * - アセットの新規作成（例：シーンやスクリプトのテンプレートからの作成）
 * - アセットの削除確認（重要なアセットを誤って削除しないように確認ダイアログを表示）
 * - アセットのリネーム機能（選択したアセットの名前を変更）
 * - アセットのインポート機能（外部からファイルをドラッグ＆ドロップしてアセットとして取り込む）
 */
class AssetBrowser
{
public:
	struct GuiSettings
	{
		bool acceptDropToFolderTree = false;
		bool acceptDropToAssetGrid = false;
		bool acceptDropToCurrentPath = false;
	};
public:
	// アセットブラウザの初期化（アイコンの読み込みなど）
	static void Initialize();

	// ドロップターゲットの初期化（Windowsのドラッグ＆ドロップAPIを使用する場合）
	static void InitializeDropTarget(HWND hwnd);

	// アセットブラウザの終了処理（リソースの解放など）
	static void FinalizeDropTarget(HWND hwnd);

	// ドロップされたファイルの処理（Windowsのドラッグ＆ドロップAPIを使用する場合）
	static void OnDropFiles(HWND hwnd, HDROP hDrop);

	static AssetType DetectAssetTypeFromFile(const fs::path& path);

	static void DrawGUI();

	static void Show() { isOpen = true; }

	// キャッシュをクリア（例：アセットの追加・削除・移動後などに呼び出す）
	static void Refresh();

	// 外部からのドラッグがアセットグリッド上にあるかどうか
	static bool IsExternalDragActive() {
		return isExternalDragActive;
	}

public:

	// アセットのリネームを開始
	static void StartRename(const fs::path& assetPath);

	// アセットの削除を開始
	static void StartDelete(const fs::path& assetPath);

	// スクリプト作成モーダルを表示
	static void ShowScriptCreationModal(const fs::path& initDir);

	// 指定したディレクトリにC#スクリプトを作成
	static void CreateCSharpScript(const fs::path& directory, const std::string& scriptName);

	// HLSLシェーダー作成モーダルを表示
	static void ShowHlslShaderCreationModal(const fs::path& initDir);

	// 指定したディレクトリにHLSLシェーダーを作成
	static void CreateHlslShader(const fs::path& directory, const std::string& shaderName, const std::string& extension);
	

	// 新規シーン作成モーダルを表示
	static void ShowNewSceneCreationModal(const fs::path& initDir);

	// 新規シーンを作成
	static void CreateNewScene(const fs::path& templateScenePath, const fs::path& newScenePath);

	// アセットをダブルクリックしたときの処理
	static void OpenAsset(const fs::path& assetPath);

	// Assetsフォルダ内を検索してkeywordにマッチするファイルをresultsに追加する（キャッシュを利用）
	static inline void SearchAssets(const fs::path& root, const std::string& keyword, std::vector<fs::directory_entry>& results);

private:

	// アセットの削除後の処理
	static void OnAssetDeleted(const fs::path& assetPath);

#ifdef USE_IMGUI

	static void DrawFolderTree(const std::filesystem::path& root, std::filesystem::path& selectedFolder);

	static void DrawAssetGrid(const std::filesystem::path& folderPath, const char* filter = "");

	static void DrawUnityPath(const std::string& path);

	// スクリプト作成モーダルの描画
	static void DrawScriptCreationModal();

	// HLSLシェーダー作成モーダルの描画
	static void DrawHlslShaderCreationModal();

	// 新規シーン作成モーダルの描画
	static void DrawNewSceneCreationModal();

	// 削除確認モーダルの描画
	static void DrawDeleteConfirmModal();


	// アセットを右クリックしたときのコンテキストメニューの処理
	static void ShowContextMenu(const fs::path& assetPath);

	//ドロップ先のターゲットの処理（戻り値：アセットドラッグ中に直前に描画されたGUIをホバー中か）
	static bool HandleDropTargetForFolder(const std::string& targetFolderPath);
#endif // USE_IMGUI

	static fs::path MakeUniqueFilePath(const fs::path& dir, const std::string& stem, const std::string& extension = ".cs");
	static std::string ToUnityStylePath(const std::string& path);
	static bool MoveAssetToFolder(const std::string& srcPath, const std::string& dstFolderPath);
	static bool MoveFolderToFolder(const std::string& source, const std::string& destinationParent);
private:
	static inline bool isOpen = true;
	static inline GuiSettings settings;

	// Script用
	static inline bool showScriptCreationModal = false;
	static constexpr char NewScriptName[128] = "NewScript"; // スクリプト作成モーダルを開くときの初期名
	static inline char scriptNameBuffer[128] = ""; // スクリプト作成モーダルの入力バッファ
	static inline fs::path scriptCreationInitDir;

	// スクリプトテンプレートのパス
	//static constexpr char ScriptTemplatePath[] = "./Assets/ScriptTemplates/NewScript.cs.tmp";

	// HLSLシェーダー用
	static inline bool showHlslShaderCreationModal = false;
	static constexpr char NewHlslShaderName[128] = "NewShader"; // HLSLシェーダー作成モーダルを開くときの初期名
	static inline char hlslShaderNameBuffer[128] = ""; // HLSLシェーダー作成モーダルの入力バッファ
	static inline fs::path hlslShaderCreationInitDir; // HLSLシェーダー作成モーダルを開くときの初期ディレクトリ

	// Scene用
	static inline bool showNewSceneCreationModal = false;
	static constexpr char NewSceneName[128] = "NewScene"; // シーン作成モーダルを開くときの初期名
	static inline char sceneNameBuffer[128] = ""; // シーン作成モーダルの入力バッファ
	static inline fs::path sceneCreationInitDir = "./Assets/Scenes"; // シーン作成モーダルを開くときの初期ディレクトリ


	// アセットブラウザ全体で使用する状態
	static inline float thumbnailSize = 64.0f;      // サムネイルサイズ（スライダー用）
	static inline bool isRenaming = false;          // リネーム中フラグ
	static inline bool renamingJustStarted = false; // フォーカスセット用・初回フレームフラグ
	static inline char renameBuffer[256] = "";      // リネーム入力バッファ
	static inline fs::path renamingTarget;          // リネーム対象パス

	static inline bool showDeleteConfirmModal = false; // 削除確認モーダルを表示するかどうかのフラグ
	static inline fs::path deleteTargetAsset; // 削除対象のアセットパス（モーダルで使用）

	// 外部からのファイルドロップを処理するための状態
	static inline RECT assetGridScreenRect = { 0,0,0,0 }; // アセットグリッドのスクリーン座標（ドラッグ＆ドロップのドロップターゲット判定に使用）
	static inline bool isExternalDragHovering = false; // 外部からのドラッグがアセットグリッド上にあるかどうかのフラグ
	static inline bool isExternalDragActive = false; // 外部からのドラッグが現在アクティブかどうかのフラグ（ドラッグ中はSleepを抑制するため）
	static inline AssetBrowserDropTarget* dropTarget = nullptr; // ドロップターゲットのインスタンス（Windowsのドラッグ＆ドロップAPIを使用する場合）
	static inline bool oleInitializedHere = false; // AssetBrowserがOLEを初期化したかどうかのフラグ（FinalizeDropTargetでのみ解放するため）
private:
	static inline std::filesystem::path currentDirectory;
	static inline std::filesystem::path assetPath;
	static inline std::filesystem::path s_AssetPath = "./Assets/";
	static inline Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> directoryIcon;
	static inline Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> fileIcon;
	static inline std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> images;
	static inline std::unordered_map<fs::path, std::unordered_map<std::string, std::vector<fs::directory_entry>>> cacheSearchResults;

	static inline std::unordered_set<fs::path, FsPathHash> selectedAssets; // 複数選択されたアセットのパスを保持するためのunordered_set
	static inline fs::path lastClickedAsset; // 最後に選択されたアセットのパス（Shift+クリックで範囲選択する際の基準点）
	static inline std::vector<fs::path> lastResultOrder; // 最後に検索結果として表示されたアセットの順序を保持するためのベクター（Shift+クリックで範囲選択する際の順序基準）
};