#include "pch.h"
#include "ResultManager.h"
#include "Engine/Scenes/Scene.h"
#include "PreserveValue.h"
#include "ComboText.h"
#include "RoundManager.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Audio/AudioSource.h"
#include "PlayerNameManager.h"
#include "Engine/Core/Color.h"
#include "RankingManager.h"


REGISTER_COMPONENT(ResultManager, "UserScripts")

void ResultManager::Start() {
	// 初期状態は完全に透明・非表示
	ApplyAlpha(0.0f);
	GetOwner()->SetActive(false);
	// パネルを少し小さくしておく（登場時のズーム用）
	GetOwner()->GetTransform()->SetScale({ 0.8f, 0.8f, 1.0f });



}

int GetDisplayWidth(const std::wstring& ws) {
	int width = 0;
	for (wchar_t wc : ws) {
		// Unicodeの範囲に基づく簡易的な判定
		// 0x007E以下（ASCII）または 0xFF61〜0xFF9F（半角カナ）は幅1、それ以外は幅2
		if (wc <= 0x007E || (wc >= 0xFF61 && wc <= 0xFF9F)) {
			width += 1;
		}
		else {
			width += 2;
		}
	}
	return width;
}
void ResultManager::ShowResult() {
	isShowing = true;
	timer = 0.0f;
	isRankingApplied = false; // ランキング表示の二重更新防止フラグをリセット

	// データの読み込み
	if (auto* pvObj = GetScene()->objectManager->Find("PreserveValue")) {
		if (auto* pv = pvObj->GetComponent<PreserveValue>()) {
			targetMoney = pv->GetTotalValue();
			displayMoney = 0; // カウントアップ開始前は0にしておく
		}
	}
	if (auto* pvObj = GetScene()->objectManager->Find("RoundManager")) {
		if (auto* pv = pvObj->GetComponent<RoundManager>()) {
			targetRound = pv->GetCurrentRound();
			maxRound = pv->GetMaxRounds();
			displayRound = 0; // カウントアップ開始前は0にしておく
		}
	}

	if (auto* pvObj = GetScene()->objectManager->Find("ComboText")) {
		if (auto* pv = pvObj->GetComponent<ComboText>()) {

			targetCombo -= pv->GetMaxComboCount();
			displayCombo = 0;
		}
	}
	ObjectId rankingRefs[] = {
		rankingfirstTextRef, rankingsecondTextRef, rankingthirdTextRef,
		rankingfourthTextRef, rankingfifthTextRef, rankingsixthTextRef,
		rankingseventhTextRef, rankingeighthTextRef, rankingninthTextRef, rankingtenthTextRef
	};

	for (ObjectId ref : rankingRefs) {
		UpdateText(ref, L"");
	}


	GetOwner()->SetActive(true);

	//BGM切り替え
	// メインBGMを停止する
	if (GameObject* bgmObj = GetScene()->objectManager->Find("BGM")) {
		if (AudioSource* mainBgm = bgmObj->GetComponent<AudioSource>()) {
			mainBgm->Stop();
		}
	}
	// エディタ上でアタッチしたリザルト用BGMを再生する
	if (AudioSource* resultBgm = GetScene()->FindComponentById<AudioSource>(resultBgmRef)) {
		resultBgm->Play();
	}

	// 背景の暗転開始音などあれば
   // Audio::PlayOneShot(L"Assets/Sounds/SE/Result_Start.wav");
}

void ResultManager::HideResult() {
	isShowing = false;
	GetOwner()->SetActive(false);
	// Audio::PlayOneShot(L"Assets/Sounds/SE/Result_End.wav");

	//リザルトBGMを止める
	if (AudioSource* resultBgm = GetScene()->FindComponentById<AudioSource>(resultBgmRef)) {
		resultBgm->Stop();
	}
}

