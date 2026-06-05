#include "pch.h"
#include "Vector2.h"

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