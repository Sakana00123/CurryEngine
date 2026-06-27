#pragma once
#include <DirectXMath.h>
#include "Vector3.h"

using namespace DirectX;

struct Quaternion : public XMFLOAT4
{
	Quaternion() : XMFLOAT4(0, 0, 0, 1) {}
	Quaternion(float x, float y, float z, float w) : XMFLOAT4(x, y, z, w) {}
	Quaternion(_In_reads_(4) const float* pArray) : XMFLOAT4(pArray) {}
	Quaternion(const Quaternion&) = default;
	Quaternion& operator=(const Quaternion&) = default;
	Quaternion(Quaternion&&) = default;
	Quaternion& operator=(Quaternion&&) = default;
	Quaternion(const XMFLOAT4& q) : XMFLOAT4(q) {}
	// クォータニオンをベクトルから構築
	Quaternion(const XMVECTOR& v)
	{
		XMStoreFloat4(this, v);
	}
	// 単位クォータニオンを返す
	static const Quaternion Identity;

	// クォータニオンの正規化
	void Normalize();

	// クォータニオンの共役を返す
	Quaternion Conjugate() const;

	// クォータニオンの逆数を返す
	Quaternion Inverse() const;

	// クォータニオンの成分にアクセス
	float& operator[](size_t index);

	float operator[](size_t index) const;

	// クォータニオンの乗算
	Quaternion operator*(const Quaternion& rhs) const;

	// クォータニオンのイコール比較
	bool operator==(const Quaternion& rhs) const;
	bool operator!=(const Quaternion& rhs) const;

	// クォータニオンを XMVECTOR に変換
	XMVECTOR ToXMVector() const;

	// クォータニオンをオイラー角（度）に変換
	Vector3 ToEuler() const;


	// クォータニオンを前ベクトルに変換
	Vector3 Forward() const;

	// クォータニオンを右ベクトルに変換
	Vector3 Right() const;

	// クォータニオンを上ベクトルに変換
	Vector3 Up() const;

	// クォータニオンを回転行列に変換
	XMMATRIX ToMatrix() const;

	// クォータニオン同士のイコール比較
	static bool Equal(const Quaternion& q1, const Quaternion& q2);

	// クォータニオン同士のノットイコール比較
	static bool NotEqual(const Quaternion& q1, const Quaternion& q2);

	// クォータニオン同士の近似イコール比較
	static bool NearEqual(const Quaternion& q1, const Quaternion& q2, float epsilon = 1e-4f);

	// クォータニオンの正規化
	static Quaternion Normalized(const Quaternion& q);

	// オイラー角（度）をクォータニオンに変換
	static Quaternion FromEuler(const Vector3& euler);

	// クォータニオンを回転行列に変換
	static Quaternion LookAt(const Vector3& from, const Vector3& to, const Vector3& up = Vector3::Up);

	// クォータニオンを任意軸回りの回転に変換
	static Quaternion RotationAxis(const Vector3& axis, float angle);

	// クォータニオンを指定軸の回転角に変換
	static float ToAxisAngle(const Vector3& axis, const Quaternion& q);

	// クォータニオン同士の線形補間
	static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t);

	// クォータニオン同士の乗算
	static Quaternion Multiply(const Quaternion& q1, const Quaternion& q2);

};