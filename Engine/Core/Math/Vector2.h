#pragma once

#include <DirectXMath.h>
using namespace DirectX;

/**
 * @brief 2 次元ベクトルの軽量構造体。
 * @details `XMFLOAT2` との相互変換や基本演算子を提供します。
 */
struct Vector2
{
	float x, y;
	Vector2(const Vector2&) = default;
	Vector2& operator=(const Vector2&) = default;

	Vector2(Vector2&&) = default;
	Vector2& operator=(Vector2&&) = default;

	constexpr Vector2(float x = 0.f, float y = 0.f) noexcept : x(x), y(y) {}
	explicit constexpr Vector2(const XMFLOAT2& v) : x(v.x), y(v.y) {}

	explicit Vector2(_In_reads_(2) const float* pArray) noexcept : x(pArray[0]), y(pArray[1]) {}

	operator XMFLOAT2() const { return XMFLOAT2(x, y); }

	float& operator[](size_t index);
	float operator[](size_t index) const;
	Vector2& operator+=(const Vector2& a) { x += a.x, y += a.y; return *this; }
	Vector2& operator-=(const Vector2& a) { x -= a.x, y -= a.y; return *this; }
	Vector2& operator*=(const Vector2& a) { x *= a.x, y *= a.y; return *this; }
	Vector2& operator/=(const Vector2& a) { x /= a.x, y /= a.y; return *this; }
	Vector2& operator*=(float s) { x *= s, y *= s; return *this; }
	Vector2& operator/=(float s) { x /= s, y /= s; return *this; }

	Vector2 operator+(const Vector2& a) const { return Vector2(this->x + a.x, this->y + a.y); }
	Vector2 operator-(const Vector2& a) const { return Vector2(this->x - a.x, this->y - a.y); }
	Vector2 operator*(const Vector2& a) const { return Vector2(this->x * a.x, this->y * a.y); }
	Vector2 operator/(const Vector2& a) const { return Vector2(this->x / a.x, this->y / a.y); }
	Vector2 operator*(float a) const { return Vector2(this->x * a, this->y * a); }
	Vector2 operator/(float a) const { return Vector2(this->x / a, this->y / a); }
	bool operator==(const Vector2& a) const { return (this->x == a.x && this->y == a.y); }

	/* ベクトルの長さを計算して返します。*/
	float Length() const;

	/* ベクトルを正規化して返します。長さが 0 の場合はゼロベクトルを返します。*/
	Vector2 Normalized() const;

	/* ベクトルの内積を計算して返します。*/
	float Dot(const Vector2& other) const;

	/* ベクトルの外積を計算して返します。2D ベクトルの外積はスカラー値になります。*/
	float Cross(const Vector2& other) const;

	/* ベクトルの距離を計算して返します。*/
	float Distance(const Vector2& other) const;

	/* ベクトルを線形補間して返します。t は 0 から 1 の範囲で、0 の場合はこのベクトル、1 の場合は target ベクトルになります。*/
	static Vector2 Lerp(const Vector2& v0, const Vector2& v1, float t);

	/* 2 つのベクトルが完全に等しいかを比較します。*/
	static bool Equal(const Vector2& v0, const Vector2& v1);

	/* 2 つのベクトルがほぼ等しいかを比較します。epsilon は許容される誤差の範囲を指定します。*/
	static bool NearEqual(const Vector2& v0, const Vector2& v1, float epsilon = 1e-5f);

};