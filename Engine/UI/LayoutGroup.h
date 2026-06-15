#pragma once
#include "Engine/Core/Component.h"
#include "RectTransform.h"

class LayoutGroup : public Component
{
	C_REFLECT(LayoutGroup)
public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

#ifdef USE_IMGUI
	// エディタでプロパティを描画するためのオーバーライド関数
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI


protected:

	// 配置を更新する関数。子要素の位置を計算して配置します。
	virtual void UpdateLayout() {};
	
	// 子要素の RectTransform を取得する関数。
	std::vector<std::shared_ptr<RectTransform>> GetChildRects() const;

	// 所属する GameObject の RectTransform を取得する関数。
	std::shared_ptr<RectTransform> GetRectTransform() const;

	// レイアウトを更新する必要があることを示す関数
	void SetLayoutDirty() { m_layoutDirty = true; }

	bool m_layoutDirty = true; // レイアウトが更新される必要があるかどうかを示すフラグ
private:

	C_PROPERTY()
	float paddingLeft = 0.0f; // 左の余白

	C_PROPERTY()
	float paddingRight = 0.0f; // 右の余白

	C_PROPERTY()
	float paddingTop = 0.0f; // 上の余白

	C_PROPERTY()
	float paddingBottom = 0.0f; // 下の余白

	C_PROPERTY()
	float spacing = 4.0f; // 子要素間のスペース

	C_PROPERTY(CurryEngine::PropertyAttributes::HideInInspector)
	int layoutAlignment = 0; // 配置方法（例: 0=左揃え、1=中央揃え、2=右揃え）



};