#pragma once
#include "Engine/Core/Component.h"
#include "Engine/UI/Text.h"
#include "Engine/UI/Image.h"

class ResultManager : public Component {
	C_REFLECT(ResultManager)
public:
	void Start() override;
	void Update(float deltaTime) override;

	// リザルト表示を開始するトリガー（外部から呼ぶ想定）
	void ShowResult();

	void HideResult(); // リザルトを非表示にする関数

private:
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Image"))
		ObjectId backGroundRef;     // 背景の暗転用

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Image"))
		ObjectId resultContainerRef; // リザルトパネル全体

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId resultTextRef;      // 到達ラウンド

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId roundTextRef;      // 到達ラウンド

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId moneyTextRef;      // 最終所持金

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId comboTextRef;      // 最大コンボなど


	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId rankingTextRef;      // ランキング表示用

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId rankingfirstTextRef;      // ランキング表示用
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId rankingsecondTextRef;      // ランキング表示用
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId rankingthirdTextRef;      // ランキング表示用
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId rankingfourthTextRef;      // ランキング表示用
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId rankingfifthTextRef;      // ランキング表示用
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId rankingsixthTextRef;      // ランキング表示用
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId rankingseventhTextRef;      // ランキング表示用
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId rankingeighthTextRef;      // ランキング表示用
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId rankingninthTextRef;      // ランキング表示用
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId rankingtenthTextRef;      // ランキング表示用

	// エディタ上でリザルトBGMのAudioSourceを持つGameObjectをアタッチするためのプロパティ
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("AudioSource"))
		ObjectId resultBgmRef;

	bool isShowing = false;
	float timer = 0.0f;

	// 演出用の内部数値
	int targetMoney = 0;
	int displayMoney = 0;

	int targetCombo = 0;
	int displayCombo = 0;


	int targetRound = 0;
	int displayRound = 0;
	int maxRound = 0;

	bool isRankingApplied = false; // ランキング表示の二重更新防止フラグ

	void ApplyAlpha(float alpha);
	// ヘルパー関数：テキストを更新
	void UpdateText(ObjectId ref, const std::wstring& str, Color color = Color(1.0f, 1.0f, 1.0f, 1.0f));

	// ヘルパー関数：要素をパッと出す
	void ShowElement(ObjectId ref, const std::wstring& str);


};