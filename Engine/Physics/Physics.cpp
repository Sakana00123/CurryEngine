#include "pch.h"
#include "Physics.h"

#include "imgui_internal.h"
#include "Engine/Physics/Rigidbody.h"
#include "Engine/Physics/Collider.h"
#include "Engine/Editor/Console.h"

#include "Engine/Utils/JsonFileHandler.h"
#include "Engine/Rendering/Renderers/DebugRenderer.h"
#include "Engine/Input/InputSystem.h"

#include "Engine/Rendering/Camera/EditorCamera.h"

// シーンのレイキャストに必要なインクルード
#include "Engine/Rendering/Camera/CameraSystem.h"
#include "Engine/Rendering/Camera/CameraComponent.h"
#include "Engine/Scenes/SceneManager.h"

std::vector<Rigidbody*> Physics::s_pendingRigidbodies;
std::vector<Collider*> Physics::s_pendingColliders;

// --- ヘルパー関数 ---

static physx::PxVec3 ToPxVec3(const Vector3& vec)
{
	return physx::PxVec3(vec.x, vec.y, vec.z);
}

static physx::PxQuat ToPxQuat(const Quaternion& quat)
{
	return physx::PxQuat(quat.x, quat.y, quat.z, quat.w);
}

static Vector3 ToVector3(const physx::PxVec3& vec)
{
	return Vector3(vec.x, vec.y, vec.z);
}

static Quaternion ToQuaternion(const physx::PxQuat& quat)
{
	return Quaternion(quat.x, quat.y, quat.z, quat.w);
}

// --- SimulationEventCallback クラスの実装 ---

void SimulationEventCallback::ClearTriggerStayPairs()
{
	pxTriggerStayPairs.clear();
	collisionEnterEvents.clear();
	collisionStayEvents.clear();
	collisionExitEvents.clear();
	triggerEnterEvents.clear();
	triggerExitEvents.clear();
}

void SimulationEventCallback::ClearTriggerStayPairsForShape(physx::PxShape* shape)
{
	// shape に関連するペアをすべて削除
	for (auto it = pxTriggerStayPairs.begin(); it != pxTriggerStayPairs.end(); )
	{
		const ShapePair& pair = it->first;
		physx::PxShape* shapeA = pair.first;
		physx::PxShape* shapeB = pair.second;
		if (shapeA == shape || shapeB == shape)
		{
			// 削除する前にトリガーの継続イベントが終了したときの処理を呼び出す
			Collider* colliderA = static_cast<Collider*>(shapeA->userData);
			Collider* colliderB = static_cast<Collider*>(shapeB->userData);
			if (colliderA != nullptr)
			{
				TriggerInfo info{};
				info.self = colliderA->GetOwner();
				info.other = colliderB != nullptr ? colliderB->GetOwner() : nullptr;
				info.selfCollider = colliderA;
				info.otherCollider = colliderB;
				colliderA->OnTriggerExit(info);
			}
			if (colliderB != nullptr)
			{
				TriggerInfo info{};
				info.self = colliderB->GetOwner();
				info.other = colliderA != nullptr ? colliderA->GetOwner() : nullptr;
				info.selfCollider = colliderB;
				info.otherCollider = colliderA;
				colliderB->OnTriggerExit(info);
			}
			if (shapeA != nullptr)
			{
				shapeA->userData = nullptr; // userData をクリア
			}
			if (shapeB != nullptr)
			{
				shapeB->userData = nullptr; // userData をクリア
			}

			// shape に関連するペアを削除
			it = pxTriggerStayPairs.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void SimulationEventCallback::Update()
{
	// トリガーの継続イベントを処理
	for (auto& item : pxTriggerStayPairs)
	{
		physx::PxTriggerPair& pair = item.second;
		//if (pair.status == physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS) // eNOTIFY_TOUCH_PERSISTS は PhysX ではサポートされていないため、常に継続イベントとして処理する
		{
			// トリガーが継続しているときの処理
			auto a = static_cast<Collider*>(pair.triggerShape->userData);
			auto b = static_cast<Collider*>(pair.otherShape->userData);
			if (a != nullptr)
			{
				// a に対する処理
				TriggerInfo info;
				info.self = a->GetOwner();
				info.other = b != nullptr ? b->GetOwner() : nullptr;
				info.selfCollider = a;
				info.otherCollider = b;
				a->OnTriggerStay(info);
			}
			if (b != nullptr)
			{
				// b に対する処理
				TriggerInfo info;
				info.self = b->GetOwner();
				info.other = a != nullptr ? a->GetOwner() : nullptr;
				info.selfCollider = b;
				info.otherCollider = a;
				b->OnTriggerStay(info);
			}
		}
	}
}

void SimulationEventCallback::CallCollisionEvents()
{
	// 衝突イベントを処理
	for (auto& event : collisionEnterEvents)
	{
		Collider* collider = event.first;
		const CollisionInfo& info = event.second;
		collider->OnCollisionEnter(info);
	}
	for (auto& event : collisionStayEvents)
	{
		Collider* collider = event.first;
		const CollisionInfo& info = event.second;
		collider->OnCollisionStay(info);
	}
	for (auto& event : collisionExitEvents)
	{
		Collider* collider = event.first;
		const CollisionInfo& info = event.second;
		collider->OnCollisionExit(info);
	}
	collisionEnterEvents.clear();
	collisionStayEvents.clear();
	collisionExitEvents.clear();
}

void SimulationEventCallback::CallTriggerEvents()
{
	// トリガーイベントを処理
	for (auto& event : triggerEnterEvents)
	{
		Collider* collider = event.first;
		const TriggerInfo& info = event.second;
		collider->OnTriggerEnter(info);
	}
	// TODO: 書き方キモいので後で直す
	Update(); // トリガーの継続イベントを処理
	for (auto& event : triggerExitEvents)
	{
		Collider* collider = event.first;
		const TriggerInfo& info = event.second;
		collider->OnTriggerExit(info);
	}
	triggerEnterEvents.clear();
	triggerExitEvents.clear();
}

void SimulationEventCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
	physx::PxActor* actorA = pairHeader.actors[0];
	physx::PxActor* actorB = pairHeader.actors[1];

	// アクターがすでに削除されているかチェックし、不正なアクセスを防止する
	bool removedActorA = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_0;
	bool removedActorB = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_1;

	// TODO: userDataに入れるデータが変更されたらここも変更する必要がある
	auto actorDataA = removedActorA ? nullptr : static_cast<Transform*>(actorA->userData);
	auto actorDataB = removedActorB ? nullptr : static_cast<Transform*>(actorB->userData);

	CollisionInfo infoA{};
	{
		infoA.self = actorDataA != nullptr ? actorDataA->GetOwner() : nullptr;
		infoA.other = actorDataB != nullptr ? actorDataB->GetOwner() : nullptr;
	}
	CollisionInfo infoB{};
	{
		infoB.self = actorDataB != nullptr ? actorDataB->GetOwner() : nullptr;
		infoB.other = actorDataA != nullptr ? actorDataA->GetOwner() : nullptr;
	}

	for (physx::PxU32 i = 0; i < nbPairs; ++i)
	{
		const physx::PxContactPair& pair = pairs[i];

		physx::PxShape* shapeA = pair.shapes[0];
		physx::PxShape* shapeB = pair.shapes[1];

		// TODO: userDataに入れるデータが変更されたらここも変更する必要がある
		auto a = static_cast<Collider*>(shapeA->userData);
		auto b = static_cast<Collider*>(shapeB->userData);
		
		if (pair.flags & physx::PxContactPairFlag::eREMOVED_SHAPE_0)
		{
			// shapeA はすでに削除されているため、a に対する処理は行わない
			a = nullptr;
		}
		if (pair.flags & physx::PxContactPairFlag::eREMOVED_SHAPE_1)
		{
			// shapeB はすでに削除されているため、b に対する処理は行わない
			b = nullptr;
		}

		physx::PxContactPairPoint contactPoints[MAX_CONTACTS_PER_PAIR]; // 衝突点の情報を格納する配列
		physx::PxU32 contactCount = pair.extractContacts(contactPoints, _countof(contactPoints));

		// infoA,infoB に衝突点の情報を格納
		{
			infoA.selfCollider = a;
			infoA.otherCollider = b;
			infoA.impulse = 0.f;

			infoB.selfCollider = b;
			infoB.otherCollider = a;
			infoB.impulse = 0.f;

			// 1つのループで両方処理する
			for (physx::PxU32 j = 0; j < contactCount; ++j)
			{
				const physx::PxContactPairPoint& contactPoint = contactPoints[j];
				Vector3 point = ToVector3(contactPoint.position);
				Vector3 normal = ToVector3(contactPoint.normal);
				Vector3 impulse = ToVector3(contactPoint.impulse);

				// infoA 用
				ContactPoint& contactInfoA = infoA.contacts.emplace_back();
				contactInfoA.point = point;
				contactInfoA.normal = normal;
				contactInfoA.separation = contactPoint.separation;
				contactInfoA.thisCollider = a;
				contactInfoA.otherCollider = b;
				infoA.impulse += impulse;

				// infoB 用 (法線と衝撃量の反転)
				ContactPoint& contactInfoB = infoB.contacts.emplace_back();
				contactInfoB.point = point;
				contactInfoB.normal = -normal;
				contactInfoB.separation = contactPoint.separation;
				contactInfoB.thisCollider = b;
				contactInfoB.otherCollider = a;
				infoB.impulse += -impulse;
			}
		}

		// pair.events は、衝突イベントの種類を示すフラグの組み合わせです。これをチェックして、どのイベントが発生したかを判断します。
		if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			// 衝突が開始したときの処理
			if (a != nullptr)
			{
				// a に対する処理
				collisionEnterEvents.push_back({ a, std::move(infoA) });
			}
			if (b != nullptr)
			{
				// b に対する処理
				collisionEnterEvents.push_back({ b, std::move(infoB) });
			}
		}
		else if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
		{
			// 衝突が継続しているときの処理
			if (a != nullptr)
			{
				// a に対する処理
				collisionStayEvents.push_back({ a, std::move(infoA) });
			}
			if (b != nullptr)
			{
				// b に対する処理
				collisionStayEvents.push_back({ b, std::move(infoB) });
			}
		}
		else if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
		{
			// 衝突が終了したときの処理
			if (a != nullptr)
			{
				// a に対する処理
				collisionExitEvents.push_back({ a, std::move(infoA) });
			}
			if (b != nullptr)
			{
				// b に対する処理
				collisionExitEvents.push_back({ b, std::move(infoB) });
			}
		}
	}
}

void SimulationEventCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
	// すでに処理されたペアを追跡するためのセット
	std::set<std::pair<void*, void*>> processedPairs;

	// pairs 配列には、トリガーイベントが発生したすべてのペアが含まれています。これをループして処理します。
	for (physx::PxU32 i = 0; i < count; ++i)
	{
		const physx::PxTriggerPair& pair = pairs[i];

		if (pair.flags & physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER || pair.flags & physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER)
		{
			// どちらかの形状がすでに削除されている場合は、トリガーイベントを処理しない
			continue;
		}

		void* triggerActorPtr = pair.triggerActor;
		void* otherActorPtr = pair.otherActor;

		auto pairKey = (triggerActorPtr < otherActorPtr) ? std::make_pair(triggerActorPtr, otherActorPtr) : std::make_pair(otherActorPtr, triggerActorPtr);

		if (processedPairs.contains(pairKey))
		{
			// すでに処理されたペアの場合はスキップ
			Console::LogWarning(std::format("Duplicate trigger event detected for actors: {} and {}. Skipping duplicate event.", triggerActorPtr, otherActorPtr));
			continue;
		}
		// ペアを処理した後、セットに追加して重複処理を防止
		processedPairs.insert(pairKey);

		// ペアを処理する
		TriggerInfo triggerInfo, otherInfo;
		auto trigger = static_cast<Collider*>(pair.triggerShape->userData);
		auto other = static_cast<Collider*>(pair.otherShape->userData);
		if (trigger != nullptr)
		{
			triggerInfo.self = trigger->GetOwner();
			triggerInfo.other = other != nullptr ? other->GetOwner() : nullptr;
			triggerInfo.selfCollider = trigger;
			triggerInfo.otherCollider = other;
		}
		if (other != nullptr)
		{
			otherInfo.self = other->GetOwner();
			otherInfo.other = trigger != nullptr ? trigger->GetOwner() : nullptr;
			otherInfo.selfCollider = other;
			otherInfo.otherCollider = trigger;
		}


		// ここでトリガーイベントを処理するコードを追加できます
		if (pair.status & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			// トリガーに入ったときの処理
			ShapePair shapePair = { pair.triggerShape, pair.otherShape };

			if (trigger != nullptr)
			{
				triggerEnterEvents.push_back({ trigger, triggerInfo });
			}
			if (other != nullptr)
			{
				triggerEnterEvents.push_back({ other, otherInfo });
			}

			// トリガーの継続イベントを管理するためのマップに追加
			pxTriggerStayPairs[shapePair] = pair;
		}
		else if (pair.status & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
		{
			// トリガーから出たときの処理
			if (trigger != nullptr)
			{
				triggerExitEvents.push_back({ trigger, triggerInfo });
			}
			if (other != nullptr)
			{
				triggerExitEvents.push_back({ other, otherInfo });
			}

			// トリガーの継続イベントを管理するためのマップから削除
			ShapePair shapePair = { pair.triggerShape, pair.otherShape };
			pxTriggerStayPairs.erase(shapePair);
		}
	}
}

// --- FilterShader クラスの実装 ---

physx::PxFilterFlags FilterShader::SimulationFilter(
	physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
	// レイヤーによる衝突判定
	// filterData.word0 には、自身のレイヤーIDが格納されている。
	// filterData.word1 には、衝突させたいレイヤーのビットフラグが格納されている。
	bool ab = (filterData0.word1 & filterData1.word0) == 0; // filterData0 の衝突させたいレイヤーに filterData1 のレイヤーID が含まれていない場合は、衝突判定を行わないようにする
	bool ba = (filterData1.word1 & filterData0.word0) == 0; // filterData1 の衝突させたいレイヤーに filterData0 のレイヤーID が含まれていない場合は、衝突判定を行わないようにする

	if (ab || ba)
	{
		return physx::PxFilterFlag::eSUPPRESS; // どちらか一方のレイヤーがもう一方の衝突させたいレイヤーに含まれていない場合は、衝突判定を行わないようにする
	}

	// 片方がトリガーの場合は、トリガーイベントを発生させるためのフラグを設定して終了
	if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
		return physx::PxFilterFlag::eDEFAULT;
	}

	// 衝突イベントを発生させるためのフラグを設定
	pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

	pairFlags |= physx::PxPairFlag::eDETECT_CCD_CONTACT; // CCDを使用している場合に、衝突イベントを発生させるためのフラグを追加

	pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND | physx::PxPairFlag::eNOTIFY_TOUCH_LOST | physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS | physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
	
	return physx::PxFilterFlag::eDEFAULT;
}

physx::PxQueryHitType::Enum FilterShader::preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags)
{
	physx::PxFilterData shapeFilterData = shape->getQueryFilterData();
	// レイヤーによるクエリ判定
	if ((shapeFilterData.word0 & filterData.word0) == 0)
	{
		return physx::PxQueryHitType::eNONE;
	}
	// IgnoreRaycast レイヤーを持つオブジェクトは、レイキャストのクエリに対してヒットしないようにする
	if ((shapeFilterData.word0 & LayerMasks::IgnoreRaycast) != 0)
	{
		return physx::PxQueryHitType::eNONE;
	}

	//return physx::PxQueryHitType::eTOUCH;
	return physx::PxQueryHitType::eBLOCK; // ブロックにすることで、クエリがヒットした位置で止まるようにする。eTOUCH にすると、クエリがヒットしてもそのまま進み続けてしまうため、ブロックにするのが適切。
}

