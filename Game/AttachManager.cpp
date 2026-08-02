#include "pch.h"
#include "AttachManager.h"
#include "TestAttachment.h"
#include "PhaseManager.h"
#include "RoundManager.h"
#include "PreserveValue.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Core/ObjectManager.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/UI/Button.h"
#include "Inventory.h"
#include "Engine/Physics/Physics.h"
#include "Engine/Scenes/SceneManager.h"
#include "Engine/Rendering/Camera/CameraSystem.h"
#include "Engine/Rendering/Camera/CameraComponent.h"
#include "Engine/Physics/Collider.h"
#include "Gadget.h"
#include "Flipper.h"
#include "Engine/Audio/Audio.h"
#include "GadgetItemView.h"
#include "TutorialSystem.h"
#include "CoinAirDrop.h"


// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(AttachManager, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(AttachManager, "UserScripts", ComponentAttributes::None, {})


void AttachManager::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	selectedPrefabPath.clear(); // プレハブパスを初期化
	selectedPrefabInstance = nullptr; // プレハブインスタンスを初期化
	pendingBall = nullptr;
	isWaitingForAttachment = false;


}

void AttachManager::Update(float deltaTime)
{
	if (!GetOwner()) return;

    //PhaseManagerを取得
    PhaseManager* phaseManager = GetScene()->objectManager->Find("PhaseManager")->GetComponent<PhaseManager>();
    if (!phaseManager) return;

	// ガジェットのツールチップ表示処理
    bool isAttachingGadget = false;
    {
        GadgetItemView* gadgetItemView = GetScene()->FindComponentById<GadgetItemView>(gadgetItemViewReference);
		bool showTooltip = false;
        // 結果フェーズではアタッチ処理を行わない
        if (phaseManager->GetCurrentPhase() != PhaseManager::Result)
        {
            if (Gadget* gadgetComponent = RaycastToGadget())
            {
                if (selectedPrefabInstance && selectedPrefabInstance == gadgetComponent->GetOwner())
                {
                    // 現在配置しようとしているガジェットのピンにホバーしている場合はツールチップを表示しない
                }
                else
                {
                    // ピンにガジェットがアタッチされている場合の処理
                    if (gadgetItemView)
                    {
                        // ガジェットの情報を取得してツールチップを表示
                        GadgetItemData data = gadgetComponent->GetGadgetItemData(); // ガジェットのアイテムデータを取得
                        gadgetItemView->ShowTooltip(gadgetComponent->GetOwner()->GetParent()->GetTransform(), data);

                        showTooltip = true; // ツールチップを表示するフラグを立てる
                        isAttachingGadget = true; // ガジェットのツールチップを表示している状態
                    }
                }
            }
        }
		if (!showTooltip)
        {
            // ピンにホバーしていない場合はツールチップを非表示にする
            if (gadgetItemView)
            {
                gadgetItemView->HideTooltip();
			}
        }
	}

    // 結果フェーズではアタッチ処理を行わない
    if (phaseManager->GetCurrentPhase() == PhaseManager::Result)
    {
        return;
    }

	// 毎フレームの更新処理をここに実装します。
	if (!isWaitingForAttachment) return;

	//ショップで何も買わずにフェーズ0（プレイ）に戻ってきた場合のスキップ処理
	if(phaseManager->GetCurrentPhase() ==PhaseManager::Playing && selectedPrefabPath.empty())
	{
		Console::Log("AttachManager: Skipped. No item purchased.");

		
		//PreserveValueを取得して値をリセット
		PreserveValue* preserveValue = GetScene()->objectManager->Find("PreserveValue")->GetComponent<PreserveValue>();
		if (preserveValue) preserveValue->ResetPreservedValue();

		//待機中のボールがあればリセット
		//if (pendingBall) pendingBall->ResetBallCount();

		isWaitingForAttachment = false; // 待機状態を解除
		//pendingBall = nullptr; // 待機中のボールをクリア
		return;
	}

    TestAttachment* testAttachment = GetScene()->objectManager->Find("TestAttachment")->GetComponent<TestAttachment>();
    if (!testAttachment) return;

    if (selectedPrefabPath.empty())
    {
        return;
	}

    if (InputSystem::GetKeyTrigger(VK_RBUTTON))
    {
        Console::Log("AttachManager: Attachment cancelled by user.");
        ClearSelectedPrefab(); // 選択されたプレハブをクリアする関数を呼び出す
        if (onAttachmentCancel)
        {
            onAttachmentCancel(); // キャンセルのコールバックを呼び出す
			onAttachmentCancel = nullptr; // コールバックをクリア
        }
    }

    if (phaseManager->GetCurrentPhase() == PhaseManager::Placement)
    {
        Pin* currentHoveredPin = RaycastToPin();

        // ゴールデンピンはホバー対象にしない
        if (currentHoveredPin && currentHoveredPin->IsGoldenPin())
        {
            currentHoveredPin = nullptr;
        }

        // ホバー対象が変わった時だけスケールを更新
        if (currentHoveredPin != hoveredPin)
        {
            // 前のピンを元に戻す
            if (hoveredPin)
            {
                hoveredPin->GetOwner()->GetTransform()->SetScale(hoveredPinOriginalScale);
				hoveredPinScaleFactor = 1.0f; // スケールファクターをリセット

				// プレビューのオブジェクトの親子関係を解除して、シーンのルートに移動させる
                if (selectedPrefabInstance)
                {
					if (Gadget* gadgetComponent = selectedPrefabInstance->GetComponent<Gadget>())
					{
						gadgetComponent->OnPreviewExit(); // プレビュー表示を終了するためのイベントを呼び出す
					}
					selectedPrefabInstance->SetActive(false); // プレビューオブジェクトを非表示にする
				}
            }
            // 新しいピンを拡大
            if (currentHoveredPin)
            {
                hoveredPinOriginalScale = currentHoveredPin->GetOwner()->GetTransform()->GetScale();
				hoveredPinScaleFactor = 1.0f; // 新しいピンのスケールファクターをリセット
                //easingHandler.Clear();
                //easingHandler.AddEasing(EaseType::OutQuart, 1.0f, 1.5f, 0.3f);

				// プレビューのオブジェクトを新しいピンの子にして、位置を合わせる
                if (selectedPrefabInstance)
                {
                    selectedPrefabInstance->SetParent(currentHoveredPin->GetOwner());
					selectedPrefabInstance->SetActive(true); // プレビューオブジェクトを表示する
					selectedPrefabInstance->GetTransform()->SetPosition(spawnPosition);
					selectedPrefabInstance->GetTransform()->SetRotation(spawnRotation);
					selectedPrefabInstance->GetTransform()->SetWorldScale(spawnScale);
                    selectedPrefabInstance->GetTransform()->UpdateTransform();

                    if (Gadget* gadgetComponent = selectedPrefabInstance->GetComponent<Gadget>())
                    {
                        gadgetComponent->OnPreviewEnter(); // プレビュー表示を開始するためのイベントを呼び出す
					}
                }
                Audio::PlayOneShot(L"./Assets/Sounds/SE/beforeGadgetPlace.wav", 0.5f);
            }

            hoveredPin = currentHoveredPin;
        }

        // ホバー中のピンにスケールを適用
        //easingHandler.Update(hoveredPinScaleFactor, deltaTime);
        if (hoveredPin)
        {
            //hoveredPin->GetOwner()->GetTransform()->SetScale(hoveredPinOriginalScale * hoveredPinScaleFactor);
        }

		// 左クリックでアタッチ処理
        bool mouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        if (!mouseDown) return;
        if (!hoveredPin) return;

        // アタッチ前にスケールを元に戻す
        //hoveredPin->GetOwner()->GetTransform()->SetScale(hoveredPinOriginalScale);
        Pin* hitPin = hoveredPin;
        hoveredPin = nullptr;

		// ピンにオブジェクトをアタッチ
        //bool attached = testAttachment->AttachToPin(selectedPrefabPath, hitPin);
        if (!selectedPrefabInstance)
        {
            Console::Log("AttachManager: No prefab instance to attach.");
            return;
		}
		selectedPrefabInstance->SetParent(nullptr); // アタッチ前に親子関係を解除して、シーンのルートに移動させる      
        selectedPrefabInstance->GetTransform()->SetRotation(spawnRotation);
		selectedPrefabInstance->GetTransform()->SetScale(spawnScale); // プレハブの元のスケールを適用
        bool attached = testAttachment->AttachToPin(selectedPrefabInstance, hitPin);
        if (!attached) return;

        for (auto* pin : GetScene()->FindComponents<GoldenPin>())
        {
            if (pin && pin->IsGoldenPin())
            {
                pin->GetOwner()->GetTransform()->SetPosition(Vector3(0, 0, 0));
                Console::Log("Attached object is a Golden Pin. Position adjusted.");
            }
        }

        Console::Log("AttachManager: Item attached successfully.");

        selectedPrefabPath.clear();
		selectedPrefabInstance = nullptr; // 生成したプレハブのインスタンスをクリア
		if (onAttachmentComplete)
        {
            onAttachmentComplete(); // アタッチ完了のコールバックを呼び出す
			onAttachmentComplete = nullptr; // コールバックをクリア
        }
        Console::Log("Attachment Success. Path cleared.");

        Audio::PlayOneShot(L"./Assets/Sounds/SE/placeGadget.wav", 0.5f);

        // チュートリアルの特定のステップであれば、次のステップに進める
        if (TutorialSystem* tutorialSystem = TutorialSystem::GetInstance())
        {
			// TODO: 処理が変更される可能性あり
			// 配置の基本操作を説明するステップと配置を完了することを待つステップであれば、次のステップに進める
            if (tutorialSystem->GetCurrentTutorialStep() == TutorialSystem::TutorialStep::Placement)
            {
                tutorialSystem->AdvanceTutorialStep(); // チュートリアルを進める
            }
            if (tutorialSystem->GetCurrentTutorialStep() == TutorialSystem::TutorialStep::WaitForPlacementEnd)
            {
                tutorialSystem->AdvanceTutorialStep(); // チュートリアルを進める
			}
        }

    }
}

