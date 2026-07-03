#include "pch.h"
#include "Engine/Core/Transform.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Core/ObjectManager.h"


// ObjectId から GameObject を取得するユーティリティ関数
static GameObject* FindObject(uint64_t objectId)
{
	return ObjectManager::Find(ObjectId::FromValue(objectId));
}

// Transform クラスのメソッドをスクリプトから呼び出せるようにするためのエクスポート関数

// -------- Transform のプロパティアクセス関数 ---------

// --------- Position ---------

ENGINE_API Vector3 Transform_GetLocalPosition(uint64_t objectId)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			return transform->GetPosition();
		}
	}
	return Vector3::Zero; // オブジェクトやコンポーネントが見つからない場合はゼロベクトルを返す
}

ENGINE_API void Transform_SetLocalPosition(uint64_t objectId, Vector3 position)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			transform->SetPosition(position);
		}
	}
}

ENGINE_API Vector3 Transform_GetPosition(uint64_t objectId)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			return transform->GetWorldPosition();
		}
	}
	return Vector3::Zero; // オブジェクトやコンポーネントが見つからない場合はゼロベクトルを返す
}

ENGINE_API void Transform_SetPosition(uint64_t objectId, Vector3 position)
{
	static_assert(sizeof(Vector3) == (12), "Vector3 size missmatch! C# expects 12 bytes."); // Vector3 と XMFLOAT3 のサイズが同じであることを確認

	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			transform->SetWorldPosition(position);
		}
	}
}

ENGINE_API void Transform_Translate(uint64_t objectId, Vector3 translation)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			transform->Translate(translation);
		}
	}
}

// --------- Rotation ---------

ENGINE_API Quaternion Transform_GetLocalRotation(uint64_t objectId)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			return transform->GetRotation();
		}
	}
	return Quaternion{ 0,0,0,1 }; // オブジェクトやコンポーネントが見つからない場合は単位クォータニオンを返す
}

ENGINE_API void Transform_SetLocalRotation(uint64_t objectId, Quaternion rotation)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			transform->SetRotation(rotation);
		}
	}
}

ENGINE_API Quaternion Transform_GetRotation(uint64_t objectId)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			return transform->GetWorldRotation();
		}
	}
	return Quaternion{ 0,0,0,1 }; // オブジェクトやコンポーネントが見つからない場合は単位クォータニオンを返す
}

ENGINE_API void Transform_SetRotation(uint64_t objectId, Quaternion rotation)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			transform->SetWorldRotation(rotation);
		}
	}
}

ENGINE_API void Transform_Rotate(uint64_t objectId, Vector3 eulers)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			transform->Rotate(eulers);
		}
	}
}

ENGINE_API void Transform_RotateAround(uint64_t objectId, Vector3 point, Vector3 axis, float angle)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			// 回転軸を正規化
			Vector3 normalizedAxis = axis.Normalize();
			// クォータニオンを生成
			Quaternion rotation = Transform::QuaternionRotationAxis(normalizedAxis, angle);
			// オブジェクトの現在の位置を取得
			Vector3 currentPosition = transform->GetWorldPosition();
			// 回転中心からオブジェクトへのベクトルを計算
			Vector3 toObject = currentPosition - point;
			// 回転を適用
			XMVECTOR rotatedVector = XMVector3Rotate(
				XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&toObject)), Transform::QuaternionToXMVector(rotation)); // ベクトルを回転させる。これで回転後のベクトルが得られる。
			Vector3 newPosition;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&newPosition), rotatedVector); // 回転後のベクトルをVector3に変換。これで回転後の位置が得られる。

			// オブジェクトの位置と回転を更新
			transform->SetWorldPosition(newPosition);
			transform->Rotate(rotation);
		}
	}
}


ENGINE_API Vector3 Transform_GetEulerAngles(uint64_t objectId)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			return transform->GetEulerAngles();
		}
	}
	return Vector3::Zero; // オブジェクトやコンポーネントが見つからない場合はゼロベクトルを返す
}

