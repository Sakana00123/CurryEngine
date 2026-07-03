#pragma once
#include <DirectXMath.h>
using namespace DirectX;
#include "Engine/Core/Reflection/Meta.h"

//カラー（XMFLOAT4と互換性あり）
C_STRUCT()
struct Color
{
	float r;
	float g;
	float b;
	float a;

	Color(const Color&) = default;
	Color& operator=(const Color&) = default;

	Color(Color&&) = default;
	Color& operator=(Color&&) = default;

	constexpr Color(float r = 1.f, float g = 1.f, float b = 1.f, float a = 1.f) noexcept : r(r), g(g), b(b), a(a) {}
	explicit constexpr Color(const XMFLOAT4& color) : r(color.x), g(color.y), b(color.z), a(color.w) {}

	explicit Color(_In_reads_(4) const float* pArray) noexcept : r(pArray[0]), g(pArray[1]), b(pArray[2]), a(pArray[3]) {}

	operator XMFLOAT4() const { return XMFLOAT4(r, g, b, a); }

	float& operator[](size_t index);
	float operator[](size_t index) const;
	//Color operator=(const XMFLOAT4& color) { return { color.x,color.y,color.z,color.w }; }
	Color& operator+=(const Color& c) { r += c.r, g += c.g, b += c.b, a += c.a; return *this; }
	Color& operator-=(const Color& c) { r -= c.r, g -= c.g, b -= c.b, a -= c.a; return *this; }
	Color& operator*=(const Color& c) { r *= c.r, g *= c.g, b *= c.b, a *= c.a; return *this; }
	Color& operator/=(const Color& c) { r /= c.r, g /= c.g, b /= c.b, a /= c.a; return *this; }
	Color& operator*=(float scale) { r *= scale, g *= scale, b *= scale, a *= scale; return *this; }
	Color& operator/=(float scale) { r /= scale, g /= scale, b /= scale, a /= scale; return *this; }

	Color operator+(const Color& c) const { return { this->r + c.r, this->g + c.g, this->b + c.b, this->a + c.a }; }
	Color operator-(const Color& c) const { return { this->r - c.r, this->g - c.g, this->b - c.b, this->a - c.a }; }
	Color operator*(const Color& c) const { return { this->r * c.r, this->g * c.g, this->b * c.b, this->a * c.a }; }
	Color operator/(const Color& c) const { return { this->r / c.r, this->g / c.g, this->b / c.b, this->a / c.a }; }
	Color operator*(float scale) const { return { this->r * scale, this->g * scale, this->b * scale, this->a * scale }; }
	Color operator/(float scale) const { return { this->r / scale, this->g / scale, this->b / scale, this->a / scale }; }
	bool operator==(Color& c) { return (r == c.r && g == c.g && b == c.b && a == c.a); }
	bool operator!=(Color& c) { return (r != c.r || g != c.g || b != c.b || a != c.a); }
	bool operator==(const Color& c) const { return (r == c.r && g == c.g && b == c.b && a == c.a); }
	bool operator!=(const Color& c) const { return (r != c.r || g != c.g || b != c.b || a != c.a); }

	static const Color Red;
	static const Color Green;
	static const Color Blue;
	static const Color Yellow;
	static const Color Orange;
	static const Color Purple;
	static const Color White;
	static const Color Black;

	static void ConvertToPastelColors(Color& color);
	/**
	 * @brief 16進数カラーコード（0xRRGGBB）をRGB（0〜1）に変換する
	 * @param hexColor 0xRRGGBB形式のカラーコード
	 * @param color 変換後のColor構造体（r, g, b, a）
	 * @param alpha アルファ値（省略時は1.0）
	 */
	static void HexToRGB(uint32_t hexColor, Color& color, float alpha = 1.f);
	/**
	* @brief 色を少し暗くする
	* @param color 対象の色（0〜1の範囲）
	* @param factor 暗くする割合（0.0〜1.0、デフォルトは0.8で20%暗くする）
	*/
	static void DarkenColor(Color& color, float factor = 0.8f);
};
