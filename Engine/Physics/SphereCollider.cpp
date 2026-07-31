#include "pch.h"
#include "SphereCollider.h"

REGISTER_COMPONENT(SphereCollider, "Physics")

void SphereCollider::Initialize()
{
	// デバッグプリミティブの準備など、必要な初期化処理をここに実装します。
	Collider::Initialize();
}

void SphereCollider::Register()
{
	Vector3 worldScale = Vector3(GetTransform()->GetWorldScale());

	SphereColliderData data;
	data.radius = radius; // 半径を指定
	data.center = Vector3(center); // オフセットを中心位置として設定（ワールドスケールは SphereColliderData 内で考慮されるため、ここでは適用しない）
	data.materialHandle = m_materialHandle; // マテリアルハンドルを設定
	data.isTrigger = isTrigger; // トリガーかどうかのフラグを設定
	data.contactOffset = contactOffset; // 接触オフセットを設定
	data.collider = this; // コライダへのポインタを設定（必要に応じて）

	// 物理エンジンにコライダーを登録
	if (!Physics::AddSphereShape(GetTransform(), data, m_shapeHandle))
	{
		// 追加に失敗した場合のエラーハンドリング
		LOG_ERROR("Failed to add SphereCollider shape to physics engine.");
	}
}

void SphereCollider::FitToBoundingBox(const Vector3& center, const Vector3& size)
{
	this->center = center; // オフセットを中心位置として設定
	radius = max(size.x, size.z); // XZ平面の最大サイズを半径とする
	SetNeedSync(); // 物理エンジンとの状態同期が必要であることを示すフラグを立てる
}

void SphereCollider::SyncWithPhysics()
{
	// 物理エンジンにローカルポーズを更新
	Vector3 position = Vector3(center); // オフセットをワールドスケールで調整してローカル座標に変換
	Quaternion rotation = Quaternion(0, 0, 0, 1);
	Physics::SetLocalPose(m_shapeHandle, position, rotation);

	// サイズの変更も反映
	radius = max(radius, 0.01f); // 半径が0以下にならないように最低値を設定
	physx::PxSphereGeometry geometry(radius); // 半径を指定（サイズのx成分を使用）
	if (!geometry.isValid())
	{
		LOG_ERROR("Invalid geometry parameters for SphereCollider. Radius must be greater than 0.");
		return;
	}
	Physics::SetGeometry(m_shapeHandle, geometry);
}

#ifdef USE_IMGUI
//void SphereCollider::DrawProperty(const PropertyDrawContext& context)
//{
//	IMGUI_PROPERTY_BEGIN();
//	Collider::DrawProperty(context);
//	bool isChanged = false;
//	//isChanged |= ImGui::DragFloat3("Center", &center.x);
//	//isChanged |= ImGui::DragFloat("Radius", &radius, 0.1f, 0.0f, FLT_MAX);
//	IMGUI_PROPERTY_VECTOR3("Center", center, isChanged);
//	IMGUI_PROPERTY_FLOAT("Radius", radius, isChanged, 0.1f, 0.0f, FLT_MAX);
//
//	if (isChanged)
//	{
//		// プロパティが変更された場合、物理エンジンとの状態同期が必要であることを示すフラグを立てます。
//		SetNeedSync();
//	}
//	IMGUI_PROPERTY_END();
//}
#endif // USE_IMGUI

json SphereCollider::Serialize() const
{
	json j = Collider::Serialize();
	j["center"] = { center.x, center.y, center.z };
	j["radius"] = radius;
	return j;
}

void SphereCollider::Deserialize(const json& j)
{
	Collider::Deserialize(j);
	// "center" プロパティが存在し、配列であることを確認してから読み取ります。
	if (j.contains("center") && j["center"].is_array() && j["center"].size() == 3)
	{
		center.x = j["center"][0].get<float>();
		center.y = j["center"][1].get<float>();
		center.z = j["center"][2].get<float>();
	}
	// "offset" プロパティが存在し、配列であることを確認してから読み取ります。(旧プロパティのサポート)
	else if (j.contains("offset") && j["offset"].is_array() && j["offset"].size() == 3)
	{
		center.x = j["offset"][0].get<float>();
		center.y = j["offset"][1].get<float>();
		center.z = j["offset"][2].get<float>();
	}

	// "radius" プロパティが存在し、数値であることを確認してから読み取ります。
	if (j.contains("radius"))
	{
		radius = j["radius"].get<float>();
	}

	// "size" プロパティが存在し、配列であることを確認してから読み取ります。(旧プロパティのサポート)
	if (j.contains("size") && j["size"].is_array() && j["size"].size() == 3)
	{
		radius = j["size"][0].get<float>(); // サイズのx成分を半径として使用
	}
}

Vector3 SphereCollider::GetCenter() const
{
	return center;
}

void SphereCollider::SetCenter(const Vector3& newCenter)
{
	if (center != newCenter)
	{
		center = newCenter;
		SetNeedSync(); // 物理エンジンとの状態同期が必要であることを示すフラグを立てる
	}
}

float SphereCollider::GetRadius() const
{
	return radius;
}

void SphereCollider::SetRadius(float newRadius)
{
	if (radius != newRadius)
	{
		radius = newRadius;
		SetNeedSync(); // 物理エンジンとの状態同期が必要であることを示すフラグを立てる
	}
}
