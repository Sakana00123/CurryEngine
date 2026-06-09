#include "pch.h"
#include "Collider.h"
#include "Engine/Rendering/Renderers/Renderer.h"
#include "Engine/Physics/Rigidbody.h"
#include "Engine/Physics/CollisionEvent.h"
#include "Engine/Core/Layer.h"
#include "Engine/Core/ScriptComponent.h"

void Collider::OnEnable()
{
	// それぞれのコライダーの Register() 内で Physics::CreateShape を呼び出しているため、ここではコライダーを有効にするだけで十分。
	if (m_shapeHandle != INVALID_SHAPE_HANDLE)
	{
		// 物理エンジンにコライダーを有効化するよう指示
		ColliderData colliderData{};
		colliderData.materialHandle = m_materialHandle;
		colliderData.isTrigger = isTrigger;
		colliderData.contactOffset = contactOffset;
		colliderData.collider = this;
		Physics::SetShapeEnable(m_shapeHandle, true, colliderData);
	}
}

void Collider::OnDisable()
{
	//Physics::RemoveShape(m_actorHandle, m_shapeHandle);

	// Physics のコライダー削除は OnDestroy で行う。OnDisable は単にフラグをセットするだけ。(無効にするときはcolliderDataは無視されるため、セットする必要がない)
	Physics::SetShapeEnable(m_shapeHandle, false, {});
}

void Collider::OnDestroy()
{
	// 物理エンジンからコライダーを削除
	if (m_shapeHandle != INVALID_SHAPE_HANDLE)
	{
		Physics::RemoveShape(m_shapeHandle);
		m_shapeHandle = INVALID_SHAPE_HANDLE; // ハンドルを無効化
	}
}

void Collider::Finalize()
{
	// 物理エンジンからコライダーを削除
	Physics::UnregisterPendingCollider(this); // 登録保留リストから削除
	if (m_shapeHandle != INVALID_SHAPE_HANDLE)
	{
		Physics::RemoveShape(m_shapeHandle);
		m_shapeHandle = INVALID_SHAPE_HANDLE; // ハンドルを無効化
	}
}

void Collider::OnTransformChanged()
{
	// トランスフォームが変更されたことをフラグで記録。実際の同期は FixedUpdate で行う。
	m_needSync = true;
}

void Collider::Awake()
{
	// Rigidbody を持つ場合は Rigidbody の Awake で Actor として登録されるため、Collider はここでは何もしない。
	Physics::RegisterPendingCollider(this);
}

void Collider::Start()
{
	// 物理エンジンに該当のマテリアルが存在するか確認し、存在しない場合はデフォルトマテリアルにフォールバックする
	if (!Physics::GetMaterial(m_materialHandle))
	{
		Console::LogWarning("MaterialHandle " + std::to_string(m_materialHandle) + " not found. Falling back to default material.");
		m_materialHandle = DEFAULT_MATERIAL_HANDLE;
	}
}

void Collider::Initialize()
{
	primitive = std::make_unique<GeometricPrimitive>(Graphics::GetDevice());
}

void Collider::FixedUpdate(float fixedDeltaTime)
{
	// コライダーが物理エンジンに登録されていて、トランスフォームの変更があった場合に同期を行う
	if (m_shapeHandle != INVALID_SHAPE_HANDLE)
	{
		if (m_needSync)
		{
			// トランスフォームの変更を物理エンジンに同期
			SyncWithPhysics();
			m_needSync = false; // 同期後にフラグをリセット
		}

		// レイヤーの変更があった場合も物理エンジンに同期(今はレイヤーの変更フラグがないため、常に同期するようにしている)
		{
			// レイヤーとレイヤーマスクを取得
			Layer layer = GetOwner()->GetLayer();
			LayerMask layerMask = Physics::GetCollisionMask(layer); // 物理エンジンからレイヤーに対応するレイヤーマスクを取得(後にoverrideを追加して、コライダーごとに個別のレイヤーマスクを設定できるようにするかも)

			Physics::UpdateFilterData(m_shapeHandle, layer, layerMask);
		}
	}
}

void Collider::LateUpdate(float deltaTime)
{
	// サイズとオフセットをメッシュのバウンディングボックスから自動設定
	if (autoFit)
	{
		if (Renderer* renderer = GetOwner()->GetComponent<Renderer>())
		{
			if (renderer->IsEnabled())
			{
				Math::BoundingBox box = renderer->CalculateAABB(); // 念のため再計算
				// コライダーのサイズとオフセットをバウンディングボックスにフィットさせる
				FitToBoundingBox(box.Center(), box.Size());
			}
		}
	}
}

