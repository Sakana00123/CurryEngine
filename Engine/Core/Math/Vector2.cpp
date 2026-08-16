#include "pch.h"
#include "Vector2.h"

// 定数ベクトルの定義
const Vector2 Vector2::Zero{ 0.f, 0.f };
const Vector2 Vector2::One{ 1.f, 1.f };
const Vector2 Vector2::Up{ 0.f, 1.f };
const Vector2 Vector2::Down{ 0.f, -1.f };
const Vector2 Vector2::Right{ 1.f, 0.f };
const Vector2 Vector2::Left{ -1.f, 0.f };

float& Vector2::operator[](size_t index)
{
	assert(index < 2);
	return reinterpret_cast<float*>(this)[index];
}

float Vector2::operator[](size_t index) const
{
	assert(index < 2);
	return reinterpret_cast<const float*>(this)[index];
}

float Vector2::Length() const
{
	return sqrtf(x * x + y * y);
}

Vector2 Vector2::Normalized() const
{
	float length = Length();
	if (length == 0.f) return Vector2(0.f, 0.f);
	return Vector2(x / length, y / length);
}

float Vector2::Dot(const Vector2& other) const
{
	return x * other.x + y * other.y;
}

float Vector2::Cross(const Vector2& other) const
{
	return x * other.y - y * other.x;
}

float Vector2::Distance(const Vector2& other) const
{
	float dx = x - other.x;
	float dy = y - other.y;
	return sqrtf(dx * dx + dy * dy);
}

Vector2 Vector2::Lerp(const Vector2& v0, const Vector2& v1, float t)
{
	return Vector2(v0.x + (v1.x - v0.x) * t, v0.y + (v1.y - v0.y) * t);
}

bool Vector2::Equal(const Vector2& v0, const Vector2& v1)
{
	return (v0.x == v1.x) && (v0.y == v1.y);
}

bool Vector2::NearEqual(const Vector2& v0, const Vector2& v1, float epsilon)
{
	return (fabsf(v0.x - v1.x) <= epsilon) && (fabsf(v0.y - v1.y) <= epsilon);
}
