#include "pch.h"
#include "TestEnemy.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Physics/Rigidbody.h"

REGISTER_COMPONENT(TestEnemy, "Enemy")

void TestEnemy::Start()
{
}

void TestEnemy::Update(float deltaTime)
{
	// ターゲットを追いかける
	auto targetTransform = std::dynamic_pointer_cast<Transform>(ObjectManager::FindComponent(targetId));
	if (targetTransform)
	{
		Vector3 dir = targetTransform->GetPosition() - GetTransform()->GetPosition();
		float distance = dir.Length();
		dir.y = 0; // 水平方向のみに移動

		/*if (auto rigidbody = GetOwner()->GetComponent<Rigidbody>())
		{
			rigidbody->AddForce(dir.Normalize() * moveSpeed, ForceMode::Force);
		}*/
		GetTransform()->Translate(dir.Normalize() * moveSpeed * deltaTime);

		if (distance < destroyThreshold)
		{
			// ターゲットに近づきすぎたら消す
			GetOwner()->Destroy();
		}

	}

	// 死亡判定
	if (GetTransform()->GetPosition().y < deathPositionY)
	{
		GetOwner()->Destroy();
	}
}