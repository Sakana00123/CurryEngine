#pragma once
#include "Gadget.h"

class Propeller : public Gadget
{
	C_REFLECT(Propeller)

public:
	public:
	Propeller() = default;
	virtual ~Propeller() override = default;
	void Start() override;
	void Update(float deltaTime) override;

	void OnRoundEnd() override; // ラウンド終了時の処理

	void OnAttachment() override; // ガジェットがオブジェクトにアタッチされたときの処理

public:
	/** @brief 回転速度（度/秒） **/
	C_PROPERTY()
		float rotationSpeed = 360.0f; // 1秒で1回転（360度）

};