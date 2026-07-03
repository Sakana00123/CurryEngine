#include "pch.h"
#include "BuildSettingsWindow.h"
#include "Engine/Editor/Console.h"
#include "Engine/Editor/Dialog.h"
#include <format>
#include <thread>
#include "Engine/EditorSupport/ImGuiHelpers.h"
#include "Engine/Scenes/SceneManager.h"
#include "Engine/Physics/Physics.h"
#ifdef USE_IMGUI
#include <imgui_internal.h>
#endif // USE_IMGUI


// バックスラッシュをスラッシュに統一（.rc はスラッシュが無難）
static std::string NormalizePath(const std::string& path) {
	std::string result = path;
	std::replace(result.begin(), result.end(), '\\', '/');
	return result;
}

// Shift-JIS (CP932) → UTF-8 変換ヘルパー
static std::string SjisToUtf8(const std::string& sjis)
{
	if (sjis.empty()) return {};

	// SJIS → UTF-16
	int wlen = MultiByteToWideChar(932, 0, sjis.c_str(), (int)sjis.size(), nullptr, 0);
	std::wstring wide(wlen, L'\0');
	MultiByteToWideChar(932, 0, sjis.c_str(), (int)sjis.size(), wide.data(), wlen);

	// UTF-16 → UTF-8
	int ulen = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), wlen, nullptr, 0, nullptr, nullptr);
	std::string utf8(ulen, '\0');
	WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), wlen, utf8.data(), ulen, nullptr, nullptr);

	return utf8;
}

void BuildSettingsWindow::Show()
{
	m_showWindow = true;
	LoadBuildSettings();
}

void BuildSettingsWindow::DrawGUI()
{
#ifdef USE_IMGUI
	if (!m_showWindow) return;
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking;
	// ウィンドウを中央に表示
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(1200, 500), ImGuiCond_FirstUseEver);
	ImGui::Begin("Build Settings", &m_showWindow, windowFlags);
	{
		DrawSettings();
		ImGui::Separator();
		DrawBuildButton();

		ImGui::Spacing();
		if (m_isBuilding || m_buildProgress.finished)
		{
			// mutex 越しにコピー
			ProcessProgress snap;
			{ std::lock_guard lock(m_progressMutex); snap = m_buildProgress; }
			DrawProgressPanel("##build_log", snap, m_isBuilding);
		}
		if (m_isPackaging || m_packageProgress.finished)
		{
			ImGui::SeparatorText("Package Progress");
			ProcessProgress snap;
			{ std::lock_guard lock(m_progressMutex); snap = m_packageProgress; }
			DrawProgressPanel("##pack_log", snap, m_isPackaging);
		}
	}
	ImGui::End();

	// ウィンドウが閉じられたときに設定を保存する
	if (!m_showWindow)
	{
		SaveBuildSettings();
	}

#endif // USE_IMGUI
}

#ifdef USE_IMGUI

