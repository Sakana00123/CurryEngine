#include "pch.h"
#include "InputModule.h"
#include "EventSystem.h"
#include "EventHandlers.h"

REGISTER_COMPONENT(InputModule, "EventSystem")

InputModule::InputModule() : BaseInputModule()
{
    
}

void InputModule::OnEnable()
{
	eventSystem->RegisterInputModule(gameObject->GetComponentShared<InputModule>());
    pointerEventData = std::make_shared<PointerEventData>(eventSystem);
    axisEventData = std::make_shared<AxisEventData>(eventSystem);
}

void InputModule::OnDisable()
{
	eventSystem->UnregisterInputModule();
	pointerEventData.reset();
	axisEventData.reset();
}

std::shared_ptr<PointerEventData> InputModule::GetEventData()
{
    return pointerEventData;
}


void InputModule::Process(float deltaTime)
{
    if (pointerEventData == nullptr || axisEventData == nullptr) {
        return; // データが初期化されていない場合は処理しない
	}

    //入力情報を更新
    pointerEventData->lastPosition.x = static_cast<float>(InputSystem::GetOldMousePositionX());
    pointerEventData->lastPosition.y = static_cast<float>(InputSystem::GetOldMousePositionY());
    InputSystem::GetMousePosition(&pointerEventData->position.x);

    //レイキャスト処理
    RaycastResult result = pointerEventData->pointerCurrentRaycast = eventSystem->RaycastAll();

    //ホバーしているオブジェクトが変わってたら
    if (pointerEventData->GetPointerEnter() != result.GetHitGameObject()) {
        //TODO: OnPointerExitを発行
        ExecuteEvent<IPointerExitHandler>(pointerEventData->GetPointerEnter(), pointerEventData, &IPointerExitHandler::Execute);

        pointerEventData->SetPointerEnter(result.GetHitGameObject());

        //TODO: OnPointerEnterを発行
        ExecuteEvent<IPointerEnterHandler>(pointerEventData->GetPointerEnter(), pointerEventData, &IPointerEnterHandler::Execute);
    }
    //マウスの状態ごとの処理
    {
        if (InputSystem::GetInputState("ok", InputStateMask::Trigger, DeviceFlags::MouseOnly)) {
            pointerEventData->pointerPressRaycast = result;
            //選択オブジェクトが存在するときにイベント
            if (result.IsValid()) {
                pointerEventData->SetPointerPress(result.GetHitGameObject());
                //pointerEventData->pressPosition = result.GetHitGameObject()->GetComponent<RectTransform>()->GetWorldPosition();
                pointerEventData->pressPosition = pointerEventData->position;
                pointerEventData->eligibleForClick = true;

                ExecuteEvent<IPointerDownHandler>(pointerEventData->GetPointerPress(), pointerEventData, &IPointerDownHandler::Execute);
            }
            //選択オブジェクトが変わったときのイベント
            if (result.GetHitGameObject() != eventSystem->GetSelectedGameObject()) {
                ExecuteEvent<IDeselectHandler>(eventSystem->GetSelectedGameObject(), pointerEventData, &IDeselectHandler::Execute);
                ExecuteEvent<ISelectHandler>(result.GetHitGameObject(), pointerEventData, &ISelectHandler::Execute);
            }
            eventSystem->SetSelectedGameObject(result.GetHitGameObject());
        }
        else if (InputSystem::GetInputState("ok", InputStateMask::None, DeviceFlags::MouseOnly)) {
            //マウスの移動量更新
            pointerEventData->delta.x = pointerEventData->position.x - pointerEventData->lastPosition.x;
            pointerEventData->delta.y = pointerEventData->position.y - pointerEventData->lastPosition.y;

            //ドラッグ開始判定
            if (!pointerEventData->dragging) {
                //押されてからの移動量
                float moveX = pointerEventData->position.x - pointerEventData->pressPosition.x;
                float moveY = pointerEventData->position.y - pointerEventData->pressPosition.y;

                pointerEventData->dragging = (abs(moveX) > dragThreshold || abs(moveY) > dragThreshold);

                //ドラッグ開始
                if (pointerEventData->dragging) {
                    pointerEventData->SetPointerDrag(result.GetHitGameObject());
                    ExecuteEvent<IBeginDragHandler>(pointerEventData->GetPointerDrag(), pointerEventData, &IBeginDragHandler::Execute);
                }
            }
            else {
                //ドラッグ中
                ExecuteEvent<IDragHandler>(pointerEventData->GetPointerDrag(), pointerEventData, &IDragHandler::Execute);
            }
        }
        else if (InputSystem::GetInputState("ok", InputStateMask::Release, DeviceFlags::MouseOnly)) {
            pointerEventData->SetLastPress(pointerEventData->GetPointerPress());
            pointerEventData->SetPointerPress(nullptr);

            //ドラッグが終わったとき
            if (pointerEventData->dragging) {
                ExecuteEvent<IEndDragHandler>(pointerEventData->GetPointerDrag(), pointerEventData, &IEndDragHandler::Execute);
            }
            pointerEventData->dragging = false;
            pointerEventData->SetPointerDrag(nullptr);

            ExecuteEvent<IPointerUpHandler>(pointerEventData->GetLastPress(), pointerEventData, &IPointerUpHandler::Execute);
            ExecuteEvent<IPointerClickHandler>(result.GetHitGameObject(), pointerEventData, &IPointerClickHandler::Execute);
        }
    }

    //キーボードやゲームパッドのSubmit判定
    if (InputSystem::GetInputState("ok", InputStateMask::Release, DeviceFlags::KeyboardAndGamePad)) {
        ExecuteEvent<ISubmitHandler>(eventSystem->GetSelectedGameObject(), axisEventData, &ISubmitHandler::Execute);
    }

    //選択オブジェクトに対する更新処理
    ExecuteEvent<IUpdateSelectedHandler>(eventSystem->GetSelectedGameObject(), pointerEventData, &IUpdateSelectedHandler::Execute);

    //Axisイベント
    {
        lastMoveDir = axisEventData->moveDir;
        MoveDirection moveDir = axisEventData->moveDir = static_cast<MoveDirection>(static_cast<int>(InputSystem::GetAxisDirection()));

        if (moveDir != lastMoveDir) {
            moveCooldown = 0.0f;
            consecutiveMoveCount = 0;
        }

        if (moveDir != MoveDirection::None)
        {
            if (moveCooldown <= 0.0f)
            {
                GameObject* selectedObj = eventSystem->GetSelectedGameObject();
                ExecuteEvent<IMoveHandler>(selectedObj, axisEventData, &IMoveHandler::Execute);
                GameObject* movedSelectedObj = eventSystem->GetSelectedGameObject();
                //選択オブジェクトが変わったら、選択オブジェクトの変更イベントを発行
                if (selectedObj != movedSelectedObj) {
                    ExecuteEvent<IDeselectHandler>(selectedObj, axisEventData, &IDeselectHandler::Execute);
                    ExecuteEvent<ISelectHandler>(movedSelectedObj, axisEventData, &ISelectHandler::Execute);
                }
                moveCooldown = (consecutiveMoveCount == 0) ? initialDelay : repeatInterval;
                consecutiveMoveCount++;
            }
        }
        else {
            moveCooldown = 0.0f;
            consecutiveMoveCount = 0;
        }

        if (moveCooldown > 0.0f) {
            moveCooldown -= deltaTime;
        }
    }

    //ホイール量更新
    pointerEventData->scrollDelta = InputSystem::GetWheelDelta();

    // ホイールイベント
    if (fabsf(pointerEventData->scrollDelta) > 0.01f) {
        ExecuteEvent<IScrollHandler>(result.GetHitGameObject(), pointerEventData, &IScrollHandler::Execute);
    }
}


