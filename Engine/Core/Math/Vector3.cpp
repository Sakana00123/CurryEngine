#include "pch.h"
#include "Vector3.h"


// 定数ベクトルの定義
const Vector3 Vector3::Up{ 0,1,0 };
const Vector3 Vector3::Down{ 0,-1,0 };
const Vector3 Vector3::Right{ 1,0,0 };
const Vector3 Vector3::Left{ -1,0,0 };
const Vector3 Vector3::Forward{ 0,0,1 };
const Vector3 Vector3::Back{ 0,0,-1 };
const Vector3 Vector3::Zero{ 0,0,0 };


float Vector3::Length() const
{
	XMVECTOR v = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this));
	XMVECTOR length = XMVector3Length(v);
	return XMVectorGetX(length);
}

float Vector3::LengthSq() const
{
	XMVECTOR v = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this));
	XMVECTOR lengthSq = XMVector3LengthSq(v);
	return XMVectorGetX(lengthSq);
}

Vector3 Vector3::Normalize() const
{
	XMVECTOR v = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this));
	v = XMVector3Normalize(v);
	XMFLOAT3 result;
	XMStoreFloat3(&result, v);
	return Vector3(result);
}

float Vector3::Dot(const Vector3& v) const
{
	XMVECTOR v1 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this));
	XMVECTOR v2 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v));
	XMVECTOR dot = XMVector3Dot(v1, v2);
	return XMVectorGetX(dot);
}

Vector3 Vector3::Cross(const Vector3& v) const
{
	XMVECTOR v1 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this));
	XMVECTOR v2 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v));
	XMVECTOR cross = XMVector3Cross(v1, v2);
	XMFLOAT3 result;
	XMStoreFloat3(&result, cross);
	return Vector3(result);
}

float Vector3::Dot(const Vector3& v1, const Vector3& v2)
{
	XMVECTOR xmv1 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v1));
	XMVECTOR xmv2 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v2));
	XMVECTOR dot = XMVector3Dot(xmv1, xmv2);
	return XMVectorGetX(dot);
}

Vector3 Vector3::Cross(const Vector3& v1, const Vector3& v2)
{
	XMVECTOR xmv1 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v1));
	XMVECTOR xmv2 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v2));
	XMVECTOR cross = XMVector3Cross(xmv1, xmv2);
	XMFLOAT3 result;
	XMStoreFloat3(&result, cross);
	return Vector3(result);
}

Vector3 Vector3::Normalize(const Vector3& v)
{
	XMVECTOR xmv = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v));
	xmv = XMVector3Normalize(xmv);
	XMFLOAT3 result;
	XMStoreFloat3(&result, xmv);
	return Vector3(result);
}

Vector3 Vector3::Abs(const Vector3& v)
{
	XMVECTOR xmv = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v));
	xmv = XMVectorAbs(xmv);
	XMFLOAT3 result;
	XMStoreFloat3(&result, xmv);
	return Vector3(result);
}

Vector3 Vector3::Min(const Vector3& v1, const Vector3& v2)
{
	XMVECTOR xmv1 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v1));
	XMVECTOR xmv2 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v2));
	XMVECTOR min = XMVectorMin(xmv1, xmv2);
	XMFLOAT3 result;
	XMStoreFloat3(&result, min);
	return Vector3(result);
}

Vector3 Vector3::Max(const Vector3& v1, const Vector3& v2)
{
	XMVECTOR xmv1 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v1));
	XMVECTOR xmv2 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v2));
	XMVECTOR max = XMVectorMax(xmv1, xmv2);
	XMFLOAT3 result;
	XMStoreFloat3(&result, max);
	return Vector3(result);
}

Vector3 Vector3::Clamp(const Vector3& v, const Vector3& min, const Vector3& max)
{
	XMVECTOR xmv = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v));
	XMVECTOR xmvMin = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&min));
	XMVECTOR xmvMax = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&max));
	XMVECTOR clamped = XMVectorClamp(xmv, xmvMin, xmvMax);
	XMFLOAT3 result;
	XMStoreFloat3(&result, clamped);
	return Vector3(result);
}

Vector3 Vector3::Lerp(const Vector3& v1, const Vector3& v2, float t)
{
	XMVECTOR xmv1 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v1));
	XMVECTOR xmv2 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v2));
	XMVECTOR lerp = XMVectorLerp(xmv1, xmv2, t);
	XMFLOAT3 result;
	XMStoreFloat3(&result, lerp);
	return Vector3(result);
}

float Vector3::Distance(const Vector3& v1, const Vector3& v2)
{
	XMVECTOR xmv1 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v1));
	XMVECTOR xmv2 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v2));
	XMVECTOR distance = XMVector3Length(xmv2 - xmv1);
	return XMVectorGetX(distance);
}

bool Vector3::NearEqual(const Vector3& v1, const Vector3& v2, float epsilon)
{
	XMVECTOR xmv1 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v1));
	XMVECTOR xmv2 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v2));
	XMVECTOR delta = XMVectorAbs(xmv1 - xmv2);
	XMVECTOR epsilonVec = XMVectorReplicate(epsilon);
	XMVECTOR nearEqual = XMVectorLessOrEqual(delta, epsilonVec);
	return XMVectorGetX(nearEqual) != 0;
}