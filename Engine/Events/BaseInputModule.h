#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Math/Vector2.h"
class EventSystem;

class BaseInputModule : public Component
{
	C_REFLECT(BaseInputModule)
protected:
    EventSystem* eventSystem = nullptr;

public:
    BaseInputModule();
    virtual ~BaseInputModule() = default;

    virtual void ActivateModule() {}
    virtual void DeactivateModule() {}
    virtual bool IsModuleSupported() const { return true; }
    virtual bool ShouldActivateModule() const { return true; }

    // 毎フレーム呼ばれる
    virtual void Process(float deltaTime) = 0;

    // 入力座標を取得（オーバーライド推奨）
    virtual Vector2 GetPointerPosition() const { return { 0, 0 }; }

    // イベント生成などに使用
    EventSystem* GetEventSystem() const { return eventSystem; }
};