void AttachManager::SetSelectedPrefabPath(const GadgetItemData& data, std::function<void()> onCompleteFunc, std::function<void()> onCancelFunc)
{
	ClearSelectedPrefab(); // 既存の選択をクリアしてから新しいプレハブを設定する
	selectedPrefabPath = data.prefabPath;
	onAttachmentComplete = onCompleteFunc;
	onAttachmentCancel = onCancelFunc;
    if (selectedPrefabPath.empty())
    {
        Console::Log("Selected prefab path is empty. No item to attach.");
	}
	else if (!selectedPrefabInstance)
    {
        Console::Log("Path RECEIVED by AttachManager: " + data.prefabPath);

		// さきに生成しておく
		selectedPrefabInstance = Instantiate(selectedPrefabPath, Vector3::Zero);
        if (selectedPrefabInstance)
        {
            Console::Log("Prefab instantiated successfully: " + selectedPrefabPath);

            if (Flipper* flipper = selectedPrefabInstance->GetComponentInChildren<Flipper>())
            {
                Vector3 euler = selectedPrefabInstance->GetTransform()->GetEulerAngles();
                euler.y += flipper->isRightFlipper ? 45.0f : -45.0f;
                selectedPrefabInstance->GetTransform()->SetRotation(euler);
                spawnRotation = selectedPrefabInstance->GetTransform()->GetRotation();
            }
            //コインタワーも傾ける
            else if (CoinAirDrop* coinAirDrop = selectedPrefabInstance->GetComponentInChildren<CoinAirDrop>())
            {
                Vector3 euler = selectedPrefabInstance->GetTransform()->GetEulerAngles();
                euler.y += 45.0f; // コインタワーを45度回転させる
                selectedPrefabInstance->GetTransform()->SetRotation(euler);
                spawnRotation = selectedPrefabInstance->GetTransform()->GetRotation();
			}
            else
            {
                spawnRotation = selectedPrefabInstance->GetTransform()->GetRotation();
            }

			spawnRotation = selectedPrefabInstance->GetTransform()->GetRotation(); // プレハブの元の回転を保持
			spawnScale = selectedPrefabInstance->GetTransform()->GetScale(); // プレハブの元のスケールを保持

            if (Gadget* gadgetComponent = selectedPrefabInstance->GetComponent<Gadget>())
            {
                gadgetComponent->SetDurability(data.durability);
				gadgetComponent->SetGadgetItemData(data);
			}

			// 生成したオブジェクトは非表示にしておく
			selectedPrefabInstance->SetActive(false);
        }
        else
        {
            LOG_ERROR("Failed to instantiate prefab: " + selectedPrefabPath);
		}
    }
    else
    {
		LOG_WARNING("Prefab instance already exists for path: " + selectedPrefabPath);
    }
}