physx::PxQueryHitType::Enum FilterShader::postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit, const physx::PxShape* shape, const physx::PxRigidActor* actor)
{
	return physx::PxQueryHitType::eBLOCK;
}

// --- 内部で使用するユーティリティ関数 ---
static physx::PxRigidDynamic* GetRigidDynamic(const ActorHandle& handle)
{
	if (physx::PxRigidActor* actor = Physics::GetActor(handle))
	{
		if (actor->getType() == physx::PxActorType::eRIGID_DYNAMIC)
		{
			return actor->is<physx::PxRigidDynamic>(); // Actorが動的な場合はPxRigidDynamic*を返す
		}
	}
	return nullptr; // Actorが存在しないか、動的なActorでない場合はnullptrを返す
}

static MaterialHandle CreateMaterialHandle()
{
	static MaterialHandle nextHandle = 1; // ハンドルの初期値を1に設定
	while (Physics::GetMaterial(nextHandle) != nullptr) // すでに存在するハンドルと重複しないようにする
	{
		++nextHandle; // ハンドルをインクリメントして次の値を試す
	}
	return nextHandle++; // 現在のハンドルを返し、次のハンドルにインクリメント
}

// --- Physics クラスの実装 ---

Physics::Physics()
{
}

Physics::~Physics()
{
}

void Physics::Initialize()
{
	// 基盤生成
	{
		pxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, pxAllocator, pxErrorCallback);
		_ASSERT_EXPR(pxFoundation != nullptr, "PxCreateFoundation failed!");
	}
	// PVD 生成
	{
		pxPvd = PxCreatePvd(*pxFoundation);
		_ASSERT_EXPR(pxPvd != nullptr, "PxCreatePvd failed!");

		physx::PxPvdTransport* pxPvdTransport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		_ASSERT_EXPR(pxPvdTransport != nullptr, "PxDefaultPvdSocketTransportCreate failed!");

		bool connected = pxPvd->connect(*pxPvdTransport, physx::PxPvdInstrumentationFlag::eALL);
		Console::LogWarning(connected ? "Connected to PhysX Visual Debugger." : "Failed to connect to PhysX Visual Debugger.");
	}

	// 物理エンジン生成
	{
		// 物理エンジンの生成
		pxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *pxFoundation, physx::PxTolerancesScale(), true, pxPvd);
		_ASSERT_EXPR(pxPhysics != nullptr, "PxCreatePhysics failed!");

		// 物理エンジンの拡張機能を初期化
		PxInitExtensions(*pxPhysics, pxPvd);
	}

	// ディスパッチャー生成
	{
		pxDispatcher = physx::PxDefaultCpuDispatcherCreate(2); // スレッド数は適宜調整
		_ASSERT_EXPR(pxDispatcher != nullptr, "PxDefaultCpuDispatcherCreate failed!");
	}

	// シーン生成
	{
		physx::PxSceneDesc sceneDesc(pxPhysics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f); // 重力を設定(必要に応じて変更)
		sceneDesc.bounceThresholdVelocity = 0.05f; // 衝突のバウンスの閾値を設定(必要に応じて変更)
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD; // CCDを有効にするフラグを設定
		sceneDesc.cpuDispatcher = pxDispatcher;
		sceneDesc.simulationEventCallback = &m_simulationEventCallback; // 衝突イベントコールバックを設定
		sceneDesc.filterShader = FilterShader::SimulationFilter; // コリジョンフィルタリング関数を設定

		pxScene = pxPhysics->createScene(sceneDesc);
		_ASSERT_EXPR(pxScene != nullptr, "createScene failed!");
	}

	// PVDシーンクライアント設定
	{
		physx::PxPvdSceneClient* pvdClient = pxScene->getScenePvdClient();
		if (pvdClient != nullptr)
		{
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
	}

	// コントローラーマネージャー生成
	{
		pxControllerManager = PxCreateControllerManager(*pxScene);
		_ASSERT_EXPR(pxControllerManager != nullptr, "PxCreateControllerManager failed!");
		pxControllerManager->setDebugRenderingFlags(physx::PxControllerDebugRenderFlag::eALL); // コントローラーのデバッグ描画を有効にする
	}

	// デフォルトマテリアル生成
	{
		PhysicsMaterialData defaultMaterialData;
		defaultMaterialData.name = "DefaultMaterial";
		defaultMaterialData.staticFriction = DEFAULT_STATIC_FRICTION;
		defaultMaterialData.dynamicFriction = DEFAULT_DYNAMIC_FRICTION;
		defaultMaterialData.bounciness = DEFAULT_BOUNCINESS;
		defaultMaterialData.frictionCombineMode = DEFAULT_FRICTION_COMBINE_MODE;
		defaultMaterialData.bounceCombineMode = DEFAULT_BOUNCE_COMBINE_MODE;
		PhysicsMaterial defaultMaterial = CreateAndRegisterMaterial(defaultMaterialData, DEFAULT_MATERIAL_HANDLE);
	}

	// デシリアライズされたデータの復元
	{
		json serializedData;
		// ここで serializedData にデシリアライズされたデータを読み込む処理を実装
#ifdef _DEBUG
		JsonFileHandler::LoadJsonFromFile(serializedData, "./ProjectSettings/Physics.json");
#else
		JsonFileHandler::LoadJsonFromFile(serializedData, "./Settings/Physics.bin");
#endif // _DEBUG

		Deserialize(serializedData);
	}

}

void Physics::Terminate()
{
#ifdef _DEBUG
	// データのシリアライズ
	SaveSettings();
#endif // _DEBUG


	// 物理エンジンのクリーンアップ
	Clean();

	// すべてのマテリアルの解放
	for (auto& pair : m_materialMap)
	{
		if (physx::PxMaterial* material = pair.second.pxMaterial)
		{
			PX_RELEASE(material);
		}
	}
	m_materialMap.clear(); // マテリアルマップをクリア
	m_materialNameMap.clear(); // マテリアル名マップをクリア

	// 拡張機能のクリーンアップ
	PxCloseExtensions();

	// 生成したオブジェクトの解放
	PX_RELEASE(pxControllerManager);
	PX_RELEASE(pxScene);
	PX_RELEASE(pxDispatcher);
	PX_RELEASE(pxPhysics);

	// PVDの切断と解放
	if (pxPvd)
	{
		physx::PxPvdTransport* transport = pxPvd->getTransport();
		transport->disconnect();
		PX_RELEASE(pxPvd);
		PX_RELEASE(transport);
	}

	// 基盤の解放
	PX_RELEASE(pxFoundation);
}

void Physics::SaveSettings()
{
	// データのシリアライズ
	{
		json serializedData = Serialize();
		// ここで serializedData をファイルなどに保存する処理を実装
#ifdef _DEBUG
		JsonFileHandler::SaveJsonToFile(serializedData, "./ProjectSettings/Physics.json");
		JsonFileHandler::SaveJsonToFile(serializedData, "./Settings/Physics.bin");
#endif // _DEBUG
	}
}

void Physics::Clean()
{
	// 登録されたActorをすべてシーンから削除して解放
	GetScene()->lockWrite(); // シーンへの書き込みをロック

	for (auto& [key, value] : m_actorMap)
	{
		if (physx::PxRigidActor* actor = value.actor)
		{
			actor->userData = nullptr; // userData をクリアして、削除されたActorへのアクセスを防止する

			// ActorにアタッチされているすべてのShapeのuserDataをクリアして、削除されたActorへのアクセスを防止する
			physx::PxU32 numShapes = actor->getNbShapes();
			if (numShapes > 0)
			{
				std::vector<physx::PxShape*> shapes(numShapes);
				actor->getShapes(shapes.data(), numShapes);
				for (physx::PxU32 i = 0; i < numShapes; ++i)
				{
					if (physx::PxShape* shape = shapes[i])
					{
						shape->userData = nullptr; // userData をクリア
					}
				}
			}

			GetScene()->removeActor(*actor); // シーンからActorを削除
			PX_RELEASE(actor); // Actorを解放
		}
		value.transform = nullptr; // Transformへのポインタをクリア
	}
	m_actorMap.clear(); // マップをクリア

	for (auto& [handle, shape] : m_shapeMap)
	{
		if (shape)
		{
			shape->userData = nullptr; // userData をクリアして、削除されたShapeへのアクセスを防止する
			PX_RELEASE(shape); // Shapeを解放
		}
	}
	m_shapeMap.clear(); // シェイプマップをクリア

	m_simulationEventCallback.ClearTriggerStayPairs(); // トリガーの継続イベントのペアをクリア

	GetScene()->unlockWrite(); // シーンへの書き込みのロックを解除
}

// --- シリアライズ・デシリアライズ ---

json Physics::Serialize()
{
	json j;
	// ここで保存するデータをシリアライズする処理を実装

	// 登録されたマテリアルのデータをシリアライズする
	json materialsJson = json::array();
	for (const auto& pair : m_materialMap)
	{
		MaterialHandle handle = pair.first;
		const PhysicsMaterial& material = pair.second;
		if (handle == DEFAULT_MATERIAL_HANDLE)
		{
			continue; // デフォルトマテリアルは保存しない
		}
		json materialJson;
		materialJson["handle"] = handle; // ハンドルを保存
		materialJson["name"] = material.data.name; // マテリアル名を保存
		materialJson["staticFriction"] = material.data.staticFriction; // 静止摩擦係数を保存
		materialJson["dynamicFriction"] = material.data.dynamicFriction; // 動摩擦係数を保存
		materialJson["bounciness"] = material.data.bounciness; // 反発係数を保存
		materialJson["frictionCombineMode"] = static_cast<size_t>(material.data.frictionCombineMode); // 摩擦の結合モードを保存
		materialJson["bounceCombineMode"] = static_cast<size_t>(material.data.bounceCombineMode); // 反発の結合モードを保存
		materialsJson.push_back(materialJson);
	}
	j["materials"] = materialsJson; // マテリアルの配列を保存

	// レイヤーのデータを layersJson に追加する
	j["layerManager"] = LayerManager::Get().Serialize();

	return j;
}

void Physics::Deserialize(const json& j)
{
	// ここで保存されたデータをデシリアライズして復元する処理を実装

	// マテリアルのデシリアライズ
	if (j.contains("materials") && j["materials"].is_array())
	{
		for (const auto& materialJson : j["materials"])
		{
			if (materialJson.contains("handle") && materialJson.contains("name") && materialJson.contains("staticFriction") && materialJson.contains("dynamicFriction") && materialJson.contains("bounciness"))
			{
				MaterialHandle handle = materialJson["handle"].get<MaterialHandle>();
				PhysicsMaterialData materialData;
				materialData.name = materialJson["name"].get<std::string>();
				materialData.staticFriction = materialJson["staticFriction"].get<float>();
				materialData.dynamicFriction = materialJson["dynamicFriction"].get<float>();
				materialData.bounciness = materialJson["bounciness"].get<float>();
				size_t frictionCombineMode = (materialJson.contains("frictionCombineMode") ? materialJson["frictionCombineMode"].get<size_t>() : 0);
				materialData.frictionCombineMode = static_cast<PhysicMaterialCombineMode>(frictionCombineMode);
				size_t bounceCombineMode = (materialJson.contains("bounceCombineMode") ? materialJson["bounceCombineMode"].get<size_t>() : 0);
				materialData.bounceCombineMode = static_cast<PhysicMaterialCombineMode>(bounceCombineMode);
				// マテリアルを作成してマップに登録
				CreateAndRegisterMaterial(materialData, handle);
			}
		}
	}

	// レイヤーのデシリアライズ
	if (j.contains("layerManager"))
	{
		LayerManager::Get().Deserialize(j["layerManager"]);
	}

}

// --- 物理シミュレーションの更新とゲームオブジェクトへの反映 ---

void Physics::FixedUpdate(float fixedDeltaTime)
{
	// 物理シミュレーションの更新とゲームオブジェクトへの反映
	
	FlushPendingRegistrations(); // 保留中の登録をフラッシュして、シミュレーションに反映する

	// シミュレーションの更新前に、必要に応じてゲームオブジェクトの状態を物理エンジンに反映する処理を実装
	for (const auto& [handle, data] : m_actorMap)
	{
		if (physx::PxRigidActor* actor = data.actor)
		{
			// ActorHandleに対応するTransformを取得
			if (Transform* transform = data.transform)
			{
				if (!transform->IsChangedThisFrame())
				{
					// Transformがこのフレームで変更されていない場合は位置と回転を更新しない
					continue;
				}

				// Transformからワールド位置と回転を取得
				Vector3 worldPos = transform->GetWorldPosition();
				Quaternion worldRot = transform->GetWorldRotation();
				physx::PxVec3 pxWorldPos = ToPxVec3(worldPos);
				physx::PxQuat pxWorldRot = ToPxQuat(worldRot).getNormalized(); // PhysXの回転は正規化されている必要があるため、正規化してから使用する
				physx::PxTransform pxTransform(pxWorldPos, pxWorldRot);

				bool isKinematic = false;
				auto* dynamic = actor->is<physx::PxRigidDynamic>();
				if (dynamic)
				{
					if (dynamic->getRigidBodyFlags() & physx::PxRigidBodyFlag::eKINEMATIC)
					{
						if (dynamic->getActorFlags() & physx::PxActorFlag::eDISABLE_SIMULATION)
						{
							continue; // キネマティックなActorがスリープ中の場合は位置と回転を更新しない
						}
						isKinematic = true; // Actorがキネマティックな場合はフラグを立てる
					}
				}
				if (isKinematic)
				{
					// キネマティックなActorの場合はsetKinematicTargetを使用して位置と回転を更新
					dynamic->setKinematicTarget(pxTransform);
				}
				else
				{
					// Actorのグローバルポーズを更新
					actor->setGlobalPose(pxTransform);
				}
			}
		}
	}


	// 物理シミュレーションの更新
	if (fixedDeltaTime > 0.0f)
	{
		pxScene->simulate(fixedDeltaTime);
		pxScene->fetchResults(true);
	}

	// ここで物理シミュレーションの結果をゲームオブジェクトに反映する処理を実装

	m_simulationEventCallback.CallCollisionEvents(); // 衝突イベントを呼び出す
	m_simulationEventCallback.CallTriggerEvents(); // トリガーイベントを呼び出す

	// 登録されたActorの位置と回転を対応するTransformに反映
	for (const auto& [handle, data] : m_actorMap)
	{
		if (physx::PxRigidActor* actor = data.actor)
		{
			if (auto* dynamic = actor->is<physx::PxRigidDynamic>())
			{
				if (dynamic->isSleeping())
				{
					continue; // 動的なActorがスリープ中の場合は位置と回転を更新しない
				}
				if (dynamic->getRigidBodyFlags() & physx::PxRigidBodyFlag::eKINEMATIC)
				{
					continue; // キネマティックなActorの場合は位置と回転を更新しない
				}

				// Actorのグローバルポーズを取得
				physx::PxTransform globalPose = actor->getGlobalPose();
				// ActorHandleに対応するTransformを取得
				if (Transform* transform = data.transform)
				{
					// Transformに位置と回転を反映
					transform->SetWorldPosition(ToVector3(globalPose.p));
					transform->SetWorldRotation(ToQuaternion(globalPose.q));
				}
			}
		}
	}

}

