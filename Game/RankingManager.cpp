#include "pch.h"
#include "RankingManager.h"
#include "json.hpp"
#include <winhttp.h>
#include <Windows.h>
#include "Engine/Scenes/Scene.h"

#pragma comment(lib, "winhttp.lib")

using json = nlohmann::json;

REGISTER_COMPONENT(RankingManager, "UserScripts")

// --- 文字列変換の実装 ---
std::wstring RankingManager::Utf8ToWstring(const std::string& utf8Str) {
	if (utf8Str.empty()) return L"";
	int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), NULL, 0);
	std::wstring wstrTo(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), &wstrTo[0], sizeNeeded);
	return wstrTo;
}

std::string RankingManager::WstringToUtf8(const std::wstring& wstr) {
	if (wstr.empty()) return "";
	int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);
	return strTo;
}

void RankingManager::Start() {}

void RankingManager::UpdateRanking(const std::wstring& playerName, int score) {
	if (isLoading) return;
	// ロード中に出すUIを表示する処理
	OnLoadingStart();

	isCancelled = false; // キャンセルフラグをリセット

	if (m_workerThread.joinable()) {
		m_workerThread.join(); // 前のスレッドがまだ動いている場合は待機
	}

	Console::Log("UpdateRanking");
	m_workerThread = std::thread(&RankingManager::AsyncRequest, this, playerName, score);
}

void RankingManager::AsyncRequest(std::wstring playerName, int score) {
	std::lock_guard<std::mutex> lock(dataMutex); // データアクセスの保護
	HINTERNET hSession = WinHttpOpen(L"CurryEngine/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	Console::Log("RankASyncRrequest");
	if (!hSession) {
		OnLoadingFailed(); // 変更: 失敗したためOnLoadingFailedを呼ぶ
		ClearCurrentRanks();
		Console::Log(" WinHttp Session creation failed");
		return;
	}
	Console::Log(" WinHttp Session creation sucsess");
	HINTERNET hConnect = WinHttpConnect(hSession, webAppUrl.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
	if (!hConnect) {
		WinHttpCloseHandle(hSession);
		Console::Log(" WinHttp Connect failed");
		OnLoadingFailed(); // 変更: 失敗したためOnLoadingFailedを呼ぶ
		ClearCurrentRanks();
		return;
	}
	Console::Log(" WinHttp Connect Sucsess");
	// URL構築時に名前をUTF-8に変換
	// ※本来はここでURLエンコードも必要ですが、GAS側が対応していればこのままでも届く場合があります
	// 1. クエリを構築
	playerName.erase(
		std::remove(playerName.begin(), playerName.end(), L'\0'),
		playerName.end()
	);

	std::wstring query = (isEndlessMode ? endlessScriptPath : normalScriptPath) + L"?action=send&name=" + (playerName)+L"&score=" + std::to_wstring(score);

	// 2. フルURLを構築してログに出力
	std::wstring fullUrl = L"https://" + webAppUrl + query;

	// wstring を string に変換して Console::Log に渡す (CurryEngineの仕様に合わせる)
	std::string logUrl = WstringToUtf8(fullUrl);
	Console::Log("Sending Request to: " + logUrl);

	// 3. リクエスト開始
	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", query.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

	DWORD redirectOption = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
	WinHttpSetOption(hRequest, WINHTTP_OPTION_REDIRECT_POLICY, &redirectOption, sizeof(redirectOption));

	if (isCancelled) {
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return;
	}

	bool success = false; // 通信とパースが正常に完了したかどうかのフラグ

	if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
		WinHttpReceiveResponse(hRequest, NULL))
	{
		std::string responseData;
		DWORD dwSize = 0;
		do {
			// キャンセルフラグが立っている場合はループを抜ける
			if (isCancelled) {
				Console::Log("Request cancelled.");
				break;
			}

			if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
			if (dwSize == 0) break;

			std::vector<char> buffer(dwSize + 1);
			DWORD dwDownloaded = 0;
			if (WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded)) {
				buffer[dwDownloaded] = '\0';
				responseData.append(buffer.data(), dwDownloaded);
			}
		} while (dwSize > 0);

		if (!isCancelled)
		{
			Console::Log("Received Data: " + responseData);

			try {
				auto j = json::parse(responseData);
				for (auto& item : j) {
					std::string name = item["name"];
					int rank = item["rank"];
					int score = item["score"];
					bool isPlayer = item["isNew"];

					RankData rd;
					rd.name = Utf8ToWstring(name);
					rd.rank = rank;
					rd.score = score;
					rd.isPlayer = isPlayer;
					Console::Log(WstringToUtf8(rd.name));
					currentRanks.push_back(rd);
				}
				success = true; // パースまで成功したためフラグを上げる
			}
			catch (const std::exception& e) {
				Console::Log(std::string("JSON Parse Error: ") + e.what());
			}
		}
	}
	else
	{
		Console::Log(" WinHttp Request or Response failed");
	}

	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

	if (!isCancelled)
	{
		if (success) {
			OnLoaded();
		}
		else {
			OnLoadingFailed();
			ClearCurrentRanks();
		}
	}
}

void RankingManager::OnLoadingStart() {
	ClearCurrentRanks();
	// ロード開始前にUIを表示する処理
	if (auto* loadingIndicator = GetScene()->FindGameObjectById(loadingIndicatorReference)) {
		loadingIndicator->SetActive(true);
	}
	isLoading = true;
}

void RankingManager::OnLoaded() {
	// ロード完了後にUIを非表示にする処理
	if (GetScene() != nullptr)
	{
		if (auto* loadingIndicator = GetScene()->FindGameObjectById(loadingIndicatorReference)) {
			loadingIndicator->SetActive(false);
		}
	}
	isLoading = false;
}

void RankingManager::OnLoadingFailed() {
	// ロード失敗時にUIを表示する処理
	if (GetScene() != nullptr)
	{
		if (auto* loadingIndicator = GetScene()->FindGameObjectById(loadingIndicatorReference)) {
			loadingIndicator->SetActive(false);
		}
		if (auto* loadingFailedObject = GetScene()->FindGameObjectById(loadingFailedObjectReference)) {
			loadingFailedObject->SetActive(true);
		}
	}
	isLoading = false;
}

void RankingManager::CancelRequest() {
	if (!isLoading) return; // ロード中でなければキャンセル不要
	
	isCancelled = true; // キャンセルフラグを立てる

	if (m_workerThread.joinable()) {
		m_workerThread.join(); // ワーカースレッドが終了するのを待つ
	}
	
	OnLoaded(); // ロード完了後の処理を呼び出す
}