void BuildSettingsWindow::DrawSettings()
{
	// シーンのリストを描画
	DrawScenesInBuild("Scenes In Build");

	IMGUI_PROPERTY_BEGIN();

	bool appNameChanged = false;
	IMGUI_PROPERTY_STRING("App Name", appName, MAX_PATH, appNameChanged);
	if (appNameChanged)
	{
		m_settings.appName = SjisToUtf8(appName);
	}

	bool iconPathChanged = false;
	const char* filter = "Icon Files(*.ico)\0*.ico\0All Files(*.*)\0*.*;\0\0";
	IMGUI_PROPERTY_STRING_WITH_DIALOG("Icon Path", iconPath, MAX_PATH, filter, iconPathChanged);
	if (iconPathChanged)
	{
		m_settings.iconPath = SjisToUtf8(iconPath);
	}

	//DrawFileSelector("Icon Path", m_settings.iconPath, "*.ico");
	DrawDirectorySelector("Output Directory", m_settings.outputDir);

	IMGUI_PROPERTY_END();

	// コピーアイテムのリストを描画
	ImGui::SeparatorText("Copy Items");
	for (size_t i = 0; i < m_settings.copyItems.size(); i++)
	{
		auto& item = m_settings.copyItems[i];
		std::string label = std::format("Copy Item {}", i + 1);
		ImGui::PushID(static_cast<int>(i));
		ImGui::Text("%s", label.c_str());
		ImGui::SameLine();
		if (ImGui::Button("Remove"))
		{
			m_settings.copyItems.erase(m_settings.copyItems.begin() + i);
			ImGui::PopID();
			break;
		}
		ImGui::SameLine();
		bool srcChanged = false;
		const char* filter = "All Files(*.*)\0*.*;\0\0";
		IMGUI_PROPERTY_BEGIN();
		srcChanged = DrawFileSelector("Source", item.src, filter);
		IMGUI_PROPERTY_END();
		if (srcChanged)
		{
			item.src = SjisToUtf8(item.src);
		}
		bool typeChanged = false;
		const char* types[] = { "file", "folder", "glob" };
		int currentTypeIndex = static_cast<int>(std::find(std::begin(types), std::end(types), item.type) - std::begin(types));
		if (ImGui::Combo("Type", &currentTypeIndex, types, IM_ARRAYSIZE(types)))
		{
			item.type = types[currentTypeIndex];
		}
		if (item.type == "folder")
		{
			ImGui::Text("Exclude Extensions (for folder type)");
			for (size_t j = 0; j < item.exclude.size(); j++)
			{
				ImGui::PushID(static_cast<int>(j));
				IMGUI_PROPERTY_BEGIN();
				IMGUI_PROPERTY(("[" + std::to_string(j) + "]").c_str());
				const size_t bufferSize = MAX_PATH;
				char buffer[MAX_PATH] = {};
				// 現在のパスをバッファにコピーする（Shift-JIS で）
				strncpy_s(buffer, item.exclude[j].c_str(), bufferSize - 1);
				if (ImGui::InputText("##file", buffer, bufferSize))
				{
					// ユーザーが直接テキストを編集してパスを変更したときの処理
					item.exclude[j] = SjisToUtf8(buffer);
				}
				IMGUI_PROPERTY_END();
				
				if (ImGui::Button("Remove"))
				{
					item.exclude.erase(item.exclude.begin() + j);
					ImGui::PopID();
					break;
				}
				ImGui::PopID();
			}
			if (ImGui::Button("Add Exclude Extension"))
			{
				item.exclude.push_back("");
			}
		}
		ImGui::Separator();
		ImGui::PopID();
	}

}

