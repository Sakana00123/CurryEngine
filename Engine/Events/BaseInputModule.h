#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Math/Vector2.h"
class EventSystem;

/** @brief BaseInputModule を表すクラスです。 */
class BaseInputModule : public Component
{
	C_REFLECT(BaseInputModule)
protected:
    EventSystem* eventSystem = nullptr;

public:
    /**
     * @brief BaseInputModule を構築します。
     */
    BaseInputModule();
    /**
     * @brief BaseInputModule を破棄します。
     */
    virtual ~BaseInputModule() = default;

    virtual void ActivateModule() {}
    virtual void DeactivateModule() {}
    virtual bool IsModuleSupported() const { return true; }
    virtual bool ShouldActivateModule() const { return true; }

    // 毎フレーム呼ばれる
    /**
     * @brief Process の処理を行います。
     */
    virtual void Process(float deltaTime) = 0;

    // 入力座標を取得（オーバーライド推奨）
    virtual Vector2 GetPointerPosition() const { return { 0, 0 }; }

    // イベント生成などに使用
    EventSystem* GetEventSystem() const { return eventSystem; }
};