void ResultManager::Update(float deltaTime) {


	float prevTimer = timer;
	timer += deltaTime;

	// --- 1. 背景とパネルのバウンス登場 (0.0s - 0.6s) ---
	if (timer <= 0.6f) {
		float t = timer / 0.6f;
		ApplyAlpha(t);

		// バウンス計算：少し大きく膨らんでから 1.0 に戻る
		float bounce = std::sin(t * 3.1415f * 0.7f) * 0.2f;
		float scale = 0.8f + bounce;
		GetOwner()->GetTransform()->SetScale({ scale, scale, 1.0f });
	}
	if (prevTimer < 0.6f && timer >= 0.6f) {
		if (auto* rmObj = GetScene()->objectManager->Find("RankingManager")) {
			if (auto* rm = rmObj->GetComponent<RankingManager>()) {
				if (auto* pnmObj = GetScene()->objectManager->Find("PlayerNameManager")) {
					if (auto* pnm = pnmObj->GetComponent<PlayerNameManager>()) {
						std::wstring playerName = pnm->GetName();
						rm->UpdateRanking(playerName, targetMoney); // 実際は入力UIから取得
					}
				}
			}
		}
	}

	// --- 2. リザルトタイトルのスタンプ演出 (0.7s) ---
	if (prevTimer < 0.7f && timer >= 0.7f) {
		ShowElement(resultTextRef, L"- RESULT -");
		// Audio::PlayOneShot(L"Assets/Sounds/SE/Title_Stamp.wav");
	}

	// --- 3. ラウンド数 (0.9s) ---
	if (prevTimer < 0.9f && timer >= 0.9f) {
		// 少し左から右へスライドさせるなどの演出を入れるとGood
		ShowElement(roundTextRef, L"ラウンド: " + std::to_wstring(targetRound));
		// Audio::PlayOneShot(L"Assets/Sounds/SE/Popup_Item.wav");
	}

	// --- 4. お金のカウントアップ (1.3s - 2.3s) ---
	if (timer > 1.3f && timer < 2.3f) {
		float p = (timer - 1.3f) / 1.0f;
		displayMoney = static_cast<int>(targetMoney * p);
		UpdateText(moneyTextRef, L"金額: " + std::to_wstring(displayMoney) + L" $");

		// カウント音を細かく鳴らす（static変数でタイミング制御）
		static float tickTimer = 0.0f;
		tickTimer += deltaTime;
		if (tickTimer > 0.08f) {
			//Audio::PlayOneShot(L"Assets/Sounds/SE/Count_Tick.wav");
			tickTimer = 0.0f;
		}
	}
	// 確定の瞬間
	if (prevTimer < 2.3f && timer >= 2.3f) {
		UpdateText(moneyTextRef, L"金額: " + std::to_wstring(targetMoney) + L" $");
		// Audio::PlayOneShot(L"Assets/Sounds/SE/Count_Finish.wav");
	}

	// --- 5. ランク評価ロジック (2.6s) ---
	if (prevTimer < 2.6f && timer >= 2.6f) {

		/*		// 各項目のポイント加算用
				int totalRankPoint = 0;

				// ① ラウンド評価 (例: 最大10ラウンド想定)
				int roundPoint = 0;
				if (targetRound >= maxRound) roundPoint = 4;      // S相当
				else if (targetRound >= maxRound * 0.7f) roundPoint = 3; // A相当
				else if (targetRound >= maxRound * 0.4f) roundPoint = 2; // B相当
				else roundPoint = 1;                             // C相当

				// ② 所持金評価
				int moneyPoint = 0;
				if (targetMoney >= 5000) moneyPoint = 4;
				else if (targetMoney >= 3000) moneyPoint = 3;
				else if (targetMoney >= 1000) moneyPoint = 2;
				else moneyPoint = 1;

				// ③ コンボ評価
				int comboPoint = 0;
				if (targetCombo >= 50) comboPoint = 4;
				else if (targetCombo >= 30) comboPoint = 3;
				else if (targetCombo >= 10) comboPoint = 2;
				else comboPoint = 1;

				// 合計ポイント (3項目 × 最大4点 = 最大12点)
				totalRankPoint = roundPoint + moneyPoint + comboPoint;

				// 最終ランクの決定
				std::wstring finalRank;
				Color rankColor;

				if (totalRankPoint >= 11) { // ほぼすべてS
					finalRank = L"S";
					rankColor = Color(1.0f, 0.84f, 0.0f, 1.0f);
				}
				else if (totalRankPoint >= 8) { // 平均A以上
					finalRank = L"A";
					rankColor = Color(1.0f, 0.3f, 0.3f, 1.0f);
				}
				else if (totalRankPoint >= 5) { // 平均B以上
					finalRank = L"B";
					rankColor = Color(0.3f, 1.0f, 0.3f, 1.0f);
				}
				else {
					finalRank = L"C";
					rankColor = Color(0.6f, 0.6f, 0.6f, 1.0f);
				}*/

				// 表示処理
		std::wstring resultMsg = L"コンボ数: " + std::to_wstring(-targetCombo) /*+ L"  [RANK: " + finalRank + L"]"*/;
		ShowElement(comboTextRef, resultMsg);

		if (Text* t = GetScene()->FindComponentById<Text>(comboTextRef)) {
			//t->SetColor(rankColor);
		}

		// 演出：ランク決定時に少し大きくする
		GetOwner()->GetTransform()->SetScale({ 1.1f, 1.1f, 1.0f });
	}
	// Audio::PlayOneShot(L"Assets/Sounds/SE/Rank_S.wav");

// 演出が終わったタイミングで一度だけ呼ぶ
/*	if (prevTimer < 3.0f && timer >= 3.0f) {
		if (auto* rmObj = GetScene()->objectManager->Find("RankingManager")) {
			if (auto* rm = rmObj->GetComponent<RankingManager>()) {
				if (auto* pnmObj = GetScene()->objectManager->Find("PlayerNameManager")) {
					if (auto* pnm = pnmObj->GetComponent<PlayerNameManager>()) {
						std::wstring playerName = pnm->GetName();
						rm->UpdateRanking(playerName, targetMoney); // 実際は入力UIから取得
					}
				}
			}
		}
	}*/

	// --- Update関数内、3.0s以降の処理 ---

	if (timer > 3.0f) {
		if (auto* rmObj = GetScene()->objectManager->Find("RankingManager")) {
			auto* rm = rmObj->GetComponent<RankingManager>();

			// 通信（読み込み）が完了した瞬間に一度だけ表示を更新する
			if (rm && !rm->IsLoading()) {
				auto& list = rm->GetCurrentRanks();
				int maxRankDigits = 0 /*= std::to_wstring(list).size()*/;
				size_t maxNameLen = 0;
				int maxScoreDigits = 0;

				for (const auto& r : list) {
					maxRankDigits = (max(maxRankDigits, (int)std::to_wstring(r.rank).size()));
					maxNameLen = max((int)maxNameLen, GetDisplayWidth(r.name));
					maxScoreDigits = max(maxScoreDigits,
						(int)std::to_wstring(r.score).size());
				}

				// ---- ヘッダーで定義した各テキストRefを配列にまとめる ----
				ObjectId rankingRefs[] = {
					rankingfirstTextRef, rankingsecondTextRef, rankingthirdTextRef,
					rankingfourthTextRef, rankingfifthTextRef, rankingsixthTextRef,
					rankingseventhTextRef, rankingeighthTextRef, rankingninthTextRef, rankingtenthTextRef
				};

				// ---- 10件分の表示 ----
				for (int i = 0; i < 10; ++i) {
					if (i < list.size()) {

						const auto& r = list[i];

						// ---- 揃えた wstring を作成 ----
						std::wstring rankStr;

						// 順位（右詰め）
						std::wstring rank = std::to_wstring(r.rank);
						rankStr += std::wstring(maxRankDigits - rank.size(), L' ') + rank + L"位: ";

						// 名前（左詰め）
						std::wstring name = r.name;
						if (GetDisplayWidth(name) < maxNameLen)
							name += std::wstring(maxNameLen - GetDisplayWidth(name), L' ');
						rankStr += name + L" - ";

						// スコア（右詰め）
						std::wstring score = std::to_wstring(r.score);
						rankStr += std::wstring(maxScoreDigits - score.size(), L' ') + score + L" コイン";

						// ---- UI に反映 ----
						static const std::array<Color, 4> rankColors = {
							Color(1,1,1,1),
							Color(1.0f, 0.84f, 0.0f, 1.0f),   // 1位
							Color(0.75f, 0.75f, 0.75f, 1.0f), // 2位
							Color(0.8f, 0.5f, 0.2f, 1.0f)     // 3位
						};

						Color textColor =
							r.isPlayer
							? Color(0.4f, 0.84f, 1.0f, 1.0f)
							: (r.rank <= 3 ? rankColors[r.rank] : Color(1, 1, 1, 1));
						UpdateText(rankingRefs[i], rankStr, textColor);

					}
					else {
						UpdateText(rankingRefs[i], L"");
					}
				}
			}
		}
	}
}

