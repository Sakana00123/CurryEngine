#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/GameObject.h"
#include "BaseInputModule.h"
#include <memory>
#include "PointerEventData.h"
#include "AxisEventData.h"

class InputModule : public BaseInputModule
{
	C_REFLECT(InputModule)
private:
    float dragThreshold = 2.f;

    float moveCooldown = 0.0f;
    int consecutiveMoveCount = 0;

    float initialDelay = 0.4f;
    float repeatInterval = 0.1f;
    MoveDirection lastMoveDir = MoveDirection::None;

    std::shared_ptr<PointerEventData> pointerEventData;
    std::shared_ptr<AxisEventData> axisEventData;
public:
    /**
	 * @brief コンストラクタ。
	 * @details `EventSystem` を受け取り、内部で `PointerEventData` と `AxisEventData` を初期化します。
     */
    InputModule();

    /**
	 * @brief 有効化処理。
	 * @details `EventSystem` にこのモジュールを登録し、必要な初期化を行います。通常はシーン開始時に呼び出されます。
     */
    void OnEnable() override;

	/**
	 * @brief 無効化処理。
     * @details `EventSystem` からこのモジュールを登録解除し、必要なクリーンアップを行います。通常はシーン終了時に呼び出されます。
	 */
    void OnDisable() override;

	/**
	 * @brief 入力イベントデータを取得します。
	 * @return `PointerEventData` への共有ポインタ。現在の入力状態やイベント情報を含みます。
	 * @details イベントハンドラや他のコンポーネントがこのデータを参照して、入力に基づく処理を行うことができます。
	 */
    std::shared_ptr<PointerEventData> GetEventData();

    /**
	 * @brief 入力処理を行います。
	 * @details 毎フレーム呼び出され、入力状態の更新やイベントの発行を行います。
	 * @param deltaTime 前フレームからの経過時間（秒）。イベントの発行タイミングや連続入力の処理に使用されます。
     */
    void Process(float deltaTime) override;

    /**
	 * @brief 指定したイベントハンドラを実行します。
	 * @tparam T イベントハンドラの型。例: `IPointerDownHandler`、`IMoveHandler` など。
	 * @param target イベントを処理する対象の `GameObject`。このオブジェクトとその親オブジェクトがイベントハンドラを持っているか探索されます。
	 * @param eventData イベントに関連するデータ。イベントハンドラに渡されます。
	 * @param callback イベントハンドラのメンバ関数へのポインタ。例: `&IPointerDownHandler::Execute`。
	 * @details 指定した `GameObject` とその親オブジェクトを再帰的に探索し、型 `T` のイベントハンドラが見つかった場合に `callback` を呼び出します。イベントハンドラが見つからない場合は、親オブジェクトを探索し続けます。
     */
    template<class T>
    void ExecuteEvent(GameObject* target, std::shared_ptr<BaseEventData> eventData, std::function<void(T*, BaseEventData*)> callback) {
        if (target) {
            bool found = false;
            for (auto& component : target->GetAllComponents()) {
                T* handler = dynamic_cast<T*>(component.get());
                if (handler) {
                    callback(handler, eventData.get());
                    found = true;
                }
            }
            //イベントハンドラが見つからなかったら、親を探索
            if (!found) {
                ExecuteEvent<T>(target->parent, eventData, callback);
            }
        }
    }

    Vector2 GetPointerPosition() const override {
        return pointerEventData->position;
    }

    void DrawProperty() override;
};