#include "pch.h"
#include "Color.h"
// 定数の定義
const Color Color::Red = Color(1.0f, 0.0f, 0.0f);
const Color Color::Green = Color(0.0f, 1.0f, 0.0f);
const Color Color::Blue = Color(0.0f, 0.0f, 1.0f);
const Color Color::Yellow = Color(1.0f, 1.0f, 0.0f);
const Color Color::Orange = Color(1.0f, 0.5f, 0.0f);
const Color Color::Purple = Color(1.0f, 0.0f, 1.0f);
const Color Color::White = Color(1.0f, 1.0f, 1.0f);
const Color Color::Black = Color(0.0f, 0.0f, 0.0f);

void Color::ConvertToPastelColors(Color& color) {

	color.r = (color.r + 1) * 0.5f;
	color.g = (color.g + 1) * 0.5f;
	color.b = (color.b + 1) * 0.5f;
}
void Color::DarkenColor(Color& color, float factor)
{
	// 各成分を指定した割合で暗くする
	color.r *= factor;
	color.g *= factor;
	color.b *= factor;
	// アルファ値は変更しない
}
void Color::HexToRGB(uint32_t hexColor, Color& color, float alpha)
{
	// 赤成分を抽出し、0～1の範囲に変換
	color.r = static_cast<float>((hexColor >> 16) & 0xFF) / 255.0f;
	// 緑成分を抽出し、0～1の範囲に変換
	color.g = static_cast<float>((hexColor >> 8) & 0xFF) / 255.0f;
	// 青成分を抽出し、0～1の範囲に変換
	color.b = static_cast<float>(hexColor & 0xFF) / 255.0f;
	// アルファ値を設定
	color.a = alpha;
}