#include "pch.h"
#include "BoxCollider.h"

REGISTER_COMPONENT(BoxCollider, "Physics")

void BoxCollider::Initialize()
{
	// デバッグプリミティブの準備など、必要な初期化処理をここに実装します。
	ID3D11Device* device = Graphics::GetDevice();
	Collider::Initialize();
	primitive->CreateCube(device);
}

void BoxCollider::Register()
{
	Vector3 worldScale = Vector3(GetTransform()->GetWorldScale());

	BoxColliderData data;
	data.halfExtents = Vector3(size) * 0.5f; // 半サイズを指定
	data.center = Vector3(center); // オフセットを中心位置として設定（ワールドスケールは BoxColliderData 内で考慮されるため、ここでは適用しない）
	data.materialHandle = m_materialHandle; // マテリアルハンドルを設定
	data.isTrigger = isTrigger; // トリガーかどうかのフラグを設定
	data.contactOffset = contactOffset; // 接触オフセットを設定
	data.collider = this; // コライダへのポインタを設定（必要に応じて）

	// 物理エンジンにコライダーを登録
	if (!Physics::AddBoxShape(GetTransform(), data, m_shapeHandle))
	{
		// 追加に失敗した場合のエラーハンドリング
		Console::LogError("Failed to add BoxCollider shape to physics engine.");
	}
}

void BoxCollider::FitToBoundingBox(const Vector3& newCenter, const Vector3& newSize)
{
	if (center == newCenter && size == newSize)
	{
		return; // 変更がない場合は何もしない
	}
	center = newCenter;
	size = newSize;
	SetNeedSync(); // サイズやオフセットが変更されたことを通知して、物理エンジンに同期する必要があることを示します。
}

void BoxCollider::SyncWithPhysics()
{
	// 物理エンジンにローカルポーズを更新
	Vector3 position = Vector3(center); // オフセットをワールドスケールで調整してローカル座標に変換
	Quaternion rotation = Quaternion(0, 0, 0, 1);
	Physics::SetLocalPose(m_shapeHandle, position, rotation);
	// サイズの変更も反映
	physx::PxBoxGeometry geometry(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f); // 半サイズを指定
	if (!geometry.isValid())
	{
		Console::LogError("Invalid box geometry for BoxCollider. Size must be positive.");
		return;
	}
	Physics::SetGeometry(m_shapeHandle, geometry);
}

void BoxCollider::Render(RenderContext* rtx)
{
	// デバッグ描画用のワイヤーフレームボックスを描画(いずれはデバッグ描画用の専用クラスを作るべきかもしれません)
#ifdef _DEBUG
	ID3D11DeviceContext* immediateContext = rtx->immediateContext;
	// ワールド行列を計算
	XMFLOAT4X4 world = CalculateColliderWorldTransform(center, size);

	// ワイヤーフレームで描画
	RenderState* renderState = Graphics::GetRenderState();
	renderState->BindRasterizerState(immediateContext, RasterizerState::WireCullBack);
	primitive->Render(immediateContext, world, color);
	renderState->BindRasterizerState(immediateContext, RasterizerState::SolidCullNone);
#else
	RenderState* renderState = Graphics::GetRenderState();
	renderState->BindRasterizerState(rtx->immediateContext, RasterizerState::SolidCullNone);
#endif // DEBUG
}


#ifdef USE_IMGUI
//void BoxCollider::DrawProperty(const PropertyDrawContext& context)
//{
//	IMGUI_PROPERTY_BEGIN();
//	Collider::DrawProperty(context);
//
//	bool isChanged = false;
//
//
//	//isChanged |= ImGui::DragFloat3("Center", &center.x);
//	//isChanged |= ImGui::DragFloat3("Size", &size.x);
//	IMGUI_PROPERTY_VECTOR3("Center", center, isChanged);
//	IMGUI_PROPERTY_VECTOR3("Size", size, isChanged);
//
//	// サイズやオフセットが変更された場合は、物理エンジンに同期する必要があるため、フラグをセットします。
//	if (isChanged)
//	{
//		SetNeedSync();
//	}
//	IMGUI_PROPERTY_END();
//}
#endif // USE_IMGUI

json BoxCollider::Serialize() const
{
	json j = Collider::Serialize();
	j["center"] = { center.x, center.y, center.z };
	j["size"] = { size.x, size.y, size.z };
	return j;
}

void BoxCollider::Deserialize(const json& j)
{
	Collider::Deserialize(j);
	// "center" プロパティが存在し、配列であることを確認してから読み取ります。
	if (j.contains("center") && j["center"].is_array() && j["center"].size() == 3)
	{
		center = Vector3(j["center"][0].get<float>(), j["center"][1].get<float>(), j["center"][2].get<float>());
	}
	// "offset" プロパティが存在し、配列であることを確認してから読み取ります。(旧プロパティのサポート)
	if (j.contains("offset") && j["offset"].is_array() && j["offset"].size() == 3)
	{
		center = Vector3(j["offset"][0].get<float>(), j["offset"][1].get<float>(), j["offset"][2].get<float>());
	}

	// "size" プロパティが存在し、配列であることを確認してから読み取ります。
	if (j.contains("size") && j["size"].is_array() && j["size"].size() == 3)
	{
		size = Vector3(j["size"][0].get<float>(), j["size"][1].get<float>(), j["size"][2].get<float>());
	}
}

Vector3 BoxCollider::GetCenter() const
{
	return center;
}

void BoxCollider::SetCenter(const Vector3& newCenter)
{
	if (center == newCenter)
	{
		return; // 変更がない場合は何もしない
	}
	center = newCenter;
	SetNeedSync(); // オフセットが変更されたことを通知して、物理エンジンに同期する必要があることを示します。
}

Vector3 BoxCollider::GetSize() const
{
	return size;
}

void BoxCollider::SetSize(const Vector3& newSize)
{
	if (size == newSize)
	{
		return; // 変更がない場合は何もしない
	}
	size = newSize;
	SetNeedSync(); // サイズが変更されたことを通知して、物理エンジンに同期する必要があることを示します。
}