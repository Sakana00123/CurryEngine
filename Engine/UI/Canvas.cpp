#include "pch.h"
#include "Canvas.h"
#include "Engine/Core/GameObject.h"
#include "Graphic.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

REGISTER_COMPONENT_WITH_ATTRIBUTES(Canvas, "UI", ComponentAttributes::DisallowMultiple | ComponentAttributes::ExecuteInEditMode | ComponentAttributes::RequiredComponent, { "RectTransform" });


void Canvas::Initialize()
{
	float x, y;
	Graphics::GetScreenSize(x, y);
	if (GetRectTransform())
	{
		GetRectTransform()->size = { x,y };
	}
}

void Canvas::Update(float elapsedTime)
{
	// 遅延削除の処理
	if (!erases.empty()) {
		graphics.erase(std::remove_if(graphics.begin(), graphics.end(),
			[&](const auto& graphic) {
				return std::find(erases.begin(), erases.end(), graphic) != erases.end();
			}),
			graphics.end());
		erases.clear();
	}

	// 位置固定（画面全体にフィット）
	Vector2 screenSize{ 1920, 1080 };
	if (RectTransform* rt = GetRectTransform()) {
		rt->SetSize(screenSize);
		rt->SetAnchorMin({ 0,0 });
		rt->SetAnchorMax({ 0,0 });
		rt->SetPivot({ 0,0 });
		rt->SetAnchoredPosition({ 0,0 });
	}
}

std::vector<Graphic*> Canvas::GetGraphics() const {
#if 0
	return this->graphics;
#else
	std::vector<std::pair<Graphic*, int>> graphicWithZ; // Graphic とその描画順をペアで保持
	int parentZ = 0;
	std::function<void(GameObject*)> findGraphic = [&](GameObject* object) {
		auto* rect = object->GetComponent<RectTransform>();
		if (!rect) {
			return;
		}
		// 親のソーティングオーダー + 自身のローカルソーティングオーダーで描画順を決定
		int myZ = parentZ + rect->localSortingOrder;
		parentZ = myZ; // 子の探索に備えて更新

		if (Graphic* graphic = object->GetComponent<Graphic>())
		{
			graphicWithZ.push_back({ graphic, myZ });
		}
		for (GameObject* child : object->children)
		{
			findGraphic(child);
		}
		};
	findGraphic(gameObject);

	// 描画順でソートして Graphic* のリストを作成
	std::vector<Graphic*> graphics;
	std::stable_sort(graphicWithZ.begin(), graphicWithZ.end(), [](const auto& a, const auto& b) {
		return a.second < b.second; // 描画順でソート
		});
	for (const auto& pair : graphicWithZ) {
		graphics.push_back(pair.first);
	}
	return graphics;
#endif
}

#ifdef USE_IMGUI
void Canvas::DrawProperty(const PropertyDrawContext& context)
{
	Component::DrawProperty(context); // 自動生成されたプロパティ描画を呼び出す
	ImGui::Text("GraphicsCount:%d", static_cast<int>(GetGraphics().size()));
}
#endif // USE_IMGUI