ENGINE_API void Transform_SetEulerAngles(uint64_t objectId, Vector3 eulerAngles)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			transform->SetRotation(eulerAngles);
		}
	}
}


// --------- Scale ---------

ENGINE_API Vector3 Transform_GetLocalScale(uint64_t objectId)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			return transform->GetScale();
		}
	}
	return Vector3(1, 1, 1); // オブジェクトやコンポーネントが見つからない場合はスケール1を返す
}

ENGINE_API void Transform_SetLocalScale(uint64_t objectId, Vector3 scale)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			transform->SetScale(scale);
		}
	}
}

ENGINE_API Vector3 Transform_GetScale(uint64_t objectId)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			return transform->GetWorldScale();
		}
	}
	return Vector3(1, 1, 1); // オブジェクトやコンポーネントが見つからない場合はスケール1を返す
}

ENGINE_API void Transform_SetScale(uint64_t objectId, Vector3 scale)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			transform->SetWorldScale(scale);
		}
	}
}

ENGINE_API void Transform_Scaling(uint64_t objectId, Vector3 scale)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			transform->Scaling(scale);
		}
	}
}

// --------- 親子関係のメソッド ---------

ENGINE_API void Transform_SetParent(uint64_t objectId, uint64_t parentId)
{
	if (GameObject* obj = FindObject(objectId))
	{
		GameObject* parentObj = FindObject(parentId);
		if (parentObj)
		{
			obj->SetParent(parentObj);
		}
	}
}

ENGINE_API uint64_t Transform_GetParent(uint64_t objectId)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (GameObject* parent = obj->GetParent())
		{
			return parent->id.Value();
		}
	}
	return 0; // オブジェクトや親が見つからない場合は0を返す
}


// --------- 子オブジェクトの取得 ---------

ENGINE_API uint64_t Transform_GetChild(uint64_t objectId, int index)
{
	if (GameObject* obj = FindObject(objectId))
	{
		const auto& children = obj->children;
		if (index >= 0 && index < static_cast<int>(children.size()))
		{
			return children[index]->id.Value();
		}
	}
	return 0; // オブジェクトや子が見つからない場合は0を返す
}

ENGINE_API int Transform_GetChildCount(uint64_t objectId)
{
	if (GameObject* obj = FindObject(objectId))
	{
		return static_cast<int>(obj->children.size());
	}
	return 0; // オブジェクトが見つからない場合は0を返す
}


// --------- ルートオブジェクトの取得 ---------


// --------- 座標変換のメソッド ---------


// --------- 行列の取得 ---------

// --------- 方向ベクトルの取得 ---------

ENGINE_API Vector3 Transform_GetForward(uint64_t objectId)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			return transform->GetForward();
		}
	}
	return Vector3::Forward; // オブジェクトやコンポーネントが見つからない場合はワールドの前方を返す
}

ENGINE_API Vector3 Transform_GetUp(uint64_t objectId)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			return transform->GetUp();
		}
	}
	return Vector3::Up; // オブジェクトやコンポーネントが見つからない場合はワールドの上方向を返す
}

ENGINE_API Vector3 Transform_GetRight(uint64_t objectId)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			return transform->GetRight();
		}
	}
	return Vector3::Right; // オブジェクトやコンポーネントが見つからない場合はワールドの右方向を返す
}


// --------- その他のメソッド ---------

ENGINE_API void Transform_LookAt(uint64_t objectId, Vector3 target, Vector3 up)
{
	if (GameObject* obj = FindObject(objectId))
	{
		if (Transform* transform = obj->GetComponent<Transform>())
		{
			XMVECTOR targetVec = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&target));
			XMVECTOR upVec = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&up));
			XMVECTOR currentPos = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&transform->GetWorldPosition()));
			XMVECTOR lookAtRotation = Transform::QuaternionLookAt(currentPos, targetVec);
			transform->SetWorldRotation(Transform::XMVectorToQuaternion(lookAtRotation));
		}
	}
}