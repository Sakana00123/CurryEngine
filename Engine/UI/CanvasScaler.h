#pragma once
#include "Engine/UI/UIComponent.h"

class CanvasScaler : public UIComponent
{
	C_REFLECT(CanvasScaler)
public:
    enum class ScaleMode {
        ConstantPixelSize,      // scaleFactor固定
        ScaleWithScreenSize,    // 解像度比でスケール
    };

    ScaleMode scaleMode = ScaleMode::ScaleWithScreenSize;

    // 設計解像度
	C_PROPERTY()
    float referenceWidth = 1920.f;
	C_PROPERTY()
    float referenceHeight = 1080.f;

    // 0=幅基準, 1=高さ基準, 0.5=ブレンド
    C_PROPERTY(CurryEngine::PropertyAttributes::Range(0.0f, 1.0f))
    float matchWidthOrHeight = 0.5f;

    // ConstantPixelSizeモード用
    float constantScaleFactor = 1.f;

    // 読み取り専用 (毎フレーム更新)
	C_PROPERTY(CurryEngine::PropertyAttributes::NonSerialized)
    float scaleFactor = 1.f;

	// スケールファクターを取得します。
	float GetScaleFactor() const { return scaleFactor; }

	// 設計解像度を取得します。
	Vector2 GetReferenceResolution() const { return { referenceWidth, referenceHeight }; }

	void Start() override;
    void Update(float deltaTime) override;

private:
    int m_lastW = 0;
    int m_lastH = 0;

};