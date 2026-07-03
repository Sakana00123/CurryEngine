#pragma once
#include "EasingComponent.h"

class EasingAnchoredPosition : public EasingComponent
{
	XMFLOAT2 from;
	XMFLOAT2 to;
	EasingHandler handler;
	bool useUnscaledTime = true;
public:
	void Initialize() override;
	void Update(float deltaTime) override;

#ifdef USE_IMGUI
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI


	/** @brief 開始位置の設定。*/
	void SetFrom(const XMFLOAT2& v) { from = v; }

	/** @brief 終了位置の設定。*/
	void SetTo(const XMFLOAT2& v) { to = v; }

	/** @brief 開始位置の取得。*/
	XMFLOAT2 GetFrom() const { return from; }

	/** @brief 終了位置の取得。*/
	XMFLOAT2 GetTo() const { return to; }

	/** @brief イージングハンドラを設定します。*/
	void SetHandler(const EasingHandler& h) { handler = h; }

	/** @brief 非スケール時間を使用するかを設定します。*/
	void SetUseUnscaledTime(bool use) { useUnscaledTime = use; }

	/** @brief 非スケール時間を使用するかを返します。*/
	bool UsesUnscaledTime() const { return useUnscaledTime; }
};

//class EasingSize : public EasingComponent
//{
//public:
//	void DrawProperty() override;
//};
//
//class EasingAngle : public EasingComponent
//{
//public:
//	void DrawProperty() override;
//};