// ヘルパー関数：テキストを更新
void ResultManager::UpdateText(ObjectId ref, const std::wstring& str, Color color) {
	if (Text* t = GetScene()->FindComponentById<Text>(ref)) {
		t->SetText(str);
		color.a = t->color.a;
		t->SetColor(color);
	}
}

// ヘルパー関数：要素をパッと出す
void ResultManager::ShowElement(ObjectId ref, const std::wstring& str) {
	if (Text* t = GetScene()->FindComponentById<Text>(ref)) {
		t->GetOwner()->SetActive(true);
		t->SetText(str);
		// 少し揺らす演出などを追加してもOK
	}
}

void ResultManager::ApplyAlpha(float alpha) {
	if (Image* bg = GetScene()->FindComponentById<Image>(backGroundRef)) {
		bg->color.a = alpha * 0.7f;
	}
	if (Image* container = GetScene()->FindComponentById<Image>(resultContainerRef)) {
		container->color.a = alpha;
	}
	if (Text* t = GetScene()->FindComponentById<Text>(resultTextRef)) {
		t->color.a = alpha;
	}
	if (Text* t = GetScene()->FindComponentById<Text>(roundTextRef)) {
		t->color.a = alpha;
	}
	if (Text* t = GetScene()->FindComponentById<Text>(moneyTextRef)) {
		t->color.a = alpha;
	}
	if (Text* t = GetScene()->FindComponentById<Text>(comboTextRef)) {
		t->color.a = alpha;
	}
}