void BuildSettingsWindow::DrawScenesInBuild(const char* label)
{
	// ビルドに含めるシーンのセレクタの実装
	ImGui::Text("%s", label);
	// ドロップターゲットを作成
	ImGui::BeginChild("SceneList", ImVec2(0, 150), ImGuiChildFlags_Borders);
	{
		bool removeThisFrame = false; // 今フレームで削除するシーンがあるかどうか
		std::string setFirstSceneCandidate = SceneManager::firstSceneName; // 最初のシーンに設定する候補（右クリックメニューで選択されたシーン名）
		int setFirstSceneIndexThisFrame = -1; // 今フレームで最初のシーンに設定するシーンのインデックス（右クリックメニューで選択されたシーンのインデックス）
		int index = 0;
		int selectedCount = static_cast<int>(std::count_if(SceneManager::sceneEntries.begin(), SceneManager::sceneEntries.end(),
			[](const SceneManager::SceneEntry& entry) { return entry.selected; })); // 選択されているシーンの数

		for (auto& entry : SceneManager::sceneEntries)
		{
			ImGui::PushID(index);

			// チェックボックスとシーン名を同じ行に表示
#if 0 // ビルドに含めるかどうかのチェックボックスは今のところ不要そうなので非表示にする
			ImGui::Checkbox("##includeInBuild", &entry.enabled);
			ImGui::SameLine();
#endif // 0
			ImGuiSelectableFlags flags = ImGuiSelectableFlags_SelectOnClick;
			if (!entry.enabled)
			{// 無効なシーンは選択できないようにする
				flags |= ImGuiSelectableFlags_Disabled;
			}
			// シーン名を表示
			bool wasSelected = entry.selected;
			if (ImGui::Selectable(entry.name.c_str(), &wasSelected, flags))
			{
				// クリックされたときの処理
				bool ctrl = ImGui::GetIO().KeyCtrl;
				bool shift = ImGui::GetIO().KeyShift;
				if (shift)
				{
					// Shift キーが押されているときは範囲選択する
					if (SceneManager::sceneEntries.size() > 1)
					{
						// 最後に選択されたエントリを見つける
						auto lastSelectedIt = std::find_if(SceneManager::sceneEntries.begin(), SceneManager::sceneEntries.end(),
							[](const SceneManager::SceneEntry& e) { return e.selected; });
						if (lastSelectedIt != SceneManager::sceneEntries.end())
						{
							// 範囲選択する
							int lastSelectedIndex = static_cast<int>(std::distance(SceneManager::sceneEntries.begin(), lastSelectedIt));
							int currentIndex = index;
							int start = (std::min)(lastSelectedIndex, currentIndex);
							int end = (std::max)(lastSelectedIndex, currentIndex);
							for (int i = start; i <= end; i++)
							{
								SceneManager::sceneEntries[i].selected = true;
							}
						}
						else
						{
							entry.selected = true; // 最後に選択されたエントリがない場合は単純に選択状態にする
						}
					}
					else
					{
						entry.selected = true; // シーンが1つしかない場合は単純に選択状態にする
					}
				}
				else
				{
					if (ctrl)
					{
						// Ctrl キーが押されているときは選択状態をトグルする
						entry.selected = !entry.selected;
					}
					else
					{
						// Ctrl キーが押されていないときは単一選択にする
						for (auto& otherEntry : SceneManager::sceneEntries)
						{
							if (&otherEntry != &entry)
							{
								otherEntry.selected = false;
							}
						}
						entry.selected = true; // クリックされたエントリを選択状態にする
					}
				}

			}

			// 選択されてる時の処理
			if (entry.selected)
			{
				// Delete キーが押されたときに削除する
				if (ImGui::Shortcut(ImGuiKey::ImGuiKey_Delete, ImGuiInputFlags_None))
				{
					removeThisFrame = true; // 今フレームで削除するフラグを立てる
				}

				// シーンが選択されている状態で右クリックされたときにコンテキストメニューを表示
				if (ImGui::BeginPopupContextItem(entry.name.c_str(), ImGuiPopupFlags_MouseButtonRight))
				{
					bool isFirstScene = (entry.name == SceneManager::firstSceneName);
					if (ImGui::MenuItem("Set First Scene", NULL, isFirstScene, selectedCount == 1))
					{
						setFirstSceneIndexThisFrame = index; // 今フレームで最初のシーンに設定するシーンのインデックスを保存
					}
					bool isLoadingScene = (entry.name == SceneManager::loadingSceneName);
					if (ImGui::MenuItem("Set Loading Scene", NULL, isLoadingScene, selectedCount == 1))
					{
						SceneManager::SetLoadingScene(entry.name); // 今フレームで最初のシーンに設定するシーンのインデックスを保存
					}

					if (ImGui::MenuItem("Remove", "Delete"))
					{
						removeThisFrame = true; // 今フレームで削除するフラグを立てる
						if (isLoadingScene)
						{
							SceneManager::SetLoadingScene("EmptyScene"); // ローディングシーンに設定されているシーンが削除される場合は、EmptyScene
						}
					}
					ImGui::EndPopup();
				}
			}
			ImGui::PopID();
			index++;
		}
		if (removeThisFrame)
		{
			// 選択されているシーンを削除する
			SceneManager::sceneEntries.erase(
				std::remove_if(SceneManager::sceneEntries.begin(), SceneManager::sceneEntries.end(),
					[](const SceneManager::SceneEntry& entry) { return entry.selected; }),
				SceneManager::sceneEntries.end()
			);
			SceneManager::UpdateFirstSceneName(); // 最初のシーンの名前を更新する
		}
		if (setFirstSceneIndexThisFrame != -1)
		{
			// 最初のシーンに設定する候補が今フレームで変更されている場合は、最初のシーンを更新する
			SceneManager::SetFirstScene(SceneManager::sceneEntries[setFirstSceneIndexThisFrame].name);
		}

		// ウィンドウ直下でクリックされたときに全選択を解除するための処理
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
		{
			for (auto& entry : SceneManager::sceneEntries)
			{
				entry.selected = false;
			}
		}


		// ドロップターゲットの処理
		if (auto* window = ImGui::GetCurrentWindow())
		{
			if (ImGui::BeginDragDropTargetCustom(window->Rect(), window->ID))
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH"))
				{
					// ドロップされたシーンの名前を取得
					const char* path = static_cast<const char*>(payload->Data);
					std::string pathStr(path != nullptr ? path : "");
					// シーンのリストに追加
					if (!pathStr.empty())
					{
						SceneManager::Register(pathStr);
					}
				}
				ImGui::EndDragDropTarget();
			}
		}
	}
	ImGui::EndChild();
}

