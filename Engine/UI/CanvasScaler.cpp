#include "pch.h"
#include "CanvasScaler.h"
#include "RectTransform.h"
#include <algorithm>
#include "Engine/Rendering/Pipeline/Graphics.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
//REGISTER_COMPONENT(CanvasScaler, "UserScripts")
REGISTER_COMPONENT_WITH_ATTRIBUTES(CanvasScaler, "UI", ComponentAttributes::ExecuteInEditMode | ComponentAttributes::DisallowMultiple | ComponentAttributes::RequiredComponent, { "Canvas" })


void CanvasScaler::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	priority = -100; // CanvasScalerは優先度を低くして、他のUIコンポーネントより先に更新されるようにします。
}

void CanvasScaler::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。

	// 解像度取得
	D3D11_VIEWPORT viewport;
	UINT num{ 1 };
	Graphics::GetDeviceContext()->RSGetViewports(&num, &viewport);
	int sw = (int)viewport.Width;
	int sh = (int)viewport.Height;

	// 変化がなければスキップ
	if (sw == m_lastW && sh == m_lastH) return;
	m_lastW = sw;
	m_lastH = sh;

	// スケールモードがConstantPixelSizeなら固定のスケールをセットして処理を終了
	if (scaleMode == ScaleMode::ConstantPixelSize)
	{
		scaleFactor = constantScaleFactor;
		return;
	}

	// Scale With Screen Size - log2ブレンド(Unityと同方式)
	// 線形ブレンドだと倍率の感覚的中間がズレるためlog空間で補間する
	float logW = std::log2(sw / referenceWidth);
	float logH = std::log2(sh / referenceHeight);
	float logScale = std::lerp(logW, logH, matchWidthOrHeight);

	scaleFactor = std::pow(2.f, logScale);

	OutputDebugStringW((L"CanvasScaler: Screen(" + std::to_wstring(sw) + L"x" + std::to_wstring(sh) + L") Ref(" + std::to_wstring(referenceWidth) + L"x" + std::to_wstring(referenceHeight) + L") ScaleFactor(" + std::to_wstring(scaleFactor) + L")\n").c_str());
}