#include "pch.h"
#include "CapsuleCollider.h"

REGISTER_COMPONENT(CapsuleCollider, "Physics")

void CapsuleCollider::Initialize()
{
	// デバッグプリミティブの準備など、必要な初期化処理をここに実装します。
	Collider::Initialize();
}

void CapsuleCollider::Register()
{
	Vector3 worldScale = Vector3(GetTransform()->GetWorldScale());
	CapsuleColliderData data;
	data.radius = radius; // 半径を指定
	data.height = height; // 高さを指定
	data.center = Vector3(center); // オフセットを中心位置として設定（ワールドスケールは CapsuleColliderData 内で考慮されるため、ここでは適用しない）
	data.materialHandle = m_materialHandle; // マテリアルハンドルを設定
	data.isTrigger = isTrigger; // トリガーかどうかのフラグを設定
	data.contactOffset = contactOffset; // 接触オフセットを設定
	data.collider = this; // コライダへのポインタを設定（必要に応じて）

	// 物理エンジンにコライダーを登録
	if (!Physics::AddCapsuleShape(GetTransform(), data, m_shapeHandle))
	{
		// 追加に失敗した場合のエラーハンドリング
		LOG_ERROR("Failed to add CapsuleCollider shape to physics engine.");
	}
}

void CapsuleCollider::FitToBoundingBox(const Vector3& center, const Vector3& size)
{
	this->center = center;
	radius = min(size.x, size.z); // XZ平面のサイズから半径を決定
	height = max(0.0f, size.y - radius * 2.0f); // 高さはYサイズから半径分を引いたもの（負にならないようにmaxで調整）
	SetNeedSync(); // 物理エンジンとの状態同期が必要なことをマーク
}

void CapsuleCollider::SyncWithPhysics()
{
	// 物理エンジンにローカルポーズを更新
	Vector3 position = Vector3(center); // オフセットをワールドスケールで調整してローカル座標に変換
	Quaternion rotation = Quaternion::FromEuler({ 0.0f, 0.0f, 90.0f }); // カプセルの向きをY軸からZ軸に変更（PhysXのカプセルはデフォルトでY軸に沿っているため）
	Physics::SetLocalPose(m_shapeHandle, position, rotation);

	// サイズの変更も反映
	physx::PxCapsuleGeometry geometry(radius, height); // 半径と高さを指定
	if (!geometry.isValid())
	{
		LOG_ERROR("Invalid capsule geometry parameters. Radius must be > 0 and height must be >= 0.");
		return;
	}
	Physics::SetGeometry(m_shapeHandle, geometry);
}

//#ifdef USE_IMGUI
//void CapsuleCollider::DrawProperty(const PropertyDrawContext& context)
//{
//	IMGUI_PROPERTY_BEGIN();
//	Collider::DrawProperty(context);
//	bool isChanged = false;
//
//	IMGUI_PROPERTY_VECTOR3("Center", center, isChanged);
//	IMGUI_PROPERTY_FLOAT("Radius", radius, isChanged, 0.1f, 0.0f, FLT_MAX);
//	IMGUI_PROPERTY_FLOAT("Height", height, isChanged, 0.1f, 0.0f, FLT_MAX);
//
//	if (isChanged)
//	{
//		SetNeedSync(); // 物理エンジンとの状態同期が必要なことをマーク
//	}
//	IMGUI_PROPERTY_END();
//}
//#endif

json CapsuleCollider::Serialize() const
{
	json j = Collider::Serialize();
	j["center"] = { center.x, center.y, center.z };
	j["radius"] = radius;
	j["height"] = height;
	return j;
}

void CapsuleCollider::Deserialize(const json& j)
{
	Collider::Deserialize(j);
	if (j.contains("center") && j["center"].is_array() && j["center"].size() == 3)
	{
		center.x = j["center"][0].get<float>();
		center.y = j["center"][1].get<float>();
		center.z = j["center"][2].get<float>();
	}
	if (j.contains("radius") && j["radius"].is_number())
	{
		radius = j["radius"].get<float>();
	}
	if (j.contains("height") && j["height"].is_number())
	{
		height = j["height"].get<float>();
	}
}

Vector3 CapsuleCollider::GetCenter() const
{
	return center;
}

void CapsuleCollider::SetCenter(const Vector3& newCenter)
{
	center = newCenter;
	SetNeedSync(); // 物理エンジンとの状態同期が必要なことをマーク
}

float CapsuleCollider::GetRadius() const
{
	return radius;
}

void CapsuleCollider::SetRadius(float newRadius)
{
	radius = newRadius;
	SetNeedSync(); // 物理エンジンとの状態同期が必要なことをマーク
}

float CapsuleCollider::GetHeight() const
{
	return height;
}

void CapsuleCollider::SetHeight(float newHeight)
{
	height = newHeight;
	SetNeedSync(); // 物理エンジンとの状態同期が必要なことをマーク
}