void Collider::BeginFrame()
{
	//// 前回の衝突結果を保存
	//previousCollisions = currentCollisions;
	//currentCollisions.clear(); // 今回の衝突結果は空で始める
}

void Collider::ReportCollision(Collider* other, const CollisionInfo& info)
{
	currentCollisions[other] = info;
}

void Collider::EndFrame()
{
}

XMFLOAT4X4 Collider::CalculateColliderWorldTransform(const Vector3& localPos, const Vector3& localScale) const
{
	// ワールド行列を計算
	XMFLOAT3 position = GetTransform()->GetWorldPosition();
	XMFLOAT4 rotation = GetTransform()->GetWorldRotation();
	XMMATRIX LT{ XMMatrixTranslation(position.x, position.y, position.z) };
	XMMATRIX LR{ XMMatrixRotationQuaternion(XMLoadFloat4(&rotation)) };
	XMMATRIX L = { LR * LT };
	XMMATRIX S{ XMMatrixScaling(localScale.x, localScale.y, localScale.z) };
	XMMATRIX R{ XMMatrixRotationRollPitchYaw(0,0,0) };
	XMMATRIX T{ XMMatrixTranslation(localPos.x, localPos.y, localPos.z) };
	XMMATRIX W{ S * R * T };
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, W * L);
	return world;
}

bool Collider::IsTrigger()
{
	return isTrigger;
}

void Collider::SetTrigger(bool trigger)
{
	isTrigger = trigger;
	// 物理エンジンにトリガー設定を反映
	if (m_shapeHandle != INVALID_SHAPE_HANDLE) {
		Physics::SetTrigger(m_shapeHandle, isTrigger);
	}
}

float Collider::GetContactOffset() const
{
	return contactOffset;
}

void Collider::SetContactOffset(float offset)
{
	contactOffset = offset;
	// 物理エンジンに接触オフセットを反映
	if (m_shapeHandle != INVALID_SHAPE_HANDLE) {
		Physics::SetContactOffset(m_shapeHandle, contactOffset);
	}
}

void Collider::SetMaterial(MaterialHandle materialHandle)
{
	// マテリアルハンドルを保存
	m_materialHandle = materialHandle;
	// 物理エンジンにマテリアルを設定
	if (m_shapeHandle != INVALID_SHAPE_HANDLE) {
		Physics::SetMaterial(m_shapeHandle, m_materialHandle);
	}
}

void Collider::SetMaterialData(const PhysicsMaterialData& data)
{
	// 物理マテリアルの特性を物理エンジンに設定
	if (m_materialHandle != INVALID_MATERIAL_HANDLE) {
		Physics::SetMaterialData(m_materialHandle, data);
	}
}

PhysicsMaterialData Collider::GetMaterialData() const
{
	// 物理エンジンから物理マテリアルの特性を取得
	PhysicsMaterialData data;
	if (m_materialHandle != INVALID_MATERIAL_HANDLE) {
		Physics::GetMaterialData(m_materialHandle, data);
	}
	return data;
}

void Collider::SetNeedSync()
{
	m_needSync = true;
}

void Collider::DrawProperty()
{
#ifdef USE_IMGUI

	//ImGui::Text("ShapeHandle: %d", m_shapeHandle);
	//ImGui::Text("MaterialHandle: %d", m_materialHandle);

	bool propertyChanged = false;
	IMGUI_PROPERTY_BOOL("IsTrigger", isTrigger, propertyChanged);

	//if (ImGui::Checkbox("IsTrigger", &isTrigger))
	if (propertyChanged)
	{
		SetTrigger(isTrigger);
	}

	//ImGui::Checkbox("autoFit", &autoFit);
	propertyChanged = false;
	IMGUI_PROPERTY_BOOL("Auto Fit To Mesh", autoFit, propertyChanged);
	
	// 物理エンジンに該当のマテリアルが存在するか確認し、存在しない場合はデフォルトマテリアルにフォールバックする
	if (!Physics::GetMaterial(m_materialHandle))
	{
		Console::LogWarning("MaterialHandle " + std::to_string(m_materialHandle) + " not found. Falling back to default material.");
		m_materialHandle = DEFAULT_MATERIAL_HANDLE;
	}

	// 接触オフセットの編集
	IMGUI_PROPERTY_FLOAT("Contact Offset", contactOffset, propertyChanged);
	if (propertyChanged)
	{
		SetContactOffset(contactOffset);
	}

	// マテリアルの編集
	{
		// マテリアルハンドルの編集
		std::vector<const char*> materialNames;
		std::vector<MaterialHandle> materialHandles;
		// 物理エンジンからすべてのマテリアル名とハンドルを取得
		Physics::GetAllMaterialNamesAndHandles(materialNames, materialHandles);
		int currentIndex = 0;
		for (size_t i = 0; i < materialHandles.size(); ++i)
		{
			if (materialHandles[i] == m_materialHandle)
			{
				currentIndex = static_cast<int>(i);
				break;
			}
		}
		// マテリアルのコンボボックスを表示
		IMGUI_PROPERTY("Material");
		if (ImGui::Combo("##Material", &currentIndex, materialNames.data(), static_cast<int>(materialNames.size())))
		{
			MaterialHandle oldHandle = m_materialHandle;
			MaterialHandle newHandle = materialHandles[currentIndex];
			IMGUI_PROPERTY_COMMAND_CUSTOM_SIMPLE("Material", newHandle, oldHandle, [this](const MaterialHandle& handle) {
				SetMaterial(handle);// マテリアルを更新
				});
		}
	}
#endif // USE_IMGUI
}