void Physics::RenderDebug(RenderContext* renderContext)
{
	// ここで物理エンジンに関連するデバッグ描画を行う処理を実装
	// PhysXのデバッグ描画を呼び出す
	if (pxScene)
	{

		physx::PxShape* pxShapes[128];
		auto drawShape = [&](physx::PxShape* pxShape, const physx::PxTransform& pxShapeTransform, float contactOffset, bool sleeping)
			{
				const physx::PxGeometry& pxGeometry = pxShape->getGeometry();
				physx::PxTransform pxShapeLocalTransform = pxShape->getLocalPose(); // ローカルポーズを取得
				const physx::PxMat44 pxShapeMat = physx::PxMat44(pxShapeTransform)/* * physx::PxMat44(pxShapeLocalTransform)*/; // ワールド座標系での形状の変換行列を計算
				DirectX::XMFLOAT4X4 shapeTransform = {
					pxShapeMat.column0.x, pxShapeMat.column0.y, pxShapeMat.column0.z, pxShapeMat.column0.w,
					pxShapeMat.column1.x, pxShapeMat.column1.y, pxShapeMat.column1.z, pxShapeMat.column1.w,
					pxShapeMat.column2.x, pxShapeMat.column2.y, pxShapeMat.column2.z, pxShapeMat.column2.w,
					pxShapeMat.column3.x, pxShapeMat.column3.y, pxShapeMat.column3.z, pxShapeMat.column3.w,
				};
				Vector3 shapePosition = {
					pxShapeMat.column3.x, pxShapeMat.column3.y, pxShapeMat.column3.z
				};

				Color color(0.0f, 1.0f, 0.0f, 0.3f);

				if (physx::PxRigidActor* rigidActor = pxShape->getActor())
				{
					// Actorのタイプに応じて色を変更
					switch (rigidActor->getType())
					{
					case physx::PxActorType::eRIGID_STATIC:
						color = Color(0.0f, 1.0f, 0.0f, 0.3f); // 静的なActorは緑色
						break;
					case physx::PxActorType::eRIGID_DYNAMIC:
						color = Color(0.0f, 0.0f, 1.0f, 0.3f); // 動的なActorは青色
						break;
					case physx::PxActorType::eSOFTBODY:
						color = Color(1.0f, 0.0f, 0.0f, 0.3f); // ソフトボディのActorは赤色
						break;
					default:
						color = Color(1.0f, 1.0f, 1.0f, 0.3f); // その他のActorは白色
						break;
					};
				}

				if (sleeping)
				{
					const float dark = 0.25f;
					color.r *= dark;
					color.g *= dark;
					color.b *= dark;
				}

				switch (pxGeometry.getType())
				{
				case physx::PxGeometryType::eSPHERE:
				{
					const physx::PxSphereGeometry& pxSphereGeometry = static_cast<const physx::PxSphereGeometry&>(pxGeometry);
					// 球を描画
					{
						// 球の半径を取得
						float radius = pxSphereGeometry.radius;
						// 球を描画
						DebugRenderer::DrawSphere(shapePosition, radius, color);
					}
					break;
				}
				case physx::PxGeometryType::ePLANE:
				{
					const physx::PxPlaneGeometry& pxPlaneGeometry = static_cast<const physx::PxPlaneGeometry&>(pxGeometry);
					// 平面を描画
					{
						// 平面の法線はActorの回転に依存するため、Actorの回転を取得して法線を計算
						Vector3 normal = ToVector3(pxShapeMat.rotate(physx::PxVec3(0, 1, 0)));

						// 平面の描画サイズは適当に設定（必要に応じて調整）
						float size = 1.0f;

						// 平面を描画
						DebugRenderer::DrawPlane(shapePosition, normal, size, color);
					}
					break;
				}
				case physx::PxGeometryType::eCAPSULE:
				{
					const physx::PxCapsuleGeometry& pxCapsuleGeometry = static_cast<const physx::PxCapsuleGeometry&>(pxGeometry);

					// カプセルを描画
					{
						// カプセルの半径と高さを取得
						float radius = pxCapsuleGeometry.radius;
						float halfHeight = pxCapsuleGeometry.halfHeight;
						// カプセルの上端と下端の位置を計算
						physx::PxVec3 topOffset(0, halfHeight, 0);
						physx::PxVec3 bottomOffset(0, -halfHeight, 0);

						physx::PxMat44 capsuleMat = pxShapeMat;
						// カプセルだけ、Z軸回転させているので、戻すために逆の回転をかける
						Quaternion localRot = Quaternion::FromEuler({ 0.0f, 0.0f, -90.0f }); // Z軸に-90度回転させる
						physx::PxQuat rotation = ToPxQuat(localRot).getNormalized();
						capsuleMat = capsuleMat * physx::PxMat44(rotation);

						physx::PxVec3 topPosition = capsuleMat.transform(topOffset);
						physx::PxVec3 bottomPosition = capsuleMat.transform(bottomOffset);

						Vector3 start = ToVector3(topPosition);
						Vector3 end = ToVector3(bottomPosition);

						// カプセルを描画
						DebugRenderer::DrawCapsule(start, end, radius, color);
					}

					break;
				}
				case physx::PxGeometryType::eBOX:
				{
					const physx::PxBoxGeometry& pxBoxGeometry = static_cast<const physx::PxBoxGeometry&>(pxGeometry);
					
					// ボックスを描画
					{
						// ボックスの半サイズを取得
						Vector3 size = ToVector3(pxBoxGeometry.halfExtents) * 2.0f; // 半サイズを2倍してフルサイズに変換
						// ボックスを描画
						DebugRenderer::DrawBox(shapePosition, size, color);
					}
					break;
				}
				case physx::PxGeometryType::eCONVEXMESH:
				{
					const physx::PxConvexMeshGeometry& pxConvexMeshGeometry = static_cast<const physx::PxConvexMeshGeometry&>(pxGeometry);

					const physx::PxConvexMesh& pxConvexMesh = *pxConvexMeshGeometry.convexMesh;
					const physx::PxVec3* pxVertices = pxConvexMesh.getVertices();
					const physx::PxU8* pxIndices = pxConvexMesh.getIndexBuffer();

					const physx::PxVec3 pxScale = pxConvexMeshGeometry.scale.scale;
					const physx::PxQuat pxRotation = pxConvexMeshGeometry.scale.rotation.getNormalized();
					DirectX::XMMATRIX Scale = DirectX::XMMatrixScaling(pxScale.x, pxScale.y, pxScale.z);
					DirectX::XMMATRIX Rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(pxRotation.x, pxRotation.y, pxRotation.z, pxRotation.w));
					DirectX::XMMATRIX ShapeTransform = Scale * Rotation * DirectX::XMLoadFloat4x4(&shapeTransform);

					const physx::PxU32 pxNumPolygons = pxConvexMesh.getNbPolygons();
					for (physx::PxU32 pxPolygonIndex = 0; pxPolygonIndex < pxNumPolygons; ++pxPolygonIndex)
					{
						physx::PxHullPolygon pxHullPolygon;
						pxConvexMesh.getPolygonData(pxPolygonIndex, pxHullPolygon);

						const physx::PxU32 pxNumTriangles = pxHullPolygon.mNbVerts - 2;
						const physx::PxU8 pxIndex0 = pxIndices[pxHullPolygon.mIndexBase + 0];
						const physx::PxVec3& pxVertex0 = pxVertices[pxIndex0];

						for (physx::PxU32 pxTriangleIndex = 0; pxTriangleIndex < pxNumTriangles; ++pxTriangleIndex)
						{
							const physx::PxU8 pxIndex1 = pxIndices[pxHullPolygon.mIndexBase + 0 + pxTriangleIndex + 1];
							const physx::PxU8 pxIndex2 = pxIndices[pxHullPolygon.mIndexBase + 0 + pxTriangleIndex + 2];
							const physx::PxVec3& pxVertex1 = pxVertices[pxIndex1];
							const physx::PxVec3& pxVertex2 = pxVertices[pxIndex2];

							DirectX::XMVECTOR V0 = DirectX::XMVectorSet(pxVertex0.x, pxVertex0.y, pxVertex0.z, 0);
							DirectX::XMVECTOR V1 = DirectX::XMVectorSet(pxVertex1.x, pxVertex1.y, pxVertex1.z, 0);
							DirectX::XMVECTOR V2 = DirectX::XMVectorSet(pxVertex2.x, pxVertex2.y, pxVertex2.z, 0);
							V0 = DirectX::XMVector3Transform(V0, ShapeTransform);
							V1 = DirectX::XMVector3Transform(V1, ShapeTransform);
							V2 = DirectX::XMVector3Transform(V2, ShapeTransform);
							Vector3 v0, v1, v2;
							DirectX::XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&v0), V0);
							DirectX::XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&v1), V1);
							DirectX::XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&v2), V2);

							DebugRenderer::AddVertex(v0, color);
							DebugRenderer::AddVertex(v1, color);
							DebugRenderer::AddVertex(v1, color);
							DebugRenderer::AddVertex(v2, color);
							DebugRenderer::AddVertex(v2, color);
							DebugRenderer::AddVertex(v0, color);
						}
					}
					break;
				}
				case physx::PxGeometryType::ePARTICLESYSTEM:
				{
					const physx::PxParticleSystemGeometry& pxParticleSystemGeometry = static_cast<const physx::PxParticleSystemGeometry&>(pxGeometry);
					break;
				}
				case physx::PxGeometryType::eTETRAHEDRONMESH:
				{
					const physx::PxTetrahedronMeshGeometry& pxTetrahedronMeshGeometry = static_cast<const physx::PxTetrahedronMeshGeometry&>(pxGeometry);
					const physx::PxTetrahedronMesh& pxTetrahedronMesh = *pxTetrahedronMeshGeometry.tetrahedronMesh;
					const physx::PxVec3* pxVertices = pxTetrahedronMesh.getVertices();
					const void* pxIndices = pxTetrahedronMesh.getTetrahedrons();
					const physx::PxU32* pxIndices32 = static_cast<const physx::PxU32*>(pxIndices);
					const physx::PxU16* pxIndices16 = static_cast<const physx::PxU16*>(pxIndices);
					const physx::PxU32 pxHas16BitIndices = pxTetrahedronMesh.getTetrahedronMeshFlags() & physx::PxTetrahedronMeshFlag::e16_BIT_INDICES;

					DirectX::XMMATRIX ShapeTransform = DirectX::XMLoadFloat4x4(&shapeTransform);

					physx::PxU32 pxNumTetrahedrons = pxTetrahedronMesh.getNbTetrahedrons();
					for (physx::PxU32 pxTetrahedronIndex = 0; pxTetrahedronIndex < pxNumTetrahedrons; ++pxTetrahedronIndex)
					{
						physx::PxU32 pxIndex[4];
						if (pxHas16BitIndices)
						{
							pxIndex[0] = *pxIndices16++;
							pxIndex[1] = *pxIndices16++;
							pxIndex[2] = *pxIndices16++;
							pxIndex[3] = *pxIndices16++;
						}
						else
						{
							pxIndex[0] = *pxIndices32++;
							pxIndex[1] = *pxIndices32++;
							pxIndex[2] = *pxIndices32++;
							pxIndex[3] = *pxIndices32++;
						}

						const int tetFaces[4][3] = { {0,2,1}, {0,1,3}, {0,3,2}, {1,2,3} };
						for (physx::PxU32 i = 0; i < 4; ++i)
						{
							const physx::PxVec3& pxVertex0 = pxVertices[pxIndex[tetFaces[i][0]]];
							const physx::PxVec3& pxVertex1 = pxVertices[pxIndex[tetFaces[i][1]]];
							const physx::PxVec3& pxVertex2 = pxVertices[pxIndex[tetFaces[i][2]]];

							DirectX::XMVECTOR V0 = DirectX::XMVectorSet(pxVertex0.x, pxVertex0.y, pxVertex0.z, 0);
							DirectX::XMVECTOR V1 = DirectX::XMVectorSet(pxVertex1.x, pxVertex1.y, pxVertex1.z, 0);
							DirectX::XMVECTOR V2 = DirectX::XMVectorSet(pxVertex2.x, pxVertex2.y, pxVertex2.z, 0);
							V0 = DirectX::XMVector3Transform(V0, ShapeTransform);
							V1 = DirectX::XMVector3Transform(V1, ShapeTransform);
							V2 = DirectX::XMVector3Transform(V2, ShapeTransform);
							Vector3 v0, v1, v2;
							DirectX::XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&v0), V0);
							DirectX::XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&v1), V1);
							DirectX::XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&v2), V2);

							DebugRenderer::AddVertex(v0, color);
							DebugRenderer::AddVertex(v1, color);
							DebugRenderer::AddVertex(v1, color);
							DebugRenderer::AddVertex(v2, color);
							DebugRenderer::AddVertex(v2, color);
							DebugRenderer::AddVertex(v0, color);
						}
					}
					break;
				}
				case physx::PxGeometryType::eTRIANGLEMESH:
				{
					const physx::PxTriangleMeshGeometry& pxTriangleMeshGeometry = static_cast<const physx::PxTriangleMeshGeometry&>(pxGeometry);
					const physx::PxTriangleMesh& pxTriangleMesh = *pxTriangleMeshGeometry.triangleMesh;
					const physx::PxVec3* pxVertices = pxTriangleMesh.getVertices();
					const void* pxIndices = pxTriangleMesh.getTriangles();
					const physx::PxU32* pxIndices32 = static_cast<const physx::PxU32*>(pxIndices);
					const physx::PxU16* pxIndices16 = static_cast<const physx::PxU16*>(pxIndices);
					const physx::PxU32 pxHas16BitIndices = pxTriangleMesh.getTriangleMeshFlags() & physx::PxTriangleMeshFlag::e16_BIT_INDICES;

					const physx::PxVec3 pxScale = pxTriangleMeshGeometry.scale.scale;
					const physx::PxQuat pxRotation = pxTriangleMeshGeometry.scale.rotation.getNormalized();
					DirectX::XMMATRIX Scale = DirectX::XMMatrixScaling(pxScale.x, pxScale.y, pxScale.z);
					DirectX::XMMATRIX Rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(pxRotation.x, pxRotation.y, pxRotation.z, pxRotation.w));
					DirectX::XMMATRIX ShapeTransform = Scale * Rotation * DirectX::XMLoadFloat4x4(&shapeTransform);

					physx::PxU32 pxNumTriangles = pxTriangleMeshGeometry.triangleMesh->getNbTriangles();
					for (physx::PxU32 pxTriangleIndex = 0; pxTriangleIndex < pxNumTriangles; ++pxTriangleIndex)
					{
						physx::PxU32 pxIndex0, pxIndex1, pxIndex2;
						if (pxHas16BitIndices)
						{
							pxIndex0 = *pxIndices16++;
							pxIndex1 = *pxIndices16++;
							pxIndex2 = *pxIndices16++;
						}
						else
						{
							pxIndex0 = *pxIndices32++;
							pxIndex1 = *pxIndices32++;
							pxIndex2 = *pxIndices32++;
						}
						const physx::PxVec3& pxVertex0 = pxVertices[pxIndex0];
						const physx::PxVec3& pxVertex1 = pxVertices[pxIndex1];
						const physx::PxVec3& pxVertex2 = pxVertices[pxIndex2];
						DirectX::XMVECTOR V0 = DirectX::XMVectorSet(pxVertex0.x, pxVertex0.y, pxVertex0.z, 0);
						DirectX::XMVECTOR V1 = DirectX::XMVectorSet(pxVertex1.x, pxVertex1.y, pxVertex1.z, 0);
						DirectX::XMVECTOR V2 = DirectX::XMVectorSet(pxVertex2.x, pxVertex2.y, pxVertex2.z, 0);
						V0 = DirectX::XMVector3Transform(V0, ShapeTransform);
						V1 = DirectX::XMVector3Transform(V1, ShapeTransform);
						V2 = DirectX::XMVector3Transform(V2, ShapeTransform);
						Vector3 v0, v1, v2;
						DirectX::XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&v0), V0);
						DirectX::XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&v1), V1);
						DirectX::XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&v2), V2);

						DebugRenderer::AddVertex(v0, color);
						DebugRenderer::AddVertex(v1, color);
						DebugRenderer::AddVertex(v1, color);
						DebugRenderer::AddVertex(v2, color);
						DebugRenderer::AddVertex(v2, color);
						DebugRenderer::AddVertex(v0, color);
					}
					break;
				}
				case physx::PxGeometryType::eHEIGHTFIELD:
				{
					const physx::PxHeightFieldGeometry& pxHeightFieldGeometry = static_cast<const physx::PxHeightFieldGeometry&>(pxGeometry);
					break;
				}
				case physx::PxGeometryType::eHAIRSYSTEM:
				{
					const physx::PxHairSystemGeometry& pxHairSystemGeometry = static_cast<const physx::PxHairSystemGeometry&>(pxGeometry);
					break;
				}
				case physx::PxGeometryType::eCUSTOM:
				{
					const physx::PxCustomGeometry& pxCustomGeometry = static_cast<const physx::PxCustomGeometry&>(pxGeometry);
					break;
				}
				}
			};
		physx::PxShape* pxShpaes[128] = { nullptr };
		auto drawActor = [&](physx::PxRigidActor* pxActor, float contactOffset)
			{
				const physx::PxU32 pxNumShapes = pxActor->getNbShapes();
				PX_ASSERT(pxNumShapes <= _countof(pxShpaes));
				pxActor->getShapes(pxShapes, pxNumShapes);

				physx::PxRigidDynamic* pxDynamic = pxActor->is<physx::PxRigidDynamic>();
				bool sleeping = pxDynamic ? pxDynamic->isSleeping() : false;

				for (physx::PxU32 pxShapeIndex = 0; pxShapeIndex < pxNumShapes; ++pxShapeIndex)
				{
					physx::PxShape* pxShape = pxShapes[pxShapeIndex];
					drawShape(pxShape, physx::PxShapeExt::getGlobalPose(*pxShape, *pxActor), contactOffset, sleeping);
				}
			};

		// アクター
		{
			physx::PxActorTypeFlags pxActorTypeFlags = physx::PxActorTypeFlag::eRIGID_DYNAMIC | physx::PxActorTypeFlag::eRIGID_STATIC;
			physx::PxU32 pxNumActors = pxScene->getNbActors(pxActorTypeFlags);
			if (pxNumActors > 0)
			{
				std::vector<physx::PxRigidActor*> pxActors(pxNumActors);
				pxScene->getActors(pxActorTypeFlags, reinterpret_cast<physx::PxActor**>(pxActors.data()), pxNumActors);

				for (physx::PxU32 pxActorIndex = 0; pxActorIndex < pxNumActors; ++pxActorIndex)
				{
					physx::PxRigidActor* pxActor = pxActors.at(pxActorIndex);
					// TODO: contactOffsetを考慮して描画する
					drawActor(pxActor, 0.0f);
				}
			}
		}
		// コントローラー
		{
			physx::PxU32 pxNumControllers = pxControllerManager->getNbControllers();
			if (pxNumControllers > 0)
			{
				for (physx::PxU32 pxControllerIndex = 0; pxControllerIndex < pxNumControllers; ++pxControllerIndex)
				{
					physx::PxController* pxController = pxControllerManager->getController(pxControllerIndex);
					physx::PxRigidActor* pxActor = pxController->getActor();
					drawActor(pxActor, 0.0f);
					drawActor(pxActor, pxController->getContactOffset());
				}
			}
		}
		//
		{
			physx::PxU32 pxNumArticulations = pxScene->getNbArticulations();
			if (pxNumArticulations > 0)
			{
				std::vector<physx::PxArticulationReducedCoordinate*> pxArticulations(pxNumArticulations);
				pxScene->getArticulations(reinterpret_cast<physx::PxArticulationReducedCoordinate**>(pxArticulations.data()), pxNumArticulations);

				for (physx::PxU32 pxNumArticulationIndex = 0; pxNumArticulationIndex < pxNumArticulations; ++pxNumArticulationIndex)
				{
					physx::PxArticulationReducedCoordinate* pxArticulation = pxArticulations.at(pxNumArticulationIndex);

					physx::PxU32 pxNumLinks = pxArticulation->getNbLinks();
					std::vector<physx::PxArticulationLink*> pxLinks(pxNumLinks);
					pxArticulation->getLinks(pxLinks.data(), pxNumLinks);

					bool sleeping = pxArticulation->isSleeping();
					for (physx::PxU32 pxLinkIndex = 0; pxLinkIndex < pxNumLinks; ++pxLinkIndex)
					{
						physx::PxArticulationLink* pxLink = pxLinks.at(pxLinkIndex);
						const physx::PxU32 pxNumShapes = pxLink->getNbShapes();
						PX_ASSERT(pxNumShapes <= _countof(pxShpaes));
						pxLink->getShapes(pxShapes, pxNumShapes);

						for (physx::PxU32 pxShapeIndex = 0; pxShapeIndex < pxNumShapes; ++pxShapeIndex)
						{
							physx::PxShape* pxShape = pxShapes[pxShapeIndex];
							physx::PxTransform pxShapeTransform = pxLink->getGlobalPose() * pxShape->getLocalPose();
							drawShape(pxShape, pxShapeTransform, 0.0f, sleeping);
						}
					}
				}
			}
		}
	}

	DebugRenderer::DrawAll(renderContext);
}

