#include "pch.h"
#include "Quaternion.h"

const Quaternion Quaternion::Identity = Quaternion(0, 0, 0, 1);

void Quaternion::Normalize()
{
	XMVECTOR v = XMLoadFloat4(this);
	v = XMQuaternionNormalize(v);
	XMStoreFloat4(this, v);
}

Quaternion Quaternion::operator*(const Quaternion& rhs) const
{
	return Quaternion::Multiply(*this, rhs);
}

XMVECTOR Quaternion::ToXMVector() const
{
	return XMLoadFloat4(this);
}

Vector3 Quaternion::ToEuler() const
{
	// クォータニオンからオイラー角（度）への変換
	XMFLOAT4X4 rotationMatrix;
	XMStoreFloat4x4(&rotationMatrix, XMMatrixRotationQuaternion(XMLoadFloat4(this)));
	float sx = rotationMatrix.m[2][1];
	bool unlocked = std::abs(sx) < 0.99999f;
	Vector3 eulerAngles{};
	eulerAngles.x = unlocked ? asinf(sx) : atan2f(rotationMatrix.m[2][1], rotationMatrix.m[2][2]);
	eulerAngles.y = unlocked ? atan2f(-rotationMatrix.m[2][0], rotationMatrix.m[2][2]) : 0;
	eulerAngles.z = unlocked ? atan2f(-rotationMatrix.m[0][1], rotationMatrix.m[1][1]) : atan2f(rotationMatrix.m[1][0], rotationMatrix.m[0][0]);
	eulerAngles.x = -XMConvertToDegrees(eulerAngles.x);
	eulerAngles.y = -XMConvertToDegrees(eulerAngles.y);
	eulerAngles.z = -XMConvertToDegrees(eulerAngles.z);
	return eulerAngles;
}

Vector3 Quaternion::Forward() const
{
	XMVECTOR forward = XMVectorSet(0, 0, 1, 0);
	forward = XMVector3Rotate(forward, XMLoadFloat4(this));
	Vector3 f;
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&f), forward);
	return f;
}

Vector3 Quaternion::Up() const
{
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	up = XMVector3Rotate(up, XMLoadFloat4(this));
	Vector3 u;
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&u), up);
	return u;
}

Vector3 Quaternion::Right() const
{
	XMVECTOR right = XMVectorSet(1, 0, 0, 0);
	right = XMVector3Rotate(right, XMLoadFloat4(this));
	Vector3 r;
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&r), right);
	return r;
}

XMMATRIX Quaternion::ToMatrix() const
{
	return XMMatrixRotationQuaternion(XMLoadFloat4(this));
}

Quaternion Quaternion::Normalized(const Quaternion& q)
{
	XMVECTOR v = XMLoadFloat4(&q);
	v = XMQuaternionNormalize(v);
	Quaternion result;
	XMStoreFloat4(&result, v);
	return result;
}

Quaternion Quaternion::FromEuler(const Vector3& euler)
{
	XMVECTOR v = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(-euler.x), XMConvertToRadians(-euler.y), XMConvertToRadians(-euler.z));
	Quaternion result;
	XMStoreFloat4(&result, v);
	return result;
}

Quaternion Quaternion::LookAt(const Vector3& from, const Vector3& to, const Vector3& up)
{
	XMVECTOR v = XMQuaternionRotationMatrix(XMMatrixLookAtLH(
		XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&from)),
		XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&to)),
		XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&up))));
	Quaternion result;
	XMStoreFloat4(&result, v);
	return result;
}

Quaternion Quaternion::RotationAxis(const Vector3& axis, float angle)
{
	XMVECTOR v = XMQuaternionRotationAxis(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&axis)), XMConvertToRadians(angle));
	Quaternion result;
	XMStoreFloat4(&result, v);
	return result;
}

float Quaternion::ToAxisAngle(const Vector3& axis, const Quaternion& q)
{
	XMVECTOR v = XMLoadFloat4(&q);
	float angle;
	XMVECTOR vAxis = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&axis));
	XMQuaternionToAxisAngle(&vAxis, &angle, v);
	return XMConvertToDegrees(angle);
}

Quaternion Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, float t)
{
	XMVECTOR v1 = XMLoadFloat4(&q1);
	XMVECTOR v2 = XMLoadFloat4(&q2);
	XMVECTOR result = XMQuaternionSlerp(v1, v2, t);
	Quaternion q;
	XMStoreFloat4(&q, result);
	return q;
}

Quaternion Quaternion::Multiply(const Quaternion& q1, const Quaternion& q2)
{
	XMVECTOR v1 = XMLoadFloat4(&q1);
	XMVECTOR v2 = XMLoadFloat4(&q2);
	XMVECTOR result = XMQuaternionMultiply(v1, v2);
	Quaternion q;
	XMStoreFloat4(&q, result);
	return q;
}