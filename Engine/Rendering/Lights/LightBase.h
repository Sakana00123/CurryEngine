#pragma once
#include "Engine/Core/Component.h"

class LightBase : public Component
{
	C_REFLECT(LightBase)
public:
	LightBase() = default;
	virtual ~LightBase() override = default;
	
	virtual void OnEnable() override = 0; // ライトの有効化時の処理（例：ライトリストへの登録）
	virtual void OnDisable() override = 0; // ライトの無効化時の処理（例：ライトリストからの削除）

	bool m_InternalEnable = true; // ライトの有効状態（内部管理用）
};