void Physics::DrawGUI()
{
#ifdef USE_IMGUI
	// ここで物理エンジンに関連するGUIを描画する処理を実装

	{
		ImGui::Begin("Physics");

		static bool enableRaycasts = false;
		ImGui::Checkbox("Enable Test Raycasts", &enableRaycasts);
		if (enableRaycasts)
		{
			ImGui::Text("Raycasts are enabled. Click to perform raycasts.");
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				// マウスクリック時にレイキャストを行う処理
				Vector2 rayStartScreen = InputSystem::GetMousePosition(); // マウスのスクリーン座標を取得
				Vector3 rayStart, rayDir;
				// カメラのスクリーン座標からワールド座標への変換を行う関数を呼び出して、rayStartとrayDirを計算する
				//EditorCamera::ScreenPointToRay(rayStartScreen, rayStart, rayDir);

				// シーンの取得やカメラの取得方法はプロジェクトによって異なるため、以下は一例です。実際のプロジェクトの構造に合わせて適切に修正してください。
				// Componentから取得するときは、GetOwner()->GetScene()などを経由してシーンを取得することもできます。
				SceneManager::GetLoadingSceneOrCurrentScene()->GetCameraSystem()->GetMainCamera()->ScreenPointToRay(rayStartScreen, rayStart, rayDir); // 現在のシーンのカメラシステムからスクリーン座標からレイを計算する

				Console::Log(std::format("Ray Start: ({}, {}, {}), Ray Dir: ({}, {}, {})", rayStart.x, rayStart.y, rayStart.z, rayDir.x, rayDir.y, rayDir.z));

				float rayLength = 1000.0f; // レイの長さ
				RaycastHit hitInfo;
				if (Raycast(rayStart, rayDir, rayLength, hitInfo, LayerMasks::Everything))
				{
					Console::Log(std::format("Raycast hit at position: ({}, {}, {})", hitInfo.point.x, hitInfo.point.y, hitInfo.point.z));
					Console::Log(std::format("Hit Normal: ({}, {}, {})", hitInfo.normal.x, hitInfo.normal.y, hitInfo.normal.z));
					Console::Log(std::format("Hit Distance: {}", hitInfo.distance));
					Console::Log(std::format("Hit Collider: {}", hitInfo.collider ? hitInfo.collider->GetOwner()->GetName() : "Null"));
				}
				else
				{
					Console::Log("Raycast did not hit anything.");
				}
			}
		}

		// 重力の編集
		ImGui::Separator();
		/*{
			Vector3 gravity = GetGravity();
			if (ImGui::DragFloat3("Gravity", &gravity.x, 0.1f))
			{
				SetGravity(gravity);
			}
			if (ImGui::Button("Reset Gravity"))
			{
				SetGravity({0.0f, -9.81f, 0.0f});
			}
		}*/

		// 物理マテリアルの一覧を表示
		if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
		{
			// 削除予定のマテリアルのハンドルを追跡するための変数
			MaterialHandle materialToRemove = INVALID_MATERIAL_HANDLE;
			
			// 登録されたマテリアルをループして表示
			for (auto& pair : m_materialMap)
			{
				MaterialHandle handle = pair.first;
				PhysicsMaterial& material = pair.second;
				ImGui::PushID(handle); // ユニークなIDをプッシュ

				// マテリアルエディタの描画
				{
					bool isDefaultMaterial = (handle == DEFAULT_MATERIAL_HANDLE);
					bool isEditable = !isDefaultMaterial; // デフォルトマテリアルは編集不可にする
					bool isDeletable = !isDefaultMaterial; // デフォルトマテリアルは削除不可にする

					// 編集不可な場合はGUIをグレーアウトする
					ImGui::BeginDisabled(!isEditable);

					// マテリアルのプロパティを編集するGUIを描画
					//ImGui::Text("Material Handle: %d", handle);
					
					// 名前の編集
					char nameBuffer[256];
					if (!ImGui::IsItemEdited())
					{
						strncpy_s(nameBuffer, material.data.name.c_str(), sizeof(nameBuffer));
					}
					ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer));
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						material.data.name = nameBuffer; // 名前を更新
						m_materialNameMapDirty = true; // マテリアル名マップが古くなったことを示すフラグを立てる
					}

					// 物理マテリアルのプロパティを編集するGUIを描画
					const char* format = "%.2f"; // 小数点以下2桁のフォーマット
					ImGuiSliderFlags sliderFlags = 0; // ImGuiのバージョンによっては ImGuiSliderFlags_ReadOnly が存在しないため、フラグは使用しないことにする

					bool propertyChanged = false; // プロパティが変更されたかどうかを追跡するフラグ
					propertyChanged |= ImGui::DragFloat("Static Friction", &material.data.staticFriction, 0.01f, 0.0f, 10.0f, format, sliderFlags);
					propertyChanged |= ImGui::DragFloat("Dynamic Friction", &material.data.dynamicFriction, 0.01f, 0.0f, 10.0f, format, sliderFlags);
					propertyChanged |= ImGui::DragFloat("Bounciness", &material.data.bounciness, 0.01f, 0.0f, 1.0f, format, sliderFlags);
					propertyChanged |= ImGui::Combo("Friction Combine Mode", (int*)&material.data.frictionCombineMode, "Average\0Minimum\0Multiply\0Maximum\0");
					propertyChanged |= ImGui::Combo("Bounce Combine Mode", (int*)&material.data.bounceCombineMode, "Average\0Minimum\0Multiply\0Maximum\0");


					if (propertyChanged)
					{
						// プロパティが変更された場合は、PhysXのマテリアルも更新する
						if (physx::PxMaterial* pxMaterial = material.pxMaterial)
						{
							pxMaterial->setStaticFriction(material.data.staticFriction);
							pxMaterial->setDynamicFriction(material.data.dynamicFriction);
							pxMaterial->setRestitution(material.data.bounciness);
							pxMaterial->setFrictionCombineMode((physx::PxCombineMode::Enum)material.data.frictionCombineMode);
							pxMaterial->setRestitutionCombineMode((physx::PxCombineMode::Enum)material.data.bounceCombineMode);
						}
					}

					// マテリアルの削除ボタン
					if (isDeletable)
					{
						if (ImGui::Button("Delete Material"))
						{
							// 削除予定のマテリアルのハンドルをリストに追加
							materialToRemove = handle;
						}
					}

					ImGui::EndDisabled();
				}

				ImGui::Separator(); // 区切り線
				ImGui::PopID(); // IDをポップ
			}

			// 削除予定のマテリアルをループの外で削除する
			if (materialToRemove != INVALID_MATERIAL_HANDLE)
			{
				RemoveMaterial(materialToRemove);
			}

			// マテリアルの追加ボタン
			if (ImGui::Button("Add Material"))
			{
				// マテリアルを追加する処理を実装
				PhysicsMaterialData newMaterialData{
					.name = "New Material",
					.staticFriction = DEFAULT_STATIC_FRICTION,
					.dynamicFriction = DEFAULT_DYNAMIC_FRICTION,
					.bounciness = DEFAULT_BOUNCINESS,
					.frictionCombineMode = DEFAULT_FRICTION_COMBINE_MODE,
					.bounceCombineMode = DEFAULT_BOUNCE_COMBINE_MODE
				};
				CreateAndRegisterMaterial(newMaterialData);
			}
		}

		ImGui::Separator(); // 区切り線

		// レイヤー管理のGUIを表示するボタン
		if (ImGui::Button("Open Layer Settings"))
		{
			LayerManager::Get().OpenLayerSettingsGUI();
		}

		ImGui::End();
	}


#endif // USE_IMGUI
}

void Physics::RegisterPendingRigidbody(Rigidbody* rigidbody)
{
	// Rigidbodyを保留中のリストに追加する処理を実装
	s_pendingRigidbodies.push_back(rigidbody);
}

void Physics::UnregisterPendingRigidbody(Rigidbody* rigidbody)
{
	// Rigidbodyを保留中のリストから削除する処理を実装
	s_pendingRigidbodies.erase(std::remove(s_pendingRigidbodies.begin(), s_pendingRigidbodies.end(), rigidbody), s_pendingRigidbodies.end());
}

void Physics::RegisterPendingCollider(Collider* collider)
{
	// Colliderを保留中のリストに追加する処理を実装
	s_pendingColliders.push_back(collider);
}

void Physics::UnregisterPendingCollider(Collider* collider)
{
	// Colliderを保留中のリストから削除する処理を実装
	s_pendingColliders.erase(std::remove(s_pendingColliders.begin(), s_pendingColliders.end(), collider), s_pendingColliders.end());
}

void Physics::FlushPendingRegistrations()
{
	// 保留中のRigidbodyとColliderを物理エンジンに登録する処理を実装
	// Rigidbodyの登録
	for (Rigidbody* rigidbody : s_pendingRigidbodies)
	{
		if (rigidbody != nullptr)
		{
			rigidbody->Register();
		}
	}
	// Colliderの登録
	for (Collider* collider : s_pendingColliders)
	{
		if (collider != nullptr)
		{
			collider->Register();
		}
	}
	// Colliderの登録後に呼び出す処理
	for (Rigidbody* rigidbody : s_pendingRigidbodies)
	{
		if (rigidbody != nullptr)
		{
			rigidbody->PostColliderRegister(); // Rigidbodyの登録後に呼び出す処理
		}
	}

	s_pendingRigidbodies.clear(); // 登録が完了したらリストをクリア
	s_pendingColliders.clear(); // 登録が完了したらリストをクリア
}