void AttachManager::StartWaitingForAttachment()
{
	isWaitingForAttachment = true;
	Console::Log("AttachManager: Started waiting for attachment.");
}

Pin* AttachManager::RaycastToPin() const
{
    Vector2 rayStartScreen = InputSystem::GetMousePosition();
    Vector3 rayStart, rayDir;
	Scene* currentScene = SceneManager::GetCurrentScene();
	if (!currentScene) return nullptr;
	CameraComponent* mainCamera = currentScene->GetCameraSystem()->GetMainCamera();
	if (!mainCamera) return nullptr;
    mainCamera->ScreenPointToRay(rayStartScreen, rayStart, rayDir);
	float rayLength = 5.0f; // レイの長さを適切に設定

    RaycastHit hitInfo;
	static constexpr LayerMask layerMask = ToMask(5); // レイヤー5（例: "Pin" レイヤー）にのみ当たるようにする
    if (Physics::Raycast(rayStart, rayDir, rayLength, hitInfo, layerMask))
    {
        if (!hitInfo.collider) return nullptr;
        GameObject* hitObject = hitInfo.collider->GetOwner();
        if (!hitObject) return nullptr;
		Pin* pinComponent = hitObject->GetComponent<Pin>();
		if (pinComponent && pinComponent->IsEnabled()) // ピンコンポーネントが存在し、有効な場合にのみ返す
        {
            return pinComponent;
		}
    }
    return nullptr;
}

