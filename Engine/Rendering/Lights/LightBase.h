#pragma once
#include "Engine/Core/Component.h"

/** @brief LightBase を表すクラスです。 */
class LightBase : public Component
{
	C_REFLECT(LightBase)
public:
	/**
	 * @brief LightBase を構築します。
	 */
	LightBase() = default;
	/**
	 * @brief LightBase を破棄します。
	 */
	virtual ~LightBase() override = default;
	
	/**
	 * @brief Enable イベントを処理します。
	 */
	virtual void OnEnable() override = 0; // ライトの有効化時の処理（例：ライトリストへの登録）
	/**
	 * @brief Disable イベントを処理します。
	 */
	virtual void OnDisable() override = 0; // ライトの無効化時の処理（例：ライトリストからの削除）

	bool m_InternalEnable = true; // ライトの有効状態（内部管理用）
};