#include "pch.h"
#include "HingeJoint.h"
#include "Rigidbody.h"
#include "Engine/Scenes/Scene.h"
#include <PxPhysicsAPI.h>

REGISTER_COMPONENT_WITH_ATTRIBUTES(HingeJoint, "Physics", ComponentAttributes::RequiredComponent, { "Rigidbody" })

void HingeJoint::Start()
{
	//CreateJoint();
}

void HingeJoint::LateUpdate(float deltaTime)
{
	// connectedBody が無効になった場合はジョイントを破棄します。
	if (!connectedBody.IsValid())
	{
		DestroyJoint();
	}
	if (!pxJoint)
	{
		// connectedBody が有効で、まだジョイントが作成されていない場合はジョイントを作成します。
		CreateJoint();
	}
}

void HingeJoint::OnDestroy()
{
	DestroyJoint();
}

void HingeJoint::CreateJoint()
{
	// ここで PhysX の回転ジョイントを作成する処理を実装します。
	// connectedBody が有効な場合にのみジョイントを作成します。
	if (connectedBody.IsValid())
	{
		// connectedBody から Rigidbody コンポーネントを取得して、PhysX のアクターを取得します。
		if (const auto& connectedRigidbody = std::reinterpret_pointer_cast<Rigidbody>(ObjectManager::FindComponent(connectedBody)))
		{
			physx::PxRigidActor* actorA = Physics::GetActor(Physics::GetActorHandle(GetOwner()->GetTransform()));
			physx::PxRigidActor* actorB = Physics::GetActor(Physics::GetActorHandle(connectedRigidbody->GetTransform()));
			if (actorA && actorB)
			{
				// Z軸を中心に回転させるため、ローカルのX軸をZ軸方向へ向ける（Y軸周りに90度回転）
				physx::PxQuat rotZ(physx::PxHalfPi, physx::PxVec3(0, 1, 0));

				physx::PxTransform localFrameA = physx::PxTransform(physx::PxVec3(0, 0.5f, 0), rotZ);
				physx::PxTransform localFrameB = physx::PxTransform(physx::PxVec3(0, 0, 0), rotZ);

				// 双方の位置の中点を計算して、ローカルフレームの位置を調整します。
				{
					physx::PxVec3 posA = actorA->getGlobalPose().p;
					physx::PxVec3 posB = actorB->getGlobalPose().p;
					physx::PxVec3 midPoint = (posA + posB) * 0.5f;

					localFrameA.p = midPoint - posA;
					localFrameB.p = midPoint - posB;

				}

				// PhysX の回転ジョイントを作成します。
				pxJoint = physx::PxRevoluteJointCreate(*Physics::GetPhysics(), actorA, localFrameA, actorB, localFrameB);
				if (pxJoint)
				{
					// 1. 角度制限を有効化する
					pxJoint->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eLIMIT_ENABLED, true);

					// 2. 制限角度を指定する（例: -30度 から 30度）
					float limitAngle = physx::PxPi / 6.0f;
					physx::PxJointAngularLimitPair limit(-limitAngle, limitAngle);

					// 3. 反動（バウンス）とスプリング挙動を完全に消去する（硬い衝突）
					limit.restitution = 0.5f;
					limit.stiffness = 0.5f;
					limit.damping = 0.5f;

					pxJoint->setLimit(limit);
				}
				else
				{
					// ジョイントの作成に失敗した場合のエラーハンドリング
					std::cerr << "Failed to create hinge joint." << std::endl;
				}
			}
		}
	}
}

void HingeJoint::DestroyJoint()
{
	if (pxJoint)
	{
		pxJoint->release();
		pxJoint = nullptr;
	}
}