json Collider::Serialize() const
{
	json j = Component::Serialize();

	j["material"] = m_materialHandle;

	return j;
}

void Collider::Deserialize(const json& j)
{
	Component::Deserialize(j);
	
	if (j.contains("material"))
	{
		m_materialHandle = j["material"].get<MaterialHandle>();
	}
}

void Collider::AddOnCollisionEnterEvent(const std::function<void(const CollisionInfo&)>& callback)
{
	onCollisionEnterEvents.push_back(callback);
}

void Collider::AddOnCollisionStayEvent(const std::function<void(const CollisionInfo&)>& callback)
{
	onCollisionStayEvents.push_back(callback);
}

void Collider::AddOnCollisionExitEvent(const std::function<void(const CollisionInfo&)>& callback)
{
	onCollisionExitEvents.push_back(callback);
}

void Collider::AddOnTriggerEnterEvent(const std::function<void(const TriggerInfo&)>& callback)
{
	onTriggerEnterEvents.push_back(callback);
}

void Collider::AddOnTriggerStayEvent(const std::function<void(const TriggerInfo&)>& callback)
{
	onTriggerStayEvents.push_back(callback);
}

void Collider::AddOnTriggerExitEvent(const std::function<void(const TriggerInfo&)>& callback)
{
	onTriggerExitEvents.push_back(callback);
}

void Collider::OnCollisionEnter(const CollisionInfo& info)
{
	for (auto& event : onCollisionEnterEvents) {
		event(info);
	}
	for (auto& callback : GetOwner()->GetComponents<ICollisionEventCallback>()) {
		if (auto component = dynamic_cast<Component*>(callback); component && component->IsEnabled()) {
			callback->OnCollisionEnter(info);
		}
	}
	/*if (!onCollisionEnterEvents.empty())
	{
		Console::Log("OnCollisionEnter: " + GetOwner()->name + " hit " + info.other->name + " by " + info.self->name);
	}*/
}

void Collider::OnCollisionStay(const CollisionInfo& info)
{
	for (auto& event : onCollisionStayEvents) {
		event(info);
	}
	for (auto& callback : GetOwner()->GetComponents<ICollisionEventCallback>()) {
		if (auto component = dynamic_cast<Component*>(callback); component && component->IsEnabled()) {
			callback->OnCollisionStay(info);
		}
	}
}

void Collider::OnCollisionExit(const CollisionInfo& info)
{
	for (auto& event : onCollisionExitEvents) {
		event(info);
	}
	for (auto& callback : GetOwner()->GetComponents<ICollisionEventCallback>()) {
		if (auto component = dynamic_cast<Component*>(callback); component && component->IsEnabled()) {
			callback->OnCollisionExit(info);
		}
	}
}

void Collider::OnTriggerEnter(const TriggerInfo& info)
{
	for (auto& event : onTriggerEnterEvents) {
		event(info);
	}
	for (auto& callback : GetOwner()->GetComponents<ITriggerEventCallback>()) {
		if (auto component = dynamic_cast<Component*>(callback); component && component->IsEnabled()) {
			callback->OnTriggerEnter(info);
		}
	}
}

void Collider::OnTriggerStay(const TriggerInfo& info)
{
	for (auto& event : onTriggerStayEvents) {
		event(info);
	}
	for (auto& callback : GetOwner()->GetComponents<ITriggerEventCallback>()) {
		if (auto component = dynamic_cast<Component*>(callback); component && component->IsEnabled()) {
			callback->OnTriggerStay(info);
		}
	}
}

void Collider::OnTriggerExit(const TriggerInfo& info)
{
	for (auto& event : onTriggerExitEvents) {
		event(info);
	}
	for (auto& callback : GetOwner()->GetComponents<ITriggerEventCallback>()) {
		if (auto component = dynamic_cast<Component*>(callback); component && component->IsEnabled()) {
			callback->OnTriggerExit(info);
		}
	}
}