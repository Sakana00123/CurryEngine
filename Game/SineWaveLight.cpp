#include "pch.h"
#include "SineWaveLight.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(SineWaveLight, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(SineWaveLight, "UserScripts", ComponentAttributes::None, {})


void SineWaveLight::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	m_timer = 0;	
}

void SineWaveLight::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
		m_timer += deltaTime;

		// サイン波に基づいてライトの明るさを変化させる
		intensity = (std::sin(m_timer) + 1.0f) / 2.0f * intensityAmplitude; // サイン波の値を0から1の範囲に変換して振幅をかける
}