bool BuildSettingsWindow::DrawFileSelector(const char* label, std::string& path, const char* filter)
{
	// ファイルセレクタの実装
	IMGUI_PROPERTY(label);
	const size_t bufferSize = MAX_PATH;
	char buffer[MAX_PATH] = {};
	// 現在のパスをバッファにコピーする（Shift-JIS で）
	strncpy_s(buffer, path.c_str(), bufferSize - 1);
	if (ImGui::InputText("##file", buffer, bufferSize))
	{
		// ユーザーが直接テキストを編集してパスを変更したときの処理
		path = SjisToUtf8(buffer);
	}
	ImGui::SameLine();
	if (ImGui::Button("..."))
	{
		// ファイル選択ダイアログを開く
		char* selectedPath = SelectFileDialog(label, filter);
		if (selectedPath != nullptr)
		{
			path = SjisToUtf8(selectedPath); // 選択されたパスをpathにセットする
			return true; // パスが変更された場合はtrueを返す
		}
	}
	return false; // パスが変更されなかった場合はfalseを返す
}

bool BuildSettingsWindow::DrawDirectorySelector(const char* label, std::string& path)
{
	// ディレクトリセレクタの実装
	IMGUI_PROPERTY(label);
	const size_t bufferSize = MAX_PATH;
	char buffer[MAX_PATH] = {};
	// 現在のパスをバッファにコピーする（Shift-JIS で）
	strncpy_s(buffer, path.c_str(), bufferSize - 1);
	if (ImGui::InputText("##directory", buffer, bufferSize))
	{
		// ユーザーが直接テキストを編集してパスを変更したときの処理
		path = SjisToUtf8(buffer);
	}
	ImGui::SameLine();
	if (ImGui::Button("..."))
	{
		// ディレクトリ選択ダイアログを開く
		char* selectedPath = SelectDirectoryDialog(label);
		if (selectedPath != nullptr)
		{
			// 成功した場合はpathが更新されているはず
			std::filesystem::path fsPath(selectedPath);
			// 絶対パスが入ってるので、プロジェクトからの相対パスに変換して保存する
			std::filesystem::path projectDir = std::filesystem::absolute("./"); // プロジェクトのルートディレクトリを取得（必要に応じて変更）
			std::filesystem::path relativePath = std::filesystem::relative(fsPath, projectDir);
			path = relativePath.string(); // 選択されたパスをpathにセットする
			return true; // パスが変更された場合はtrueを返す
		}
	}
	return false; // パスが変更されなかった場合はfalseを返す
}

void BuildSettingsWindow::DrawBuildButton()
{
	if (m_isBuilding)
	{
		ImGui::BeginDisabled();
		ImGui::Button("Building...");
		ImGui::EndDisabled();
	}
	else
	{
		bool canBuild = !m_settings.appName.empty() && !m_settings.outputDir.empty() && !SceneManager::sceneEntries.empty();
		canBuild = canBuild && std::any_of(SceneManager::sceneEntries.begin(), SceneManager::sceneEntries.end(),
			[](const SceneManager::SceneEntry& entry) { return entry.enabled; }); // ビルドに含めるシーンが1つでも選択されているかどうか
		if (!canBuild)
		{
			ImGui::BeginDisabled();
			ImGui::Button("Build");
			ImGui::EndDisabled();
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Please set App Name, Output Directory, and select at least one scene to include in the build.");
			return;
		}


		if (ImGui::Button("Build"))
		{
			RunBuild();
		}
	}

	if (m_isPackaging)
	{
		ImGui::BeginDisabled();
		ImGui::Button("Packing...");
		ImGui::EndDisabled();
	}
	else
	{
		bool canBuild = !m_settings.appName.empty() && !m_settings.outputDir.empty() && !SceneManager::sceneEntries.empty();
		canBuild = canBuild && std::any_of(SceneManager::sceneEntries.begin(), SceneManager::sceneEntries.end(),
			[](const SceneManager::SceneEntry& entry) { return entry.enabled; }); // ビルドに含めるシーンが1つでも選択されているかどうか
		if (!canBuild)
		{
			ImGui::BeginDisabled();
			ImGui::Button("Pack");
			ImGui::EndDisabled();
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Please set App Name, Output Directory, and select at least one scene to include in the build.");
			return;
		}


		if (ImGui::Button("Pack"))
		{
			PackageBuildOutput();
		}
	}
}