ActorHandle Physics::RegisterBody(Transform* transform, bool isDynamic)
{
	// ここで必要に応じてActorを生成し、コライダーを持つゲームオブジェクトを物理エンジンに登録する処理を実装
	physx::PxRigidActor* actor = nullptr;
	ActorHandle handle = GetActorHandle(transform);

	// すでにActorが存在するか確認
	if (handle != INVALID_ACTOR_HANDLE)
	{
		// すでにActorが存在する場合はそれを取得
		actor = m_actorMap[handle].actor;

		if (actor != nullptr)
		{
			// すでに動的なActorが存在する場合は新たに生成する必要はない(ただし、動的から静的への変更は考慮していない)
			if (isDynamic && actor->is<physx::PxRigidDynamic>())
			{
				return handle; // すでに動的なActorが存在する場合はそのハンドルを返す
			}

			// Static から Dynamic への変更は一度 Static Actor を削除してから Dynamic Actor を生成する必要があるため、既存の Actor を削除
			if (isDynamic && actor->is<physx::PxRigidStatic>())
			{
				// 新しい Dynamic Actor を作成
				physx::PxRigidDynamic* dynamicActor = pxPhysics->createRigidDynamic(actor->getGlobalPose());

				// 既存の Static Actor からシェイプを移行
				physx::PxU32 numShapes = actor->getNbShapes();
				std::vector<physx::PxShape*> shapes(numShapes);
				actor->getShapes(shapes.data(), numShapes);
				for (physx::PxShape* shape : shapes)
				{
					shape->acquireReference(); // 参照カウントを上げて保護
					actor->detachShape(*shape); // Static Actor からシェイプを削除
					dynamicActor->attachShape(*shape); // シェイプを Dynamic Actor に移行
					shape->release(); // 参照カウントを下げて解放
				}

				// 既存の Static Actor を削除
				GetScene()->removeActor(*actor);
				PX_RELEASE(actor); // 既存の Static Actor を削除

				// マップを更新
				ActorData data{
					.actor = dynamicActor,
					.transform = transform
				};
				m_actorMap[handle] = data; // Actor マップを更新
				GetScene()->addActor(*dynamicActor); // Dynamic Actor をシーンに追加


				return handle; // 新しい Dynamic Actor のハンドルを返す
			}
		}
	}

	// Actorが必要な場合は新規作成
	{
		// ActorHandle を新規作成
		handle = CreateActorHandle();

		// Transformからワールド位置を取得
		Vector3 worldPos = Vector3(transform->GetWorldPosition());
		Quaternion worldRot = Quaternion(transform->GetWorldRotation());
		physx::PxVec3 pxWorldPos = ToPxVec3(worldPos);
		physx::PxQuat pxWorldRot = ToPxQuat(worldRot);
		pxWorldRot.normalize(); // クォータニオンを正規化して回転の歪みを防止
		physx::PxTransform pxTransform(pxWorldPos, pxWorldRot);

		if (isDynamic)
		{
			// 動的なコライダーの場合はPxRigidDynamicを生成
			actor = pxPhysics->createRigidDynamic(pxTransform);
		}
		else
		{
			// 静的なコライダーの場合はPxRigidStaticを生成
			actor = pxPhysics->createRigidStatic(pxTransform);
		}

		_ASSERT_EXPR(actor != nullptr, "Failed to create actor!");

		// シーンにActorを追加
		if (GetScene()->addActor(*actor))
		{
			// TODO: ActorのuserDataに入れるデータが変更されたらここも変更する必要がある
			actor->userData = transform; // ActorのユーザーデータにTransformを設定

			// Actorをマップに保存
			ActorData data{
				.actor = actor,
				.transform = transform
			};
			m_actorMap[handle] = data;

			return handle; // 新しく作成したActorのハンドルを返す
		}
	}

	return INVALID_ACTOR_HANDLE; // 登録に失敗した場合はINVALID_ACTOR_HANDLEを返す
}

void Physics::UnregisterBody(Transform* transform)
{
	// ここでコライダーを持つゲームオブジェクトを物理エンジンから登録解除する処理を実装

	// Transform* に対応する ActorHandle を取得
	ActorHandle handle = GetActorHandle(transform);
	if (handle != INVALID_ACTOR_HANDLE)
	{
		// ActorHandle に対応する Actor を取得
		if (physx::PxRigidActor* actor = GetActor(handle))
		{
			physx::PxU32 numShapes = actor->getNbShapes();
			// Actorにシェイプが存在する場合はStatic Actorに移行
			if (numShapes > 0)
			{
				// 新しい Static Actor を作成
				physx::PxTransform currentPose = actor->getGlobalPose();
				physx::PxRigidStatic* staticActor = pxPhysics->createRigidStatic(currentPose);

				// 既存の Actor からシェイプを移行
				std::vector<physx::PxShape*> shapes(numShapes);
				actor->getShapes(shapes.data(), numShapes);

				for (physx::PxShape* shape : shapes)
				{
					shape->acquireReference(); // 参照カウントを上げて保護
					actor->detachShape(*shape); // 既存の Actor からシェイプを削除
					staticActor->attachShape(*shape); // シェイプを Static Actor に移行
					shape->release(); // 参照カウントを下げて解放
				}

				// ハンドルのマップを更新
				ActorData data{
					.actor = staticActor,
					.transform = transform
				};
				m_actorMap[handle] = data; // Actor マップを更新

				// Static Actor をシーンに追加
				staticActor->userData = transform; // Static ActorのユーザーデータにTransformを設定
				GetScene()->addActor(*staticActor);
				
				// シーンからActorを削除
				actor->userData = nullptr;
				GetScene()->removeActor(*actor);
				PX_RELEASE(actor); // 既存の Actor を削除
			}
			else
			{
				// Actorにシェイプが存在しない場合はそのまま削除
				RemoveActor(handle);
			}
		}
	}
}

void Physics::OnTrnasformDestroyed(Transform* transform)
{
	// Transformが破棄されたときに呼び出されるコールバック関数
	// Transform* に対応する ActorHandle を取得
	ActorHandle handle = GetActorHandle(transform);
	if (handle != INVALID_ACTOR_HANDLE)
	{
		// ActorHandle に対応する Actor を取得
		if (physx::PxRigidActor* actor = GetActor(handle))
		{
			RemoveActor(handle);
		}
	}
}

bool Physics::AddBoxShape(Transform* transform, const BoxColliderData& data, ShapeHandle& outHandle)
{
	// ここでBoxColliderを持つゲームオブジェクトにBoxColliderを追加し、物理エンジンに登録する処理を実装

	// Transform* に対応する ActorHandle を取得
	ActorHandle handle = GetActorHandle(transform);

	// Actorが存在しない場合は新規作成
	if (handle == INVALID_ACTOR_HANDLE)
	{
		// Actorが存在しない場合は新規作成
		handle = RegisterBody(transform, false); // 静的なActorを作成
	}
	
	// Actorが存在する場合はBoxColliderを追加
	if (physx::PxRigidActor* actor = GetActor(handle))
	{
		// BoxColliderのサイズを設定してPxBoxGeometryを生成
		physx::PxBoxGeometry boxGeometry(data.halfExtents.x, data.halfExtents.y, data.halfExtents.z);
		physx::PxMaterial* pxMaterial = GetMaterial(data.materialHandle);
		physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, boxGeometry, *pxMaterial);
		_ASSERT_EXPR(shape != nullptr, "Failed to create shape!");

		// 追加されたBoxColliderのShapeHandleを生成(AddShape関数内でシーンに追加も行われる)
		ShapeHandle shapeHandle = CreateShapeHandle();
		_ASSERT_EXPR(shapeHandle != INVALID_SHAPE_HANDLE, "Failed to AddShape!");
		RegisterShape(shapeHandle, shape); // ShapeHandleとPxShape*のマッピングを保存

		// BoxColliderのローカル位置を設定
		SetLocalPose(shapeHandle, data.center, Quaternion(0,0,0,1));

		// トリガーかどうかを設定
		SetTrigger(shapeHandle, data.isTrigger);

		// 接触オフセットを設定
		SetContactOffset(shapeHandle, data.contactOffset);

		// TODO: userDataに入れるデータが変更されたらここも変更する必要がある
		shape->userData = data.collider; // シェイプのユーザーデータにColliderを設定

		// 追加されたBoxColliderのShapeHandleを返す
		outHandle = shapeHandle;
		return true; // 追加に成功した場合はtrueを返す
	}

	outHandle = INVALID_SHAPE_HANDLE; // 追加に失敗した場合はINVALID_SHAPE_HANDLEを返す
	return false; // 追加に失敗した場合はfalseを返す
}

bool Physics::AddSphereShape(Transform* transform, const SphereColliderData& data, ShapeHandle& outHandle)
{
	// ここでSphereColliderを持つゲームオブジェクトにSphereColliderを追加し、物理エンジンに登録する処理を実装
	// Transform* に対応する ActorHandle を取得
	ActorHandle handle = GetActorHandle(transform);
	// Actorが存在しない場合は新規作成
	if (handle == INVALID_ACTOR_HANDLE)
	{
		// Actorが存在しない場合は新規作成
		handle = RegisterBody(transform, false); // 静的なActorを作成
	}
	// Actorが存在する場合はSphereColliderを追加
	if (physx::PxRigidActor* actor = GetActor(handle))
	{
		// SphereColliderの半径を設定してPxSphereGeometryを生成
		physx::PxSphereGeometry sphereGeometry(data.radius);
		physx::PxMaterial* pxMaterial = GetMaterial(data.materialHandle);
		physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, sphereGeometry, *pxMaterial);
		_ASSERT_EXPR(shape != nullptr, "Failed to create shape!");
		
		// 追加されたSphereColliderのShapeHandleを生成(AddShape関数内でシーンに追加も行われる)
		ShapeHandle shapeHandle = CreateShapeHandle();
		_ASSERT_EXPR(shapeHandle != INVALID_SHAPE_HANDLE, "Failed to AddShape!");
		RegisterShape(shapeHandle, shape); // ShapeHandleとPxShape*のマッピングを保存

		// SphereColliderのローカル位置を設定
		SetLocalPose(shapeHandle, data.center, Quaternion(0,0,0,1));

		// トリガーかどうかを設定
		SetTrigger(shapeHandle, data.isTrigger);

		// 接触オフセットを設定
		SetContactOffset(shapeHandle, data.contactOffset);

		// TODO: userDataに入れるデータが変更されたらここも変更する必要がある
		shape->userData = data.collider; // シェイプのユーザーデータにColliderを設定

		outHandle = shapeHandle; // 追加されたSphereColliderのShapeHandleを返す
		return true; // 追加に成功した場合はtrueを返す
	}

	outHandle = INVALID_SHAPE_HANDLE; // 追加に失敗した場合はINVALID_SHAPE_HANDLEを返す
	return false; // 追加に失敗した場合はfalseを返す
}

bool Physics::AddCapsuleShape(Transform* transform, const CapsuleColliderData& data, ShapeHandle& outHandle)
{
	// ここでCapsuleColliderを持つゲームオブジェクトにCapsuleColliderを追加し、物理エンジンに登録する処理を実装
	// Transform* に対応する ActorHandle を取得
	ActorHandle handle = GetActorHandle(transform);
	// Actorが存在しない場合は新規作成
	if (handle == INVALID_ACTOR_HANDLE)
	{
		// Actorが存在しない場合は新規作成
		handle = RegisterBody(transform, false); // 静的なActorを作成
	}
	// Actorが存在する場合はCapsuleColliderを追加
	if (physx::PxRigidActor* actor = GetActor(handle))
	{
		// CapsuleColliderの半径と高さを設定してPxCapsuleGeometryを生成
		physx::PxCapsuleGeometry capsuleGeometry(data.radius, data.height * 0.5f); // PhysXのカプセルは半分の高さを指定する必要がある
		physx::PxMaterial* pxMaterial = GetMaterial(data.materialHandle);
		physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, capsuleGeometry, *pxMaterial);
		_ASSERT_EXPR(shape != nullptr, "Failed to create shape!");

		// 追加されたCapsuleColliderのShapeHandleを生成(AddShape関数内でシーンに追加も行われる)
		ShapeHandle shapeHandle = CreateShapeHandle();
		_ASSERT_EXPR(shapeHandle != INVALID_SHAPE_HANDLE, "Failed to AddShape!");
		RegisterShape(shapeHandle, shape); // ShapeHandleとPxShape*のマッピングを保存
		// CapsuleColliderのローカル位置を設定
		Quaternion localRot = Quaternion::FromEuler({ 0.0f, 0.0f, 90.0f }); // カプセルのローカル回転を設定（Z軸回転で立てる）
		SetLocalPose(shapeHandle, data.center, localRot);
		// トリガーかどうかを設定
		SetTrigger(shapeHandle, data.isTrigger);

		// 接触オフセットを設定
		SetContactOffset(shapeHandle, data.contactOffset);

		// TODO: userDataに入れるデータが変更されたらここも変更する必要がある
		shape->userData = data.collider; // シェイプのユーザーデータにColliderを設定

		// 追加されたCapsuleColliderのShapeHandleを返す
		outHandle = shapeHandle;
		return true; // 追加に成功した場合はtrueを返す
	}

	outHandle = INVALID_SHAPE_HANDLE; // 追加に失敗した場合はINVALID_SHAPE_HANDLEを返す
	return false; // 追加に失敗した場合はfalseを返す
}

bool Physics::AddTriangleMeshShape(Transform* transform, const MeshColliderData& data, ShapeHandle& outHandle)
{
	// ここでTriangleMeshColliderを持つゲームオブジェクトにTriangleMeshColliderを追加し、物理エンジンに登録する処理を実装
	// Transform* に対応する ActorHandle を取得
	ActorHandle handle = GetActorHandle(transform);
	// Actorが存在しない場合は新規作成
	if (handle == INVALID_ACTOR_HANDLE)
	{
		// Actorが存在しない場合は新規作成
		handle = RegisterBody(transform, false); // 静的なActorを作成
	}
	// Actorが存在する場合はTriangleMeshColliderを追加
	if (physx::PxRigidActor* actor = GetActor(handle))
	{
		// TriangleMeshColliderの頂点とインデックスを設定してPxTriangleMeshGeometryを生成
		std::vector<physx::PxVec3> vertices(data.vertices.size());
		for (size_t i = 0; i < data.vertices.size(); ++i)
		{
			//vertices[i] = reinterpret_cast<const physx::PxVec3&>(data.vertices[i]);
			vertices[i] = ToPxVec3(data.vertices[i]);
		}
		std::vector<physx::PxU32> indices(data.indices.size());
		for (size_t i = 0; i < data.indices.size(); ++i)
		{
			indices[i] = static_cast<physx::PxU32>(data.indices[i]);
		}

		physx::PxTriangleMeshDesc meshDesc;
		meshDesc.points.count = static_cast<physx::PxU32>(vertices.size());
		meshDesc.points.stride = sizeof(physx::PxVec3);
		meshDesc.points.data = vertices.data();
		meshDesc.triangles.count = static_cast<physx::PxU32>(indices.size() / 3);
		meshDesc.triangles.stride = sizeof(physx::PxU32) * 3;
		meshDesc.triangles.data = indices.data();
		_ASSERT_EXPR(meshDesc.isValid(), L"PxTriangleMeshDesc is invalid!");

		physx::PxCookingParams cookingParams(pxPhysics->getTolerancesScale());
		cookingParams.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eFORCE_32BIT_INDICES;
		physx::PxTriangleMesh* triangleMesh = PxCreateTriangleMesh(cookingParams, meshDesc);
		_ASSERT_EXPR(triangleMesh != nullptr, L"PxCreateTriangleMesh failed!");

		//physx::PxMeshScale meshScale(ToPxVec3(transform->GetWorldScale())); // メッシュのスケールを設定
		Vector3 worldScale = transform->GetWorldScale();

		_ASSERT_EXPR(std::isfinite(worldScale.x), "scale.x invalid!");
		_ASSERT_EXPR(std::isfinite(worldScale.y), "scale.y invalid!");
		_ASSERT_EXPR(std::isfinite(worldScale.z), "scale.z invalid!");

		_ASSERT_EXPR(worldScale.x > 0.0001f, L"scale x <= 0");
		_ASSERT_EXPR(worldScale.y > 0.0001f, L"scale y <= 0");
		_ASSERT_EXPR(worldScale.z > 0.0001f, L"scale z <= 0");

		physx::PxMeshScale meshScale(ToPxVec3(worldScale)); // メッシュのスケールを設定
		physx::PxTriangleMeshGeometry triangleMeshGeometry(triangleMesh, meshScale);
		physx::PxMaterial* pxMaterial = GetMaterial(data.materialHandle);
		physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, triangleMeshGeometry, *pxMaterial);
		_ASSERT_EXPR(shape != nullptr, L"Failed to create shape!");

		triangleMesh->release(); // TriangleMeshはShapeにコピーされるため、TriangleMeshは解放しても問題ない

		// 追加されたTriangleMeshColliderのShapeHandleを生成(AddShape関数内でシーンに追加も行われる)
		ShapeHandle shapeHandle = CreateShapeHandle();
		_ASSERT_EXPR(shapeHandle != INVALID_SHAPE_HANDLE, L"Failed to AddShape!");
		RegisterShape(shapeHandle, shape); // ShapeHandleとPxShape*のマッピングを保存
		// TriangleMeshColliderのローカル位置を設定
		//SetLocalPose(shapeHandle, data.center, Quaternion(0, 0, 0, 1));
		// トリガーかどうかを設定
		SetTrigger(shapeHandle, data.isTrigger);

		// 接触オフセットを設定
		SetContactOffset(shapeHandle, data.contactOffset);

		// TODO: userDataに入れるデータが変更されたらここも変更する必要がある
		shape->userData = data.collider; // シェイプのユーザーデータにColliderを設定
		// 追加されたTriangleMeshColliderのShapeHandleを返す
		outHandle = shapeHandle;
		return true; // 追加に成功した場合はtrueを返す
	}

	outHandle = INVALID_SHAPE_HANDLE; // 追加に失敗した場合はINVALID_SHAPE_HANDLEを返す
	return false; // 追加に失敗した場合はfalseを返す
}