Gadget* AttachManager::RaycastToGadget() const
{
    Vector2 rayStartScreen = InputSystem::GetMousePosition();
    Vector3 rayStart, rayDir;
    Scene* currentScene = SceneManager::GetCurrentScene();
    if (!currentScene) return nullptr;
    CameraComponent* mainCamera = currentScene->GetCameraSystem()->GetMainCamera();
    if (!mainCamera) return nullptr;
    mainCamera->ScreenPointToRay(rayStartScreen, rayStart, rayDir);
    float rayLength = 5.0f; // レイの長さを適切に設定
    RaycastHit hitInfo;
	static constexpr LayerMask layerMask = ToMask(5) | ToMask(6); // レイヤー5（Pin レイヤー）とレイヤー6（お邪魔ガジェット用Pin）に当たるようにする
    if (Physics::Raycast(rayStart, rayDir, rayLength, hitInfo, layerMask))
    {
        if (!hitInfo.collider) return nullptr;
        GameObject* hitObject = hitInfo.collider->GetOwner();
        if (!hitObject) return nullptr;
        return hitObject->GetComponentInChildren<Gadget>();
    }
    return nullptr;
}

void AttachManager::ClearSelectedPrefab()
{
    selectedPrefabPath.clear();
    if (selectedPrefabInstance)
    {
        Destroy(selectedPrefabInstance);
        selectedPrefabInstance = nullptr;
    }
    Console::Log("Selected prefab cleared.");
}