void BuildSettingsWindow::DrawProgressPanel(const char* id, const ProcessProgress& prog, bool isRunning)
{
	// --- ステータス行 ---
	if (isRunning)
	{
		const char* spinner = "|/-\\";
		char sp = spinner[(int)(ImGui::GetTime() * 8.0f) % 4];
		ImGui::TextColored({ 1.0f, 0.8f, 0.2f, 1.0f }, "[%c] Building...", sp);
	}
	else
	{
		if (prog.succeeded)
			ImGui::TextColored({ 0.2f, 1.0f, 0.4f, 1.0f }, "[\xe2\x9c\x93] Build Succeeded");
		else
			ImGui::TextColored({ 1.0f, 0.3f, 0.3f, 1.0f }, "[\xe2\x9c\x97] Build Failed");
	}

	// --- プログレスバー ---
	float fraction = (prog.progressTotal > 0)
		? std::clamp((float)prog.progressCurrent / (float)prog.progressTotal, 0.0f, 1.0f)
		: (prog.finished ? 1.0f : 0.0f);

	char overlay[32];
	if (prog.progressTotal > 0)
		snprintf(overlay, sizeof(overlay), "%d / %d  (%.0f%%)",
			prog.progressCurrent, prog.progressTotal, fraction * 100.0f);
	else
		snprintf(overlay, sizeof(overlay), prog.finished ? "100%%" : "Waiting...");

	ImVec4 barColor = prog.finished
		? (prog.succeeded ? ImVec4{ 0.2f, 0.75f, 0.35f, 1.0f }
			: ImVec4{ 0.8f, 0.25f, 0.25f, 1.0f })
		: ImVec4{ 0.2f, 0.55f, 0.9f, 1.0f };

	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
	ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f), overlay);
	ImGui::PopStyleColor();

	ImGui::Spacing();

	// --- ログ表示 ---
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f));
	if (ImGui::BeginChild(id, { 0, 180 }, true, ImGuiWindowFlags_HorizontalScrollbar))
	{
		// mutex を最小限にしてスナップショットコピー
		std::vector<std::string> snapshot;
		{
			std::lock_guard lock(m_progressMutex);
			snapshot = prog.logs;
		}

		for (const auto& line : snapshot)
		{
			if (line.find("error") != std::string::npos ||
				line.find("Error") != std::string::npos ||
				line.find("\xe2\x9c\x97") != std::string::npos)
				ImGui::TextColored({ 1.0f, 0.4f, 0.4f, 1.0f }, "%s", line.c_str());
			else if (line.find("warning") != std::string::npos ||
				line.find("Warning") != std::string::npos)
				ImGui::TextColored({ 1.0f, 0.85f, 0.3f, 1.0f }, "%s", line.c_str());
			else if (line.find("\xe2\x9c\x93") != std::string::npos)
				ImGui::TextColored({ 0.2f, 1.0f, 0.4f, 1.0f }, "%s", line.c_str());
			else
				ImGui::TextUnformatted(line.c_str());
		}

		if (isRunning) ImGui::SetScrollHereY(1.0f);
	}
	ImGui::EndChild();
	ImGui::PopStyleColor();
}

#endif // USE_IMGUI

void BuildSettingsWindow::SaveBuildSettings()
{
	m_settings.Save();
}

void BuildSettingsWindow::LoadBuildSettings()
{
	m_settings.Load();
	// キャッシュにコピー
	appName = m_settings.appName;
	iconPath = m_settings.iconPath;
	outputDir = m_settings.outputDir;
}

