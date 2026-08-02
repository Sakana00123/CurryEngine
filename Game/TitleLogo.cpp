#include "pch.h"
#include "TitleLogo.h"
#include "Engine/UI/Image.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(TitleLogo, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(TitleLogo, "UserScripts", ComponentAttributes::None, {})


void TitleLogo::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	fadeInTimer = 0.0f;
	m_timer = 0;
}

void TitleLogo::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
	// 経過時間を蓄積（メンバ変数として float m_timer を持っておくと便利です）

	m_timer += deltaTime;
	// rect の anchoredPosition をサイン波で上下に動かす
	RectTransform* rect = GetRectTransform();
	if (rect)
	{
		rect->SetAnchoredPosition({ rect->anchoredPosition.x, rect->anchoredPosition.y + std::sin(m_timer * 2.0f) * 0.1f });
	}

	if (Image* image = GetOwner()->GetComponent<Image>())
	{
		//開始時フェードイン

		if (fadeInTimer < 1.0f) {
			fadeInTimer += deltaTime; // フェードインの進行を時間で制御
			float alpha = min(fadeInTimer, 1.0f); // アルファ値は0から1へ変化
			image->color.a = alpha; // 色のアルファ値を更新
		}

		/*		// 色を時間で変化させる(HSV)
				float hue = fmod(m_timer * 60.0f, 360.0f); // 色相を時間で変化させる（例: 60度/秒）
				float saturation = 1.0f; // 彩度は固定

				float value = 1.0f; // 明度は固定

				// HSVからRGBに変換

				float c = value * saturation;
				float x = c * (1 - std::abs(fmod(hue / 60.0f, 2) - 1));
				float m = value - c;

				float r, g, b;
				if (hue < 60) {
					r = c, g = x, b = 0;
				}
				else if (hue < 120) {
					r = x, g = c, b = 0;
				}
				else if (hue < 180) {
					r = 0, g = c, b = x;
				}
				else if (hue < 240) {
					r = 0, g = x, b = c;
				}
				else if (hue < 300) {
					r = x, g = 0, b = c;
				}
				else {
					r = c, g = 0, b = x;
				}

				image->color.r = r + m; // RGB値を更新

				image->color.g = g + m;

				image->color.b = b + m;*/



	}
}