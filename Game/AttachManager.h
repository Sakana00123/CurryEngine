#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Ball.h"
#include "Pin.h"
#include "Engine/Easing/EasingHandler.h"
#include "GadgetItemData.h"
class ItemInventory;
class Gadget;

class AttachManager : public Component
{
	C_REFLECT(AttachManager)
public:
	AttachManager() = default;
	~AttachManager() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	// プレハブのファイルパスを設定する関数
	void SetSelectedPrefabPath(const GadgetItemData& data, std::function<void()> onCompleteFunc, std::function<void()> onCancelFunc = nullptr);

	void StartWaitingForAttachment();

	//レイキャスト専用の関数
	Pin* RaycastToPin() const;

	Gadget* RaycastToGadget() const;

	bool IsSelectingPrefab() const { return !selectedPrefabPath.empty(); } // プレハブが選択されているかどうかを判断する関数

	void ClearSelectedPrefab(); // 選択されたプレハブをクリアする関数

private:


	C_PROPERTY(CurryEngine::PropertyAttributes::NonSerialized)
	std::string selectedPrefabPath; // 配置するプレハブのファイルパス

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GadgetItemView"))
	ObjectId gadgetItemViewReference; // 例: GadgetItemView コンポーネントへの参照

	GameObject* selectedPrefabInstance = nullptr; // 配置するプレハブのインスタンスへのポインタ
	Vector3 spawnPosition = Vector3(0, -0.25f, 0); // オブジェクトの中心
	Quaternion spawnRotation; // オブジェクトの回転
	Vector3 spawnScale; // オブジェクトのスケール

	Ball* pendingBall = nullptr; // アタッチを待っているボールへのポインタ
	bool isWaitingForAttachment = false; // ボールがアタッチされるのを待っているかどうかのフラグ

	Pin* hoveredPin = nullptr; // 現在当たっているピン
	Vector3 hoveredPinOriginalScale = Vector3::Zero; // 当たっているピンの元のスケール	

	EasingHandler easingHandler; // ピンのスケールを変化させるためのイージングハンドラー
	float hoveredPinScaleFactor = 1.0f;

	std::function<void()> onAttachmentComplete; // アタッチ完了時のコールバック関数
	std::function<void()> onAttachmentCancel; // アタッチキャンセル時のコールバック関数
};