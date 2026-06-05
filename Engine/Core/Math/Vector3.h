#pragma once

#include <DirectXMath.h>
using namespace DirectX;

/**
 * @brief 3 次元ベクトルの軽量構造体。
 * @details 代表的な向きの定数や、`XMFLOAT3` との相互変換、基本演算子を提供します。
 */
struct Vector3
{
	static constexpr DirectX::XMFLOAT3 up{ 0,1,0 };
	static constexpr DirectX::XMFLOAT3 down{ 0,-1,0 };
	static constexpr DirectX::XMFLOAT3 right{ 1,0,0 };
	static constexpr DirectX::XMFLOAT3 left{ -1,0,0 };
	static constexpr DirectX::XMFLOAT3 forward{ 0,0,1 };
	static constexpr DirectX::XMFLOAT3 back{ 0,0,-1 };
	static constexpr DirectX::XMFLOAT3 zero{ 0,0,0 };

	/** @brief 2 つの XMFLOAT3 が等しいかを比較します。*/
	static bool Equal(const XMFLOAT3& v0, const XMFLOAT3& v1) { return (v0.x == v1.x && v0.y == v1.y && v0.z == v1.z); }

#if 1
	float x, y, z;
	Vector3(const Vector3&) = default;
	Vector3& operator=(const Vector3&) = default;

	Vector3(Vector3&&) = default;
	Vector3& operator=(Vector3&&) = default;

	constexpr Vector3(float x = 0.f, float y = 0.f, float z = 0.f) noexcept : x(x), y(y), z(z) {}
	explicit constexpr Vector3(const XMFLOAT3& v) : x(v.x), y(v.y), z(v.z) {}

	explicit Vector3(_In_reads_(3) const float* pArray) noexcept : x(pArray[0]), y(pArray[1]), z(pArray[2]) {}

	operator XMFLOAT3() const { return XMFLOAT3(x, y, z); }

	Vector3& operator+=(const Vector3& a) { x += a.x, y += a.y, z += a.z; return *this; }
	Vector3& operator-=(const Vector3& a) { x -= a.x, y -= a.y, z -= a.z; return *this; }
	Vector3& operator*=(const Vector3& a) { x *= a.x, y *= a.y, z *= a.z; return *this; }
	Vector3& operator/=(const Vector3& a) { x /= a.x, y /= a.y, z /= a.z; return *this; }
	Vector3& operator*=(float s) { x *= s, y *= s, z *= s; return *this; }
	Vector3& operator/=(float s) { x /= s, y /= s, z /= s; return *this; }

	Vector3 operator+() const { return *this; }
	Vector3 operator-() const { return Vector3(-x, -y, -z); }
	Vector3 operator+(const Vector3& a) const { return Vector3(this->x + a.x, this->y + a.y, this->z + a.z); }
	Vector3 operator-(const Vector3& a) const { return Vector3(this->x - a.x, this->y - a.y, this->z - a.z); }
	Vector3 operator*(const Vector3& a) const { return Vector3(this->x * a.x, this->y * a.y, this->z * a.z); }
	Vector3 operator/(const Vector3& a) const { return Vector3(this->x / a.x, this->y / a.y, this->z / a.z); }
	Vector3 operator*(float a) const { return Vector3(this->x * a, this->y * a, this->z * a); }
	Vector3 operator/(float a) const { return Vector3(this->x / a, this->y / a, this->z / a); }
	bool operator==(const Vector3& a) const { return (this->x == a.x && this->y == a.y && this->z == a.z); }

	static const Vector3 Up;
	static const Vector3 Down;
	static const Vector3 Right;
	static const Vector3 Left;
	static const Vector3 Forward;
	static const Vector3 Back;
	static const Vector3 Zero;
#endif
	/** @brief ベクトルの長さを返す。*/
	float Length() const;
	/** @brief ベクトルの長さの二乗を返す。*/
	float LengthSq() const;
	/** @brief 正規化したベクトルを返す。*/
	Vector3 Normalize() const;
	/** @brief 内積を計算する。*/
	float Dot(const Vector3& v) const;
	/** @brief 外積を計算する。*/
	Vector3 Cross(const Vector3& v) const;
	/** @brief 2 つのベクトルの内積を計算する。*/
	static float Dot(const Vector3& v1, const Vector3& v2);
	/** @brief 2 つのベクトルの外積を計算する。*/
	static Vector3 Cross(const Vector3& v1, const Vector3& v2);
	/** @brief ベクトルを正規化する。*/
	static Vector3 Normalize(const Vector3& v);
	/** @brief ベクトルの各成分の絶対値を返す。*/
	static Vector3 Abs(const Vector3& v);
	/** @brief 2 つのベクトルの各成分の最小値を返す。*/
	static Vector3 Min(const Vector3& v1, const Vector3& v2);
	/** @brief 2 つのベクトルの各成分の最大値を返す。*/
	static Vector3 Max(const Vector3& v1, const Vector3& v2);
	/** @brief ベクトルを min と max の範囲内にクランプする。*/
	static Vector3 Clamp(const Vector3& v, const Vector3& min, const Vector3& max);
	/** @brief 2 つのベクトルの線形補間を計算する。*/
	static Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);
	/** @brief 2 つのベクトルの距離を計算する。*/
	static float Distance(const Vector3& v1, const Vector3& v2);
	/** @brief 2 つのベクトルがほぼ等しいかを比較する。epsilon は許容される誤差の範囲を指定します。*/
	static bool NearEqual(const Vector3& v0, const Vector3& v1, float epsilon = 1e-5f);
};