void BuildSettingsWindow::RunBuild()
{
	// ビルド中はビルドボタンを無効化する
	if (m_isBuilding)
	{
		Console::LogWarning("[Build] Build is already in progress.");
		return;
	}

	std::lock_guard lock(m_progressMutex);
	m_buildProgress = {};   // リセット

	// 設定を保存してからビルドを実行する
	SaveBuildSettings();

	Physics::SaveSettings(); // 物理エンジンの状態を保存しておく。
	SceneManager::SaveSettings(); // シーンマネージャの状態を保存しておく。

	// ビルド前のフック（必要に応じてビルド前にやりたい処理があればここに書く）
	{
		// ビルドするシーンを保存しておく
		for (auto& entry : SceneManager::sceneEntries)
		{
			std::filesystem::path scenePath(entry.path);
			std::filesystem::path buildScenePath = scenePath.replace_extension(".bin");

			// シーンをビルド用の形式で保存する
			json cachedJson;
			if (JsonFileHandler::LoadJsonFromFile(cachedJson, entry.path))
			{
				JsonFileHandler::SaveJsonToFile(cachedJson, buildScenePath.string(), JsonIOFormat::Binary);
			}
		}
	}


	// ビルドコマンドを構築
	// 引数: build.bat <アプリケーション名> <アイコンパス> <ビルド出力先ディレクトリ>
	std::string cmd = std::format(
		"build.bat \"{}\" \"{}\" \"{}\"",
		m_settings.appName,
		NormalizePath(m_settings.iconPath),
		NormalizePath(m_settings.outputDir)
	);

	// --- パイプ作成 ---
	HANDLE hReadPipe = nullptr;
	HANDLE hWritePipe = nullptr;

	SECURITY_ATTRIBUTES sa{};
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;   // 子プロセスにハンドルを継承させる
	sa.lpSecurityDescriptor = nullptr;

	if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
	{
		Console::LogError("[Build] Failed to create pipe.");
		return;
	}

	// 親側の読み取りハンドルは継承不要
	SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

	// --- STARTUPINFO にパイプを接続 ---
	STARTUPINFOA si{};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = hWritePipe;   // stdout → パイプ
	si.hStdError = hWritePipe;   // stderr も同じパイプへ

	PROCESS_INFORMATION pi{};

	if (!CreateProcessA(
		nullptr, cmd.data(),
		nullptr, nullptr,
		TRUE,               // ハンドル継承を有効化
		CREATE_NO_WINDOW,   // コンソール非表示
		//CREATE_NEW_CONSOLE, // 新しいコンソールで開く
		nullptr, nullptr,
		&si, &pi))
	{
		Console::LogError("[Build] Failed to start build process.");
		CloseHandle(hReadPipe);
		CloseHandle(hWritePipe);
		return;
	}

	// 子プロセスが書き込み側ハンドルを持つので、親側は閉じる
	// ※ これをしないと ReadFile がブロックしたままになる
	CloseHandle(hWritePipe);

	m_isBuilding = true;
	Console::Log("[Build] Build started.");

	// --- 読み取りスレッド ---
	std::thread([this, hReadPipe, pi]() mutable
		{
			auto pushLog = [this](std::string line)
				{
					std::lock_guard lock(m_progressMutex);
					m_buildProgress.logs.push_back(std::move(line));
				};

			std::string remainder;
			char buffer[512]{};
			DWORD bytesRead = 0;

			while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0)
			{
				buffer[bytesRead] = '\0';
				remainder += SjisToUtf8(std::string(buffer, bytesRead));

				size_t pos;
				while ((pos = remainder.find('\n')) != std::string::npos)
				{
					std::string line = remainder.substr(0, pos);
					if (!line.empty() && line.back() == '\r') line.pop_back();
					remainder = remainder.substr(pos + 1);
					if (line.empty()) continue;

					// [PROGRESS_TOTAL N] → トータルをセット
					if (line.rfind("[PROGRESS_TOTAL ", 0) == 0)
					{
						int total = 0;
						if (sscanf_s(line.c_str(), "[PROGRESS_TOTAL %d]", &total) == 1)
						{
							std::lock_guard lock(m_progressMutex);
							m_buildProgress.progressTotal = total;
						}
					}
					// [PROGRESS N] → 現在ステップをセット
					else if (line.rfind("[PROGRESS ", 0) == 0)
					{
						int cur = 0;
						if (sscanf_s(line.c_str(), "[PROGRESS %d]", &cur) == 1)
						{
							std::lock_guard lock(m_progressMutex);
							m_buildProgress.progressCurrent = cur;
						}
						// ラベル部分だけログに流す
						auto labelStart = line.find("] ");
						if (labelStart != std::string::npos)
							pushLog(line.substr(labelStart + 2));
					}
					else
					{
						pushLog(line);
					}
				}
			}
			if (!remainder.empty()) pushLog(remainder);

			WaitForSingleObject(pi.hProcess, INFINITE);
			DWORD exitCode = 0;
			GetExitCodeProcess(pi.hProcess, &exitCode);

			bool ok = (exitCode == 0);
			pushLog(ok ? "[Build] Build succeeded." : std::format("[Build] Build failed. (exit code: {})", exitCode));

			if (ok) Console::Log("[Build] Build succeeded.");
			else    Console::LogError(std::format("[Build] Build failed. (exit code: {})", exitCode));

			CloseHandle(hReadPipe);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);

			std::lock_guard lock(m_progressMutex);
			m_buildProgress.succeeded = ok;
			m_buildProgress.finished = true;
			m_isBuilding = false;

		}).detach();
}