bool Physics::AddConvexMeshShape(Transform* transform, const MeshColliderData& data, ShapeHandle& outHandle)
{
	// ここでConvexMeshColliderを持つゲームオブジェクトにConvexMeshColliderを追加し、物理エンジンに登録する処理
	// Transform* に対応する ActorHandle を取得
	ActorHandle handle = GetActorHandle(transform);
	// Actorが存在しない場合は新規作成
	if (handle == INVALID_ACTOR_HANDLE)
	{
		// Actorが存在しない場合は新規作成
		handle = RegisterBody(transform, false); // 静的なActorを作成
	}
	// Actorが存在する場合はConvexMeshColliderを追加
	if (physx::PxRigidActor* actor = GetActor(handle))
	{
		// ConvexMeshColliderの頂点を設定してPxConvexMeshGeometryを生成
		std::vector<physx::PxVec3> vertices(data.vertices.size());
		for (size_t i = 0; i < data.vertices.size(); ++i)
		{
			vertices[i] = ToPxVec3(data.vertices[i]);
			//vertices[i] = reinterpret_cast<const physx::PxVec3&>(data.vertices[i]);
		}
		physx::PxConvexMeshDesc meshDesc;
		meshDesc.points.count = static_cast<physx::PxU32>(vertices.size());
		meshDesc.points.stride = sizeof(physx::PxVec3);
		meshDesc.points.data = vertices.data();
		meshDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX; // ConvexMeshを自動生成するためのフラグを設定
		_ASSERT_EXPR(meshDesc.isValid(), L"PxConvexMeshDesc is invalid!");

		physx::PxCookingParams cookingParams(pxPhysics->getTolerancesScale());
		cookingParams.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eFORCE_32BIT_INDICES;
		physx::PxConvexMesh* convexMesh = PxCreateConvexMesh(cookingParams, meshDesc);
		_ASSERT_EXPR(convexMesh != nullptr, L"PxCreateConvexMesh failed!");

		Vector3 worldScale = transform->GetWorldScale();

		_ASSERT_EXPR(std::isfinite(worldScale.x), "scale.x invalid!");
		_ASSERT_EXPR(std::isfinite(worldScale.y), "scale.y invalid!");
		_ASSERT_EXPR(std::isfinite(worldScale.z), "scale.z invalid!");

		_ASSERT_EXPR(worldScale.x > 0.0001f, L"scale x <= 0");
		_ASSERT_EXPR(worldScale.y > 0.0001f, L"scale y <= 0");
		_ASSERT_EXPR(worldScale.z > 0.0001f, L"scale z <= 0");

		physx::PxMeshScale meshScale(ToPxVec3(worldScale)); // メッシュのスケールを設定
		physx::PxConvexMeshGeometry convexMeshGeometry(convexMesh, meshScale);
		physx::PxMaterial* pxMaterial = GetMaterial(data.materialHandle);
		physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, convexMeshGeometry, *pxMaterial);
		_ASSERT_EXPR(shape != nullptr, L"Failed to create shape!");

		convexMesh->release(); // ConvexMeshはShapeにコピーされるため、ConvexMeshは解放しても問題ない

		// 追加されたConvexMeshColliderのShapeHandleを生成(AddShape関数内でシーンに追加も行われる)
		ShapeHandle shapeHandle = CreateShapeHandle();
		_ASSERT_EXPR(shapeHandle != INVALID_SHAPE_HANDLE, L"Failed to AddShape!");
		RegisterShape(shapeHandle, shape); // ShapeHandleとPxShape*のマッピングを保存
		// ConvexMeshColliderのローカル位置を設定
		//SetLocalPose(shapeHandle, data.center, Quaternion(0, 0, 0, 1));
		// トリガーかどうかを設定
		SetTrigger(shapeHandle, data.isTrigger);

		// 接触オフセットを設定
		SetContactOffset(shapeHandle, data.contactOffset);

		// TODO: userDataに入れるデータが変更されたらここも変更する必要がある
		shape->userData = data.collider; // シェイプのユーザーデータにColliderを設定
		// 追加されたConvexMeshColliderのShapeHandleを返す
		outHandle = shapeHandle;
		return true; // 追加に成功した場合はtrueを返す
	}

	outHandle = INVALID_SHAPE_HANDLE; // 追加に失敗した場合はINVALID_SHAPE_HANDLEを返す
	return false; // 追加に失敗した場合はfalseを返す
}

// --- 形状 (Collider) の情報取得 ---

// --- 形状 (Collider) の情報設定 ---


// --- シーンの設定 ---

void Physics::SetGravity(const Vector3& gravity)
{
	// ここでシーンの重力を設定する処理を実装
	GetScene()->setGravity(ToPxVec3(gravity)); // PhysXのシーンに重力を設定
}

Vector3 Physics::GetGravity()
{
	// ここでシーンの重力を取得する処理を実装
	physx::PxVec3 gravity = GetScene()->getGravity(); // PhysXのシーンから重力を取得
	return ToVector3(gravity); // Vector3に変換して返す
}

// --- レイキャスト ---

bool Physics::Raycast(const Vector3& origin, const Vector3& direction, float maxDistance, RaycastHit& outHitInfo, LayerMask layerMask)
{
	// ここでレイキャストを実行し、衝突したオブジェクトの情報を取得する処理を実装
	physx::PxQueryFilterData filterData(physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::ePREFILTER | physx::PxQueryFlag::ePOSTFILTER); // クエリフィルタデータを設定（静的と動的の両方をクエリする）
	filterData.data.word0 = layerMask; // クエリフィルタデータのword0にレイヤーマスクを設定

	physx::PxVec3 pxOrigin = ToPxVec3(origin);
	physx::PxVec3 pxDirection = ToPxVec3(direction);
	physx::PxRaycastBufferN<1> hitInfo;
	bool hit = GetScene()->raycast(pxOrigin, pxDirection, maxDistance, hitInfo, physx::PxHitFlag::eDEFAULT, filterData, &m_filterShader);
	if (hit && hitInfo.hasBlock) // レイが何かに衝突した場合は、衝突情報をoutHitInfoに設定
	{
		outHitInfo.point = ToVector3(hitInfo.block.position);
		outHitInfo.normal = ToVector3(hitInfo.block.normal);
		outHitInfo.distance = hitInfo.block.distance;
		outHitInfo.collider = reinterpret_cast<Collider*>(hitInfo.block.shape->userData); // シェイプのユーザーデータからColliderを取得
	}
	return hit; // レイが何かに衝突した場合はtrue、そうでない場合はfalseを返す
}


// 形状のレイヤーとレイヤーマスクの設定
void Physics::SetLayer(const ShapeHandle& shapeHandle, Layer layer)
{
	// ここで形状のレイヤーを設定する処理を実装
	if (physx::PxShape* shape = GetShape(shapeHandle))
	{
		physx::PxFilterData filterData = shape->getSimulationFilterData();
		filterData.word0 = static_cast<physx::PxU32>(ToMask(layer)); // レイヤーをフィルタデータのword0に設定
		//filterData.word1 = LayerManager::Get().GetCollisionMask(layer); // レイヤーの衝突マスクをフィルタデータのword1に設定
		shape->setSimulationFilterData(filterData); // フィルタデータをシェイプに設定
		shape->setQueryFilterData(filterData); // クエリフィルタデータも同じに設定(Raycastにもレイヤーを適用するため)
	}
}

void Physics::SetLayerMask(const ShapeHandle& shapeHandle, LayerMask layerMask)
{
	// ここで形状のレイヤーマスクを設定する処理を実装
	if (physx::PxShape* shape = GetShape(shapeHandle))
	{
		physx::PxFilterData filterData = shape->getSimulationFilterData();
		filterData.word1 = layerMask; // レイヤーマスクをフィルタデータのword1に設定
		shape->setSimulationFilterData(filterData); // フィルタデータをシェイプに設定
		shape->setQueryFilterData(filterData); // クエリフィルタデータも同じに設定(Raycastにもレイヤーマスクを適用するため)
	}
}

void Physics::UpdateFilterData(const ShapeHandle& shapeHandle, Layer layer, LayerMask layerMask)
{
	// ここで形状のフィルタデータを更新する処理を実装
	if (physx::PxShape* shape = GetShape(shapeHandle))
	{
		physx::PxFilterData filterData;
		filterData.word0 = static_cast<physx::PxU32>(ToMask(layer)); // レイヤーをフィルタデータのword0に設定
		filterData.word1 = layerMask; // レイヤーマスクをフィルタデータのword1に設定

		shape->setSimulationFilterData(filterData); // フィルタデータをシェイプに設定
		shape->setQueryFilterData(filterData); // クエリフィルタデータも同じに設定(Raycastにもレイヤーとレイヤーマスクを適用するため)
	}
}

LayerMask Physics::GetCollisionMask(Layer layer)
{
	// レイヤーの衝突マスクを取得する
	return LayerManager::Get().GetCollisionMask(layer); // LayerManagerを通じてレイヤーの衝突マスクを取得して返す
}

bool Physics::GetIgnoreLayerCollision(Layer layer1, Layer layer2)
{
	// レイヤー同士の衝突を無視するかどうかを取得する
	return !LayerManager::Get().GetLayerCollision(layer1, layer2); // LayerManagerを通じてレイヤーの衝突設定を取得し、無視するかどうかを返す
}

void Physics::SetIgnoreLayerCollision(Layer layer1, Layer layer2, bool ignore)
{
	// レイヤー同士の衝突を無視するかどうかを設定する
	LayerManager::Get().SetLayerCollision(layer1, layer2, !ignore); // LayerManagerを通じてレイヤーの衝突設定を更新
}

// --- 物理マテリアルの追加 ---

PhysicsMaterial Physics::CreateAndRegisterMaterial(const PhysicsMaterialData& data, MaterialHandle overrideHandle)
{
	// ここで物理マテリアルを追加し、物理エンジンに登録する処理を実装
	PhysicsMaterial newMaterial;
	newMaterial.handle = overrideHandle != INVALID_MATERIAL_HANDLE ? overrideHandle : CreateMaterialHandle(); // ハンドルを生成
	newMaterial.data = data;
	newMaterial.pxMaterial = pxPhysics->createMaterial(data.staticFriction, data.dynamicFriction, data.bounciness);
	newMaterial.pxMaterial->setFrictionCombineMode(static_cast<physx::PxCombineMode::Enum>(data.frictionCombineMode));
	newMaterial.pxMaterial->setRestitutionCombineMode(static_cast<physx::PxCombineMode::Enum>(data.bounceCombineMode));
	_ASSERT_EXPR(newMaterial.pxMaterial != nullptr, "Failed to create material!");
	// マテリアルマップに保存
	m_materialMap[newMaterial.handle] = newMaterial;
	m_materialNameMapDirty = true; // マテリアルの名前とハンドルのマッピングが最新でないことを示すフラグを立てる
	return newMaterial; // 作成された物理マテリアルを返す
}

// --- 物理マテリアルの削除 ---

void Physics::RemoveMaterial(MaterialHandle handle)
{
	// ここで物理マテリアルを削除し、物理エンジンから登録解除する処理を実装
	if (m_materialMap.contains(handle))
	{
		physx::PxMaterial* material = m_materialMap[handle].pxMaterial;
		PX_RELEASE(material); // マテリアルを解放
		m_materialMap.erase(handle); // マテリアルマップから削除
		m_materialNameMapDirty = true; // マテリアルの名前とハンドルのマッピングが最新でないことを示すフラグを立てる
	}
}

// --- 物理マテリアルの情報取得 ---

bool Physics::GetMaterialData(MaterialHandle handle, PhysicsMaterialData& outMaterial)
{
	// ここで物理マテリアルデータを取得する処理を実装
	if (m_materialMap.contains(handle))
	{
		// 取得した物理マテリアルデータをoutMaterialにコピー
		outMaterial = m_materialMap[handle].data;
		return true; // 取得に成功
	}
	return false; // 取得に失敗
}

// --- 物理マテリアルの情報設定 ---

bool Physics::SetMaterialData(MaterialHandle handle, const PhysicsMaterialData& material)
{
	// ここで物理マテリアルの特性を設定する処理を実装
	if (m_materialMap.contains(handle))
	{
		// マテリアルデータを更新
		PhysicsMaterialData& materialData = m_materialMap[handle].data;
		materialData.staticFriction = material.staticFriction;
		materialData.dynamicFriction = material.dynamicFriction;
		materialData.bounciness = material.bounciness;
		materialData.frictionCombineMode = material.frictionCombineMode;
		materialData.bounceCombineMode = material.bounceCombineMode;
		// PhysXのマテリアルも更新
		physx::PxMaterial* pxMat = m_materialMap[handle].pxMaterial;
		pxMat->setStaticFriction(material.staticFriction);
		pxMat->setDynamicFriction(material.dynamicFriction);
		pxMat->setRestitution(material.bounciness);
		pxMat->setFrictionCombineMode(static_cast<physx::PxCombineMode::Enum>(material.frictionCombineMode));
		pxMat->setRestitutionCombineMode(static_cast<physx::PxCombineMode::Enum>(material.bounceCombineMode));
		return true; // 設定に成功
	}
	return false; // 設定に失敗
}

// --- 物理マテリアルマップ関連 ---

void Physics::UpdateMaterialNameMap()
{
	// ここで物理マテリアルの名前とハンドルのマッピングを更新する処理を実装

	// マテリアルの名前とハンドルのマッピングが最新でない場合にのみ更新を行う
	if (m_materialNameMapDirty)
	{
		m_materialNameMap.clear(); // 既存のマッピングをクリア
		for (const auto& pair : m_materialMap)
		{
			MaterialHandle handle = pair.first;
			const std::string& name = pair.second.data.name;
			// マテリアルの名前とハンドルのペアをマッピングに追加
			m_materialNameMap.emplace_back(std::make_pair(name, handle));
		}
		m_materialNameMapDirty = false; // マッピングが最新になったのでフラグをリセット
	}
}

bool Physics::GetMaterialHandleByName(const std::string& name, MaterialHandle& outHandle)
{
	// ここで物理マテリアルの名前からハンドルを取得する処理を実装
	// マテリアルの名前とハンドルのマッピングが最新でない場合は更新する
	UpdateMaterialNameMap();

	for (const auto& pair : m_materialNameMap)
	{
		if (pair.first == name)
		{
			outHandle = pair.second; // 名前に対応するハンドルをoutHandleに設定
			return true; // 取得に成功
		}
	}
	outHandle = INVALID_MATERIAL_HANDLE; // 取得に失敗した場合はINVALID_MATERIAL_HANDLEを返す
	return false; // 取得に失敗
}

