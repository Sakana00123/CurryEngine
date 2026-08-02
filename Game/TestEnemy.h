#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"


class TestEnemy : public Component
{
	C_REFLECT(TestEnemy)
public:
	/**@brief 移動速度。*/
	C_PROPERTY()
	float moveSpeed = 7.0f;

	/**@brief 追いかける対象。 */
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Transform"))
	ObjectId targetId;

	/**@brief 死亡判定のY座標。*/
	C_PROPERTY()
	float deathPositionY = -10.0f;

	/**@brief ターゲットに近づきすぎたときの距離閾値。*/
	C_PROPERTY()
	float destroyThreshold = 0.1f;

public:
	TestEnemy() = default;
	virtual ~TestEnemy() = default;
	
	void Start() override;
	
	void Update(float deltaTime) override;
};