void BuildSettingsWindow::PackageBuildOutput()
{
	// ビルド出力のパッケージ化処理

	// 設定を保存してからビルドを実行する
	SaveBuildSettings();

	Physics::SaveSettings(); // 物理エンジンの状態を保存しておく。
	SceneManager::SaveSettings(); // シーンマネージャの状態を保存しておく。

	// ビルド前のフック（必要に応じてビルド前にやりたい処理があればここに書く）
	{
		// ビルドするシーンを保存しておく
		for (auto& entry : SceneManager::sceneEntries)
		{
			std::filesystem::path scenePath(entry.path);
			std::filesystem::path buildScenePath = scenePath.replace_extension(".bin");

			// シーンをビルド用の形式で保存する
			json cachedJson;
			if (JsonFileHandler::LoadJsonFromFile(cachedJson, entry.path))
			{
				JsonFileHandler::SaveJsonToFile(cachedJson, buildScenePath.string(), JsonIOFormat::Binary);
			}
		}
	}

	// ビルドコマンドを構築
	// 引数: build.bat <アプリケーション名> <アイコンパス> <ビルド出力先ディレクトリ>
	std::string cmd = std::format(
		"pack.bat \"{}\" \"{}\" \"{}\"",
		m_settings.appName,
		NormalizePath(m_settings.iconPath),
		NormalizePath(m_settings.outputDir)
	);

	// --- パイプ作成 ---
	HANDLE hReadPipe = nullptr;
	HANDLE hWritePipe = nullptr;

	SECURITY_ATTRIBUTES sa{};
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;   // 子プロセスにハンドルを継承させる
	sa.lpSecurityDescriptor = nullptr;

	if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
	{
		Console::LogError("[Build] Failed to create pipe.");
		return;
	}

	// 親側の読み取りハンドルは継承不要
	SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

	// --- STARTUPINFO にパイプを接続 ---
	STARTUPINFOA si{};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = hWritePipe;   // stdout → パイプ
	si.hStdError = hWritePipe;   // stderr も同じパイプへ

	PROCESS_INFORMATION pi{};

	if (!CreateProcessA(
		nullptr, cmd.data(),
		nullptr, nullptr,
		TRUE,               // ハンドル継承を有効化
		CREATE_NO_WINDOW,   // コンソール非表示
		//CREATE_NEW_CONSOLE, // 新しいコンソールで開く
		nullptr, nullptr,
		&si, &pi))
	{
		Console::LogError("[Build] Failed to start build process.");
		CloseHandle(hReadPipe);
		CloseHandle(hWritePipe);
		return;
	}

	// 子プロセスが書き込み側ハンドルを持つので、親側は閉じる
	// ※ これをしないと ReadFile がブロックしたままになる
	CloseHandle(hWritePipe);

	m_isPackaging = true;
	Console::Log("[Build] Build started.");

	// --- 読み取りスレッド ---
	std::thread([this, hReadPipe, pi]() mutable
		{
			// パイプからの出力を読み取るループ(OutputDebugStringAに出力)
			char buffer[256]{};
			DWORD bytesRead = 0;
			while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0)
			{
				OutputDebugStringA(buffer); // デバッグ出力に流す
			}

			// ビルドプロセスの終了を待機
			WaitForSingleObject(pi.hProcess, INFINITE);

			// 終了コード取得
			DWORD exitCode = 0;
			GetExitCodeProcess(pi.hProcess, &exitCode);
			if (exitCode == 0)
				Console::Log("[Build] Build succeeded.");
			else
				Console::LogError(std::format("[Build] Build failed. (exit code: {})", exitCode));

			CloseHandle(hReadPipe);

			// ハンドルリーク防止: pi.hProcess, pi.hThread を必ず閉じる
			if (pi.hProcess) {
				CloseHandle(pi.hProcess);
			}
			if (pi.hThread) {
				CloseHandle(pi.hThread);
			}

			m_isPackaging = false;

		}).detach();
}