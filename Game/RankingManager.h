#pragma once
#include "Engine/Core/Component.h"
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <WinInet.h>

struct RankingEntry {
	std::wstring name;
	int score;
};

class RankingManager : public Component {
	C_REFLECT(RankingManager)
public:
	void Start() override;
	void UpdateRanking(const std::wstring& playerName, int score);
	bool IsLoading() const { return isLoading; }

	void Finalize() override {
		isCancelled = true; // キャンセルフラグを立てる
		if (m_workerThread.joinable()) {
			m_workerThread.join(); // スレッドの終了を待機
		}
	}

	const std::vector<RankingEntry>& GetRankings() { 
		std::lock_guard<std::mutex> lock(dataMutex); // データアクセスの保護
		return rankings;
	}

private:
	std::vector<RankingEntry> rankings;
	std::atomic<bool> isLoading = false;
	std::mutex dataMutex;

	const std::wstring webAppUrl = L"script.google.com";
	const std::wstring normalScriptPath = L"/macros/s/AKfycbyZK7Q_asxPbRMIlGsL10GGj5ebni9DI5y1sBXHNlyNvKhAQHARvdE3bmb3WPbALl_-/exec";
	const std::wstring endlessScriptPath = L"/macros/s/AKfycbw_N3M41DXfrhOR2HRLrzkWcBc2LQpGd6Qs1C2XnzLI4AEmC6VU7Uuo0gNmtgsOJgro6g/exec";

	// --- 文字列変換ユーティリティ (Windows API 使用) ---
	static std::wstring Utf8ToWstring(const std::string& utf8Str);
	static std::string WstringToUtf8(const std::wstring& wstr);

	// ランキング取得の非同期処理を行う関数
	void AsyncRequest(std::wstring playerName, int score);
	

	// ロード開始前の処理（ローディングインジケーターの表示など）を行う関数
	void OnLoadingStart();

	// ロード完了後の処理（UI更新など）を行う関数
	void OnLoaded();

	// ロード失敗時の処理を行う関数
	void OnLoadingFailed();
public:

	// ランキング取得の中断を行う関数
	void CancelRequest();

	// Endlessモードかどうかを設定する関数
	void SetEndlessMode(bool endless) {
		isEndlessMode = endless;
	}

	bool IsEndlessMode() const {
		return isEndlessMode;
	}

private:
	// 取得したデータを保持する構造体（必要に応じて）
	struct RankData {
		std::wstring name;
		int rank;
		int score;
		bool isPlayer = false; // プレイヤー自身のスコアかどうかを示すフラグ

	};
	std::vector<RankData> currentRanks;
	std::atomic<bool> isCancelled = false; // キャンセルフラグ
	std::thread m_workerThread; // 非同期処理用のスレッド
public:
	const std::vector<RankData>& GetCurrentRanks() const {
		return currentRanks;
	}
	void ClearCurrentRanks() {
		currentRanks.clear();
	}

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
		ObjectId loadingIndicatorReference; // ローディングインジケーターオブジェクトへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId loadingFailedObjectReference; // ロード失敗時に表示するオブジェクトへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::NonSerialized)
		bool isEndlessMode = false; // Endlessモードかどうかのフラグ


	static bool IsNetworkAvailable()
	{
		DWORD flags;
		return InternetGetConnectedState(&flags, 0);
	}


};