void InputModule::DrawProperty()
{
#ifdef USE_IMGUI

    ImGui::DragFloat("DragThreshold", &dragThreshold, 1.0f, 0.f, 1000.f);

    ImGui::Separator();

    ImGui::DragFloat2("ScreenPos", &pointerEventData->position.x);
    ImGui::DragFloat2("LastScreenPos", &pointerEventData->lastPosition.x);
    ImGui::DragFloat2("PressPos", &pointerEventData->pressPosition.x);
    ImGui::DragFloat2("Delta", &pointerEventData->delta.x);
    ImGui::Text("PointerEnter:%s", pointerEventData->GetPointerEnter() ? pointerEventData->GetPointerEnter()->name.c_str() : "None");
    ImGui::Text("PointerPress:%s", pointerEventData->GetPointerPress() ? pointerEventData->GetPointerPress()->name.c_str() : "None");
    ImGui::Text("LastPress:%s", pointerEventData->GetLastPress() ? pointerEventData->GetLastPress()->name.c_str() : "None");
    ImGui::Text("PointerDrag:%s", pointerEventData->GetPointerDrag() ? pointerEventData->GetPointerDrag()->name.c_str() : "None");
    bool dragging = pointerEventData->dragging;
    ImGui::Checkbox("Dragging", &dragging);
    ImGui::Separator();
    ImGui::Text("HitObject:%s", pointerEventData->pointerCurrentRaycast.IsValid() ? pointerEventData->pointerCurrentRaycast.GetHitGraphic()->GetOwner()->GetName().c_str() : "None");
    ImGui::Text("CurrentSelectedGameObject:%s", eventSystem->GetSelectedGameObject() ? eventSystem->GetSelectedGameObject()->GetName().c_str() : "None");
#endif // USE_IMGUI
}