void Physics::GetAllMaterialNamesAndHandles(std::vector<const char*>& outNames, std::vector<MaterialHandle>& outHandles)
{
	// ここで物理マテリアルの名前とハンドルの一覧を取得する処理を実装
	// マテリアルの名前とハンドルのマッピングが最新でない場合は更新する
	UpdateMaterialNameMap();
	outNames.clear();
	outHandles.clear();
	for (const auto& pair : m_materialNameMap)
	{
		outNames.push_back(pair.first.c_str()); // 名前をoutNamesに追加
		outHandles.push_back(pair.second); // ハンドルをoutHandlesに追加
	}
}

// --- 物理操作 (Rigidbody) の窓口 ---

void Physics::SetMass(const ActorHandle& handle, float mass)
{
	// ここでRigidbodyを持つゲームオブジェクトの質量を設定する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		if (std::isinf(mass) || std::isnan(mass))
		{
			mass = 0.0001f; // 質量が無限大またはNaNの場合は最小値にクランプして設定する
			Console::LogWarning("Invalid mass value! Mass must be finite and non-NaN. Mass has been clamped to 0.0001.");
		}
		if (dynamicActor->getActorFlags() & physx::PxActorFlag::eDISABLE_SIMULATION)
		{
			// シミュレーションが無効なActorの場合は質量を設定しても意味がないため、質量を設定せずに終了する
			Console::LogWarning("Attempted to set mass on an actor with simulation disabled! Mass will not be set.");
			return;
		}
		if (dynamicActor->getRigidBodyFlags() & physx::PxRigidBodyFlag::eKINEMATIC)
		{
			// キネマティックなActorの場合は質量を設定しても意味がないため、質量を設定せずに終了する
			Console::LogWarning("Attempted to set mass on a kinematic actor! Mass will not be set.");
			return;
		}

		// 質量を設定して慣性を更新する
		{
			//dynamicActor->setMass(mass);
			physx::PxRigidBodyExt::setMassAndUpdateInertia(*dynamicActor, mass); // 質量を設定して慣性を更新
		}
	}
}

void Physics::SetInertiaTensor(const ActorHandle& handle, const Vector3& inertiaTensor)
{
	// ここでRigidbodyを持つゲームオブジェクトの慣性テンソルを設定する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		if (std::isinf(inertiaTensor.x) || std::isnan(inertiaTensor.x) ||
			std::isinf(inertiaTensor.y) || std::isnan(inertiaTensor.y) ||
			std::isinf(inertiaTensor.z) || std::isnan(inertiaTensor.z))
		{
			// 慣性テンソルのいずれかの成分が無限大またはNaNの場合は、慣性テンソルを設定せずに終了する
			Console::LogError("Invalid inertia tensor value! Inertia tensor components must be finite and non-NaN. Inertia tensor will not be set.");
			return;
		}

		dynamicActor->setMassSpaceInertiaTensor(ToPxVec3(inertiaTensor));
	}
}

void Physics::WakeUp(const ActorHandle& handle)
{
	// ここでRigidbodyを持つゲームオブジェクトを起こす処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		dynamicActor->wakeUp();
	}
}

void Physics::PutToSleep(const ActorHandle& handle)
{
	// ここでRigidbodyを持つゲームオブジェクトを休ませる処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		dynamicActor->putToSleep();
	}
}

bool Physics::IsSleeping(const ActorHandle& handle)
{
	// ここでRigidbodyを持つゲームオブジェクトが休止状態かどうかを取得する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		return dynamicActor->isSleeping();
	}
	return false; // Actorが存在しない場合はfalseを返す
}

void Physics::SetMaterial(const ShapeHandle& shapeHandle, const MaterialHandle& materialHandle)
{
	// ここでRigidbodyを持つゲームオブジェクトの物理マテリアルを設定する処理を実装
	if (physx::PxShape* shape = GetShape(shapeHandle))
	{
		shape->setMaterials(&m_materialMap[materialHandle].pxMaterial, 1); // 新しい物理マテリアルを設定
	}
}

void Physics::SetTrigger(const ShapeHandle& shapeHandle, bool isTrigger)
{
	// ここでRigidbodyを持つゲームオブジェクトのシェイプをトリガーに設定する処理を実装
	if (physx::PxShape* shape = GetShape(shapeHandle))
	{
		// 一旦シミュレーションシェイプフラグとトリガーシェイプフラグを両方ともリセットしてから、isTriggerの値に応じてフラグを設定する
		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false); // シミュレーションシェイプフラグをリセット
		shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false); // トリガーシェイプフラグをリセット	

		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger); // シミュレーションシェイプフラグを設定
		shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger); // トリガーシェイプフラグを設定
	}
}

void Physics::SetContactOffset(const ShapeHandle& shapeHandle, float offset)
{
	// ここでRigidbodyを持つゲームオブジェクトのシェイプの接触オフセットを設定する処理を実装
	if (physx::PxShape* shape = GetShape(shapeHandle))
	{
		shape->setContactOffset(offset);
		// TODO: Physxの仕様を確認して、接触オフセットを変更したときに休止オフセットも適切に更新する必要があるかどうかを判断する必要がある
		if (offset < shape->getRestOffset())
		{
			shape->setRestOffset(offset); // 接触オフセットが現在の休止オフセットより小さい場合は、休止オフセットも更新する
		}
	}
}

void Physics::SetUseGravity(const ActorHandle& handle, bool useGravity)
{
	// ここでRigidbodyを持つゲームオブジェクトの重力の影響を設定する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !useGravity);
	}
}

void Physics::SetUseCCD(const ActorHandle& handle, bool useCCD)
{
	// ここでRigidbodyを持つゲームオブジェクトの連続的な衝突検知の使用を設定する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, useCCD);
	}
}

void Physics::SetConstraints(const ActorHandle& handle, physx::PxRigidDynamicLockFlags constraints)
{
	// ここでRigidbodyを持つゲームオブジェクトのキネマティック状態を設定する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		dynamicActor->setRigidDynamicLockFlags(constraints);
	}
}

physx::PxRigidDynamicLockFlags Physics::GetConstraints(const ActorHandle& handle)
{
	// ここでRigidbodyを持つゲームオブジェクトのキネマティック状態を取得する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		return dynamicActor->getRigidDynamicLockFlags();
	}
	return physx::PxRigidDynamicLockFlags(0); // Actorが存在しない場合は制約なしを返す
}

void Physics::SetLinearDamping(const ActorHandle& handle, float linearDamping)
{
	// ここでRigidbodyを持つゲームオブジェクトの線形減衰を設定する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		dynamicActor->setLinearDamping(linearDamping);
	}
}

float Physics::GetLinearDamping(const ActorHandle& handle)
{
	// ここでRigidbodyを持つゲームオブジェクトの線形減衰を取得する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		return dynamicActor->getLinearDamping();
	}
	return 0.0f; // Actorが存在しない場合は0を返す
}

void Physics::SetLinearDrag(const ActorHandle& handle, float linearDrag)
{
	// ここでRigidbodyを持つゲームオブジェクトの線形抵抗を設定する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		dynamicActor->setLinearDamping(linearDrag); // PhysXは線形抵抗を線形減衰として扱うため、setLinearDampingを呼び出す
	}
}

float Physics::GetLinearDrag(const ActorHandle& handle)
{
	// ここでRigidbodyを持つゲームオブジェクトの線形抵抗を取得する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		return dynamicActor->getLinearDamping(); // PhysXは線形抵抗を線形減衰として扱うため、getLinearDampingを呼び出す
	}
	return 0.0f; // Actorが存在しない場合は0を返す
}

void Physics::SetMaxLinearVelocity(const ActorHandle& handle, float maxLinearVelocity)
{
	// ここでRigidbodyを持つゲームオブジェクトの最大線形速度を設定する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		dynamicActor->setMaxLinearVelocity(maxLinearVelocity);
	}
}

float Physics::GetMaxLinearVelocity(const ActorHandle& handle)
{
	// ここでRigidbodyを持つゲームオブジェクトの最大線形速度を取得する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		return dynamicActor->getMaxLinearVelocity();
	}
	return 0.0f; // Actorが存在しない場合は0を返す
}

void Physics::SetAngularDamping(const ActorHandle& handle, float angularDamping)
{
	// ここでRigidbodyを持つゲームオブジェクトの角減衰を設定する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		dynamicActor->setAngularDamping(angularDamping);
	}
}

float Physics::GetAngularDamping(const ActorHandle& handle)
{
	// ここでRigidbodyを持つゲームオブジェクトの角減衰を取得する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		return dynamicActor->getAngularDamping();
	}
	return 0.0f; // Actorが存在しない場合は0を返す
}

void Physics::SetAngularDrag(const ActorHandle& handle, float angularDrag)
{
	// ここでRigidbodyを持つゲームオブジェクトの角抵抗を設定する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		dynamicActor->setAngularDamping(angularDrag); // PhysXは角抵抗を角減衰として扱うため、setAngularDampingを呼び出す
	}
}

float Physics::GetAngularDrag(const ActorHandle& handle)
{
	// ここでRigidbodyを持つゲームオブジェクトの角抵抗を取得する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		return dynamicActor->getAngularDamping(); // PhysXは角抵抗を角減衰として扱うため、getAngularDampingを呼び出す
	}
	return 0.0f; // Actorが存在しない場合は0を返す
}

void Physics::SetMaxAngularVelocity(const ActorHandle& handle, float maxAngularVelocity)
{
	// ここでRigidbodyを持つゲームオブジェクトの最大角速度を設定する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		dynamicActor->setMaxAngularVelocity(maxAngularVelocity);
	}
}

float Physics::GetMaxAngularVelocity(const ActorHandle& handle)
{
	// ここでRigidbodyを持つゲームオブジェクトの最大角速度を取得する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		return dynamicActor->getMaxAngularVelocity();
	}
	return 0.0f; // Actorが存在しない場合は0を返す
}

void Physics::SetSleepThreshold(const ActorHandle& handle, float sleepThreshold)
{
	// ここでRigidbodyを持つゲームオブジェクトの休止スレッショルドを設定する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		dynamicActor->setSleepThreshold(sleepThreshold);
	}
}

float Physics::GetSleepThreshold(const ActorHandle& handle)
{
	// ここでRigidbodyを持つゲームオブジェクトの休止スレッショルドを取得する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		return dynamicActor->getSleepThreshold();
	}
	return 0.0f; // Actorが存在しない場合は0を返す
}

void Physics::AddForce(const ActorHandle& handle, const Vector3& force, physx::PxForceMode::Enum mode)
{
	// ここでRigidbodyを持つゲームオブジェクトに力を加える処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		dynamicActor->addForce(ToPxVec3(force), mode, true);
	}
}

void Physics::AddLocalForceAtLocalPosition(const ActorHandle& handle, const Vector3& localForce, const Vector3& localPosition, physx::PxForceMode::Enum mode)
{
	// ここでRigidbodyを持つゲームオブジェクトのローカル位置に力を加える処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		physx::PxVec3 physxLocalForce = ToPxVec3(localForce);
		physx::PxVec3 physxLocalPosition = ToPxVec3(localPosition);
		// 物理エンジンの拡張機能を使用して、ローカル位置に力を加える
		physx::PxRigidBodyExt::addLocalForceAtLocalPos(*dynamicActor, physxLocalForce, physxLocalPosition, mode, true);
	}
}

void Physics::AddLocalForceAtPosition(const ActorHandle& handle, const Vector3& localForce, const Vector3& position, physx::PxForceMode::Enum mode)
{
	// ここでRigidbodyを持つゲームオブジェクトの特定の位置に力を加える処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		physx::PxVec3 physxLocalForce = ToPxVec3(localForce);
		physx::PxVec3 physxPosition = ToPxVec3(position);
		// 物理エンジンの拡張機能を使用して、特定の位置に力を加える
		physx::PxRigidBodyExt::addLocalForceAtPos(*dynamicActor, physxLocalForce, physxPosition, mode, true);
	}
}

void Physics::AddForceAtLocalPosition(const ActorHandle& handle, const Vector3& force, const Vector3& localPosition, physx::PxForceMode::Enum mode)
{
	// ここでRigidbodyを持つゲームオブジェクトのローカル位置に力を加える処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		physx::PxVec3 physxForce = ToPxVec3(force);
		physx::PxVec3 physxLocalPosition = ToPxVec3(localPosition);
		// 物理エンジンの拡張機能を使用して、ローカル位置に力を加える
		physx::PxRigidBodyExt::addForceAtLocalPos(*dynamicActor, physxForce, physxLocalPosition, mode, true);
	}
}

void Physics::AddForceAtPosition(const ActorHandle& handle, const Vector3& force, const Vector3& position, physx::PxForceMode::Enum mode)
{
	// ここでRigidbodyを持つゲームオブジェクトの特定の位置に力を加える処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		physx::PxVec3 physxForce = ToPxVec3(force);
		physx::PxVec3 physxPosition = ToPxVec3(position);
		// 物理エンジンの拡張機能を使用して、特定の位置に力を加える
		physx::PxRigidBodyExt::addForceAtPos(*dynamicActor, physxForce, physxPosition, mode, true);
	}
}

void Physics::AddTorque(const ActorHandle& handle, const Vector3& torque, physx::PxForceMode::Enum mode)
{
	// ここでRigidbodyを持つゲームオブジェクトにトルクを加える処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		dynamicActor->addTorque(ToPxVec3(torque), mode, true);
	}
}

void Physics::SetVelocity(const ActorHandle& handle, const Vector3& velocity)
{
	// ここでRigidbodyを持つゲームオブジェクトの速度を直接設定する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		dynamicActor->setLinearVelocity(ToPxVec3(velocity));
	}
}

void Physics::GetVelocity(const ActorHandle& handle, Vector3& outVelocity)
{
	// ここでRigidbodyを持つゲームオブジェクトの速度を直接取得する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		outVelocity = ToVector3(dynamicActor->getLinearVelocity());
	}
	else
	{
		outVelocity = Vector3(0, 0, 0); // Actorが存在しない場合はゼロベクトルを返す
	}
}

void Physics::SetAngularVelocity(const ActorHandle& handle, const Vector3& angularVelocity)
{
	// ここでRigidbodyを持つゲームオブジェクトの角速度を直接設定する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		dynamicActor->setAngularVelocity(ToPxVec3(angularVelocity));
	}
}

void Physics::GetAngularVelocity(const ActorHandle& handle, Vector3& outAngularVelocity)
{
	// ここでRigidbodyを持つゲームオブジェクトの角速度を直接取得する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		outAngularVelocity = ToVector3(dynamicActor->getAngularVelocity());
	}
	else
	{
		outAngularVelocity = Vector3(0, 0, 0); // Actorが存在しない場合はゼロベクトルを返す
	}
}

void Physics::SetKinematic(const ActorHandle& handle, bool isKinematic)
{
	// ここでRigidbodyを持つゲームオブジェクトのキネマティック状態を設定する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, isKinematic);
	}
}

void Physics::SetKinematicTarget(const ActorHandle& handle, const Vector3& pos, const Quaternion& rot)
{
	// ここでRigidbodyを持つゲームオブジェクトの位置を直接設定する処理を実装
	if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
	{
		if (dynamicActor->getActorFlags() & physx::PxActorFlag::eDISABLE_SIMULATION)
		{
			Console::LogWarning("Attempting to set kinematic target on an actor that is disabled for simulation. This will have no effect.");
			return; // シミュレーションが無効なActorに対してキネマティックターゲットを設定しようとした場合は警告を出して処理をスキップする
		}
		physx::PxQuat quat = ToPxQuat(rot);
		quat.normalize(); // クォータニオンを正規化して回転の安定性を保つ
		dynamicActor->setKinematicTarget(physx::PxTransform(ToPxVec3(pos), quat));
	}
}

void Physics::SetGlobalPose(const ActorHandle& handle, const Vector3& pos, const Quaternion& rot)
{
	// ここでRigidbodyを持つゲームオブジェクトの位置を直接設定する処理を実装
	if (physx::PxRigidActor* actor = GetActor(handle))
	{
		physx::PxVec3 position = ToPxVec3(pos);
		physx::PxQuat rotation = ToPxQuat(rot);
		rotation.normalize(); // クォータニオンを正規化して回転の安定性を保つ
		// Actorのグローバルポーズを設定
		if (physx::PxRigidDynamic* dynamicActor = GetRigidDynamic(handle))
		{
			// 位置を直接設定する前に速度をゼロにして、物理シミュレーションによる予期せぬ動きを防止する
			dynamicActor->setLinearVelocity({ 0,0,0 });
			dynamicActor->setAngularVelocity({ 0,0,0 });
		}
		actor->setGlobalPose(physx::PxTransform(position, rotation));
	}
}

void Physics::GetGlobalPose(const ActorHandle& handle, Vector3& outPos, Quaternion& outRot)
{
	// ここでRigidbodyを持つゲームオブジェクトの位置を直接設定する処理を実装
	if (physx::PxRigidActor* actor = GetActor(handle))
	{
		physx::PxTransform globalPose = actor->getGlobalPose();
		outPos = ToVector3(globalPose.p);
		outRot = ToQuaternion(globalPose.q);
	}
}

void Physics::GetLocalPose(const ShapeHandle& shapeHandle, Vector3& outLocalPos, Quaternion& outLocalRot)
{
	// ここでActorHandle に対応する Actor 内の ShapeHandle に対応するShapeのローカル位置と回転を取得する処理を実装
	if (physx::PxShape* shape = GetShape(shapeHandle))
	{
		physx::PxTransform localPose = shape->getLocalPose();
		outLocalPos = ToVector3(localPose.p);
		outLocalRot = ToQuaternion(localPose.q);
	}
}

void Physics::SetLocalPose(const ShapeHandle& shapeHandle, const Vector3& localPos, const Quaternion& localRot)
{
	// ここでActorHandle に対応する Actor 内の ShapeHandle に対応するShapeのローカル位置と回転を設定する処理を実装
	if (physx::PxShape* shape = GetShape(shapeHandle))
	{
		// ShapeがActorにアタッチされているか確認
		if (shape->getActor() && shape->getActor()->is<physx::PxRigidActor>())
		{
			// Shapeのローカルポーズを設定
			physx::PxVec3 pos = ToPxVec3(localPos);
			physx::PxQuat quat = ToPxQuat(localRot);
			quat.normalize(); // クォータニオンを正規化して回転の安定性を保つ
			physx::PxTransform localPose = physx::PxTransform(pos, quat);
			if (!localPose.isValid())
			{
				_ASSERT_EXPR(false, "Invalid local pose! Position and rotation must be finite and rotation must be normalized.");
				return; // ローカルポーズが無効な場合は設定をスキップ
			}
			shape->setLocalPose(localPose);
		}

		// ローカルポーズを変更した後は、動的なActorを起こす必要がある場合があるため、wakeUpを呼び出す
		if (physx::PxRigidDynamic* dynamic = shape->getActor()->is<physx::PxRigidDynamic>())
		{
			if (dynamic->getRigidBodyFlags() & physx::PxRigidBodyFlag::eKINEMATIC)
			{
				// キネマティックなActorの場合は位置を直接設定しているため、wakeUpは必要ない
			}
			else
			{
				dynamic->wakeUp();
			}
		}
	}
}

void Physics::GetGeometry(const ShapeHandle& shapeHandle, physx::PxGeometry& outGeometry)
{
	// ここでActorHandle に対応する Actor 内の ShapeHandle に対応するShapeのジオメトリを取得する処理を実装
	if (physx::PxShape* shape = GetShape(shapeHandle))
	{
		outGeometry = shape->getGeometry(); // Shapeのジオメトリを取得
	}
}

void Physics::SetGeometry(const ShapeHandle& shapeHandle, const physx::PxGeometry& geometry)
{
	// ここでActorHandle に対応する Actor 内の ShapeHandle に対応するShapeのジオメトリを設定する処理を実装
	if (physx::PxShape* shape = GetShape(shapeHandle))
	{
		shape->setGeometry(geometry); // Shapeのジオメトリを設定
	}
}

void Physics::SetActorEnable(const ActorHandle& actorHandle, bool enable)
{
	// ここでActorHandle に対応する Actor を有効/無効にする処理を実装
	if (physx::PxRigidActor* actor = GetActor(actorHandle))
	{
		actor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, !enable); // シミュレーションフラグを設定
	}
}

bool Physics::IsActorEnabled(const ActorHandle& actorHandle)
{
	// ここでActorHandle に対応する Actor が有効かどうかを返す処理を実装
	if (physx::PxRigidActor* actor = GetActor(actorHandle))
	{
		return !(actor->getActorFlags() & physx::PxActorFlag::eDISABLE_SIMULATION); // シミュレーション無効フラグが立っていない場合は有効とみなす
	}
	return false; // Actorが存在しない場合はfalseを返す
}

void Physics::SetShapeEnable(const ShapeHandle& shapeHandle, bool enable, const ColliderData& colliderData)
{
	// ここでActorHandle に対応する Actor 内の ShapeHandle に対応するShapeを有効/無効にする処理を実装
	if (physx::PxShape* shape = GetShape(shapeHandle))
	{
		// 一旦シミュレーションシェイプフラグとトリガーシェイプフラグを両方ともリセットしてから、isTriggerの値に応じてフラグを設定する
		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false); // シミュレーションシェイプフラグをリセット
		shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false); // トリガーシェイプフラグをリセット	

		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, colliderData.isTrigger ? false : enable); // シミュレーションシェイプフラグを設定
		shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, enable ? colliderData.isTrigger : false); // トリガーシェイプフラグを設定
		shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, enable); // シーンクエリーシェイプフラグを設定
		//shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, enable); // ビジュアライゼーションフラグを設定(デバッグ用)
		shape->userData = enable ? colliderData.collider : nullptr; // シェイプのユーザーデータにColliderを設定(有効な場合はColliderを、無効な場合はnullptrを設定)
	}
}

bool Physics::IsShapeEnabled(const ShapeHandle& shapeHandle)
{
	// ここでActorHandle に対応する Actor 内の ShapeHandle に対応するShapeが有効かどうかを返す処理を実装
	if (physx::PxShape* shape = GetShape(shapeHandle))
	{
		bool isSimulationShape = shape->getFlags() & physx::PxShapeFlag::eSIMULATION_SHAPE;
		bool isTriggerShape = shape->getFlags() & physx::PxShapeFlag::eTRIGGER_SHAPE;
		bool isSceneQueryShape = shape->getFlags() & physx::PxShapeFlag::eSCENE_QUERY_SHAPE;
		//bool isVisualizationShape = shape->getFlags() & physx::PxShapeFlag::eVISUALIZATION; // デバッグ用のビジュアライゼーションフラグも考慮する場合
		bool isUserDataCond = shape->userData != nullptr; // ユーザーデータがnullptrでないことも有効の条件にする場合
		bool isEnabled = isSimulationShape || isTriggerShape || isSceneQueryShape; // いずれかのフラグが立っていれば有効とみなす
		return isEnabled && isUserDataCond; // フラグが立っていて、ユーザーデータも有効な場合にtrueを返す
	}
	return false; // Shapeが存在しない場合はfalseを返す
}

ActorHandle Physics::GetActorHandle(Transform* transform)
{
	// ここでTransform* に対応する ActorHandle を取得、存在しない場合は-1を返す処理を実装
	for (const auto& [key, value] : m_actorMap)
	{
		if (value.transform == transform)
		{
			return key; // 対応するActorHandleを返す
		}
	}
	return INVALID_ACTOR_HANDLE; // 存在しない場合は-1を返す
}

ActorHandle Physics::CreateActorHandle()
{
	// ここで ActorHandle を新規作成する処理を実装
	ActorHandle newHandle = nextActorHandle++;
	m_actorMap[newHandle] = ActorData{ nullptr, nullptr }; // 新しいActorHandleをマップに追加(初期値はnullptr)
	return newHandle;
}

Transform* Physics::GetTransform(ActorHandle actorHandle)
{
	// ここでActorHandle に対応する Transform* を取得、存在しない場合はnullptrを返す処理を実装
	if (m_actorMap.contains(actorHandle))
	{
		return m_actorMap[actorHandle].transform; // 対応するTransform*を返す
	}
	return nullptr; // 存在しない場合はnullptrを返す
}

bool Physics::HasActor(Transform* transform)
{
	// ここでTransform* に対応する ActorHandle が存在するかどうかを返す処理を実装
	return GetActorHandle(transform) != INVALID_ACTOR_HANDLE;
}

bool Physics::HasShape(ShapeHandle shapeHandle)
{
	// ここでShapeHandle に対応するShapeが存在するかどうかを返す処理を実装
	const auto& shapes = m_shapeMap;
	return shapes.contains(shapeHandle);
}


physx::PxRigidActor* Physics::GetActor(ActorHandle actorHandle)
{
	// ここでActorHandle に対応する PxRigidActor* を取得、存在しない場合はnullptrを返す処理を実装
	if (m_actorMap.contains(actorHandle))
	{
		return m_actorMap[actorHandle].actor;
	}
	return nullptr; // 存在しない場合はnullptrを返す
}

physx::PxShape* Physics::GetShape(ShapeHandle shapeHandle)
{
	// ここでShapeHandle に対応する PxShape* を取得、存在しない場合はnullptrを返す処理を実装
	const auto& shapes = m_shapeMap;
	if (shapes.contains(shapeHandle))
	{
		auto shape = shapes.at(shapeHandle);
		if (shape)
			return shape;
	}
	return nullptr; // 存在しない場合はnullptrを返す
}

ShapeHandle Physics::GetShapeHandle(physx::PxShape* shape)
{
	// ここでPxShape* に対応する ShapeHandle を取得、存在しない場合は-1を返す処理を実装
	const auto& shapes = m_shapeMap;
	for (const auto& pair : shapes)
	{
		if (pair.second == shape)
		{
			return pair.first; // 対応するShapeHandleを返す
		}
	}
	return INVALID_SHAPE_HANDLE; // 存在しない場合は-1を返す
}

ShapeHandle Physics::CreateShapeHandle()
{
	m_shapeMap[nextShapeHandle] = nullptr; // 新しいShapeHandleをマップに追加(初期値はnullptr)
	return nextShapeHandle++; // 新しいShapeHandleを返し、次のShapeHandleにインクリメント
}

void Physics::RegisterShape(ShapeHandle shapeHandle, physx::PxShape* shape)
{
	if (shapeHandle != INVALID_SHAPE_HANDLE && shape)
	{
		m_shapeMap[shapeHandle] = shape; // ShapeHandleとPxShape*のマッピングを保存
	}
}

void Physics::RemoveActor(ActorHandle actorHandle)
{
	if (m_actorMap.contains(actorHandle))
	{
		if (physx::PxRigidActor* actor = m_actorMap[actorHandle].actor)
		{
			actor->userData = nullptr; // Actorのユーザーデータをnullptrに設定して、Transformへの参照を切る

			// アタッチされているShapeのユーザーデータもnullptrに設定して、Colliderへの参照を切る
			physx::PxU32 numShapes = actor->getNbShapes();
			if (numShapes > 0)
			{
				std::vector<physx::PxShape*> shapes(numShapes);
				actor->getShapes(shapes.data(), numShapes);
				for (physx::PxShape* shape : shapes)
				{
					if (shape)
					{
						shape->userData = nullptr; // シェイプのユーザーデータをnullptrに設定して、Colliderへの参照を切る
						m_shapeMap.erase(GetShapeHandle(shape)); // マップから削除
					}
				}
			}

			GetScene()->removeActor(*actor); // シーンからActorを削除
			PX_RELEASE(actor); // Actorを解放
			m_actorMap.erase(actorHandle); // マップから削除
		}
	}
}

void Physics::RemoveShape(ShapeHandle shapeHandle)
{
	if (physx::PxShape* shape = GetShape(shapeHandle)) // シェイプが存在するか確認(存在しない場合はnullptrが返される)
	{
		if (physx::PxRigidActor* actor = shape->getActor())
		{
			// トリガーの場合は、ColliderのOnTriggerExitが呼び出されるようにするために、シェイプを削除する前にトリガーイベントを発生させる必要がある
			if (shape->getFlags() & physx::PxShapeFlag::eTRIGGER_SHAPE)
			{
				// トリガーイベントを発生させるために、ColliderのOnTriggerExitを呼び出す
				m_simulationEventCallback.ClearTriggerStayPairsForShape(shape);
			}
			else // トリガーでない場合は、ColliderのOnCollisionExitが呼び出されるようにするために、シェイプを削除する前に衝突イベントを発生させる必要がある
			{
				CollisionInfo collisionInfo{};
				collisionInfo.selfCollider = static_cast<Collider*>(shape->userData); // シェイプのユーザーデータからColliderを取得
				collisionInfo.otherCollider = nullptr; // 他のColliderはnullptrに設定(シェイプを削除する前に衝突イベントを発生させるため、他のColliderは特定できないためnullptrに設定)
				if (collisionInfo.selfCollider)
				{
					collisionInfo.selfCollider->OnCollisionExit(collisionInfo); // ColliderのOnCollisionExitを呼び出す
				}
			}

			// ShapeがActorにアタッチされている場合は、まずActorからシェイプを削除してからシェイプを解放する必要がある
			shape->userData = nullptr; // シェイプのユーザーデータをnullptrに設定して、Colliderへの参照を切る
			shape->acquireReference(); // シェイプの参照を取得して、シェイプが解放されないようにする
			actor->detachShape(*shape); // Actorからシェイプを削除
			PX_RELEASE(shape); // シェイプを解放

			// TODO: マップから削除するのはループの外で行う必要があるかもしれない(ループ内でマップを変更するとイテレータが無効になる可能性があるため)
			m_shapeMap.erase(shapeHandle); // マップから削除

			// シェイプを削除した後に、Actorにアタッチされているシェイプがなくなった場合は、Actor自体も削除する
			physx::PxU32 numShapes = actor->getNbShapes();
			if (numShapes == 0)
			{
				// Actorにアタッチされているシェイプがなくなった場合は、Actor自体も削除する
				for (auto& [key, value] : m_actorMap)
				{
					if (value.actor == actor)
					{
						RemoveActor(key); // Actorを削除
						break; // マップをループしているので、対応するActorHandleが見つかったらループを抜ける
					}
				}
			}
		}
	}
}

//void Physics::ClearShapes(ActorHandle actorHandle)
//{
//	// ここでActorHandle に対応する Actor からすべてのShapeを削除する処理を実装
//	if (physx::PxRigidActor* actor = GetActor(actorHandle))
//	{
//		auto& shapes = m_shapeMap;
//
//		// 削除するShapeHandleを一時的に保存するベクター
//		std::vector<ShapeHandle> shapeHandlesToRemove;
//
//		for (auto& [shapeHandle, shape] : shapes)
//		{
//			if (shapeHandle != INVALID_SHAPE_HANDLE && shape)
//			{
//				if (shape->getActor() == actor)
//				{
//					shape->acquireReference(); // シェイプの参照を取得して、シェイプが解放されないようにする
//					actor->detachShape(*shape); // Actorからシェイプを削除
//					PX_RELEASE(shape); // シェイプを解放
//
//					// TODO: マップから削除するのはループの外で行う必要があるかもしれない(ループ内でマップを変更するとイテレータが無効になる可能性があるため)
//					shapeHandlesToRemove.push_back(shapeHandle); // 削除するShapeHandleを保存
//				}
//			}
//		}
//
//		for (ShapeHandle shapeHandle : shapeHandlesToRemove)
//		{
//			shapes.erase(shapeHandle); // マップから削除
//		}
//	}
//}
