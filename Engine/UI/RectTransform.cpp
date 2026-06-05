#include "pch.h"
#include "RectTransform.h"
#include "Engine/Utils/RectTransformUtils.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Canvas.h"

REGISTER_COMPONENT_WITH_ATTRIBUTES(RectTransform, "UI", ComponentAttributes::DisallowMultiple | ComponentAttributes::HideInAddComponentMenu | ComponentAttributes::ExecuteInEditMode, {});

void RectTransform::Start()
{
	// 生成直後の1フレームはUpdateが呼ばれないため、ここでUpdateを呼び出して初期値を計算する
	Update(0.0f);
}

RectTransform* RectTransform::GetParent() const 
{
	//親がいなかったらnullを返す
	if (!gameObject->parent)
		return nullptr;
	//親オブジェクトのTransformから、RectTransform型にキャストする
	return dynamic_cast<RectTransform*>(gameObject->parent->transform);
}

void RectTransform::Update(float elapsedTime)
{

	//中心座標とサイズ計算
	if (RectTransform* p = GetParent()) {

		XMVECTOR AnchorMin = XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(&anchorMin));
		XMVECTOR AnchorMax = XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(&anchorMax));

		XMVECTOR anchorRectMin = XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(&p->unrotatedTopLeft));
		XMVECTOR anchorRectMax = XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(&p->unrotatedBottomRight));

		XMVECTOR worldSizeVec;
		XMVECTOR worldPosVec;

		if (!XMVector2Equal(AnchorMin, AnchorMax)) {
			// ストレッチ（anchorMin ≠ anchorMax）
			XMVECTOR rectMin = anchorRectMin + XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(&offsetMin));
			XMVECTOR rectMax = anchorRectMax + XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(&offsetMax));
			worldSizeVec = rectMax - rectMin;
			worldPosVec = rectMin;
		}
		else {
			// 固定アンカー（anchorMin == anchorMax）
			worldSizeVec = XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(&size));
			XMVECTOR anchorPos = anchorRectMin + (anchorRectMax - anchorRectMin) * AnchorMin; // AnchorMin == AnchorMax

			worldPosVec = anchorPos + XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(&anchoredPosition));
		}

		XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&worldSize), worldSizeVec);
		XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&worldPos), worldPosVec);
		worldAngle = angle + p->worldAngle;
	}
	else {
		worldSize = size;
		worldPos = anchoredPosition;
		worldAngle = angle;
	}

	//４点の座標を求める
	{
		Vector2 min = { worldPos.x - worldSize.x * pivot.x, worldPos.y - worldSize.y * pivot.y };
		Vector2 max = { worldPos.x + worldSize.x * (1.f - pivot.x), worldPos.y + worldSize.y * (1.f - pivot.y) };
		unrotatedTopLeft = topLeft = { min };
		unrotatedTopRight = topRight = { max.x, min.y };
		unrotatedBottomLeft = bottomLeft = { min.x, max.y };
		unrotatedBottomRight = bottomRight = { max };

		Rotate(topLeft, worldPos, worldAngle);
		Rotate(topRight, worldPos, worldAngle);
		Rotate(bottomLeft, worldPos, worldAngle);
		Rotate(bottomRight, worldPos, worldAngle);

	}
}

void RectTransform::DrawProperty()
{
#ifdef USE_IMGUI

	if (isOpen) {
		isOpen = !(ImGui::Button("Cancel") || DrawAnchorPreset());
	}
	else {
		isOpen = ImGui::Button("AnchorPreset");
	}

	if (anchorMin.x != anchorMax.x || anchorMin.y != anchorMax.y) {
		float left = GetLeft();
		if (ImGui::DragFloat("Left", &left)) {
			SetLeft(left);
		}
		float right = GetRight();
		if (ImGui::DragFloat("Right", &right)) {
			SetRight(right);
		}
		float up = GetTop();
		if (ImGui::DragFloat("Up", &up)) {
			SetTop(up);
		}
		float down = GetBottom();
		if (ImGui::DragFloat("Down", &down)) {
			SetBottom(down);
		}
	}
	else {
		ImGui::DragFloat2("Pos", &anchoredPosition.x);
		ImGui::DragFloat2("Size", &size.x);
	}

	ImGui::BeginDisabled();
	ImGui::DragFloat2("WorldPos", &worldPos.x);
	ImGui::DragFloat2("WorldSize", &worldSize.x);
	ImGui::EndDisabled();

	if (ImGui::TreeNodeEx("Anchor", ImGuiTreeNodeFlags_DefaultOpen)) {
		Vector2 min = anchorMin;
		Vector2 max = anchorMax;
		if (ImGui::DragFloat2("Min", &min.x, 0.1f, 0.0f, 1.0f) || ImGui::DragFloat2("Max", &max.x, 0.1f, 0.0f, 1.0f)) {
			RectTransformUtils::SetAnchorAndPivotWithoutAffectingPosition(
				this, min, max, pivot
			);
		}
		ImGui::TreePop();
	}
	Vector2 newPivot = pivot;
	if (ImGui::DragFloat2("Pivot", &newPivot.x, 0.1f)) {
		SetPivot(newPivot);
	}
	ImGui::DragFloat("Angle", &angle);

	ImGui::BeginDisabled();
	float worldAngle = GetWorldAngle();
	ImGui::DragFloat("WorldAngle", &worldAngle);
	ImGui::EndDisabled();

	// Sorting Order は Canvas の Sorting Order + localSortingOrder で決まるため、localSortingOrder のみを編集可能にする
	
	ImGui::DragInt("SortingOrder", &localSortingOrder);

#endif // USE_IMGUI
}

bool RectTransform::DrawAnchorPreset()
{
#ifdef USE_IMGUI
	bool isChanged = false;
	/*const char* labels[4][3] = {
	{ "TL", "TC", "TR" },
	{ "CL", "CC", "CR" },
	{ "BL", "BC", "BR" },
	{ "HS", "VS", "FS" }
	};

	ImVec2 anchorPositions[3] = { ImVec2(0, 0), ImVec2(0.5f, 0.5f), ImVec2(1, 1) };

	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			ImGui::PushID(y * 3 + x);
			if (ImGui::Button(labels[y][x], ImVec2(30, 30))) {
				anchorMin = anchorMax = Vector2(
					anchorPositions[x].x,
					anchorPositions[y].y
				);
				isChanged = true;
			}
			ImGui::PopID();
			if (x < 2) ImGui::SameLine();
		}
	}*/
	struct Preset {
		const char* label;
		Vector2 min;
		Vector2 max;
	};

	// 9-point anchors
	Preset presets[12] = {
		{ "TL", {0,0}, {0,0} },
		{ "TC", {0.5f,0}, {0.5f,0} },
		{ "TR", {1,0}, {1,0} },
		{ "CL", {0,0.5f}, {0,0.5f} },
		{ "CC", {0.5f,0.5f}, {0.5f,0.5f} },
		{ "CR", {1,0.5f}, {1,0.5f} },
		{ "BL", {0,1}, {0,1} },
		{ "BC", {0.5f,1}, {0.5f,1} },
		{ "BR", {1,1}, {1,1} },
		// Stretch presets
		{ "HS", {0, 0.5f}, {1, 0.5f} }, // Horizontal Stretch
		{ "VS", {0.5f, 0}, {0.5f, 1} }, // Vertical Stretch
		{ "ST", {0, 0}, {1, 1} },       // Stretch Full
	};

	// 3x3 grid + stretch row
	for (int i = 0; i < 12; ++i) {
		ImGui::PushID(i);
		if (ImGui::Button(presets[i].label, ImVec2(30, 30))) {

			RectTransformUtils::SetAnchorAndPivotWithoutAffectingPosition(
				this,
				presets[i].min,
				presets[i].max,
				pivot
			);
			isChanged = true;
		}
		ImGui::PopID();

		// 行ごとに改行
		if (i % 3 == 2) {
			ImGui::NewLine();
		}
		else {
			ImGui::SameLine();
		}
	}
	return isChanged;
#endif // USE_IMGUI
}


bool RectTransform::Contains(const Vector2& point)
{
	//角度つき対応
	if (worldAngle != 0)
		return PointInQuad(point, TopLeft(), TopRight(), BottomRight(), BottomLeft());


	Vector2 min = TopLeft();
	Vector2 max = BottomRight();
	return (min.x <= point.x && point.x <= max.x &&
		min.y <= point.y && point.y <= max.y);
}


bool RectTransform::PointInQuad(Vector2 p, Vector2 a, Vector2 b, Vector2 c, Vector2 d)
{
	auto Cross = [](Vector2 u, Vector2 v) {
		return u.x * v.y - u.y * v.x;
		};
	auto Sub = [](Vector2 u, Vector2 v) {
		return Vector2{ u.x - v.x, u.y - v.y };
		};
	// 点pが abcd で構成される四角形内にあるかを判定
	bool ab = Cross(Sub(b, a), Sub(p, a)) >= 0;
	bool bc = Cross(Sub(c, b), Sub(p, b)) >= 0;
	bool cd = Cross(Sub(d, c), Sub(p, c)) >= 0;
	bool da = Cross(Sub(a, d), Sub(p, d)) >= 0;
	return ab && bc && cd && da;
}

Vector2 RectTransform::ToNDC() const
{
	D3D11_VIEWPORT viewport;
	UINT num{ 1 };
	Graphics::GetDeviceContext()->RSGetViewports(&num, &viewport);
	Vector2 ndc{};
	ndc.x = 2.0f * anchoredPosition.x / viewport.Width - 1.0f;
	ndc.y = 1.0f - 2.0f * anchoredPosition.y / viewport.Height;
	return ndc;
}
Vector2 RectTransform::ScreenToNDC(const Vector2& anchoredPosition)
{
	D3D11_VIEWPORT viewport;
	UINT num{ 1 };
	Graphics::GetDeviceContext()->RSGetViewports(&num, &viewport);
	Vector2 ndc{};
	ndc.x = 2.0f * anchoredPosition.x / viewport.Width - 1.0f;
	ndc.y = 1.0f - 2.0f * anchoredPosition.y / viewport.Height;
	return ndc;
}
Vector2 RectTransform::NDCToScreen(const Vector2& ndc)
{
	D3D11_VIEWPORT viewport;
	UINT num{ 1 };
	Graphics::GetDeviceContext()->RSGetViewports(&num, &viewport);
	Vector2 screen{};
	screen.x = viewport.Width * (ndc.x + 1.0f) * 0.5f;
	screen.y = viewport.Height * (1.0f - ndc.y) * 0.5f;
	return screen;
}

Vector2 RectTransform::GetWorldPosition()
{
	Update(0.0f);
	return worldPos;
}
Vector2 RectTransform::GetAnchoredPosition() const
{
	return anchoredPosition;
}

void RectTransform::SetAnchoredPositionByAnchor(const Vector2& targetAnchor, const Vector2& targetAnchoredPos)
{
	RectTransform* parentRect = GetParent();
	if (!parentRect) return;

	Vector2 parentSize = Vector2(parentRect->GetWorldSize());
	Vector2 parentPos = Vector2(parentRect->GetWorldPosition());

	// ① 新しい基準アンカーでのワールド座標を算出
	Vector2 newWorldPos = parentPos
		+ parentSize * Vector2(targetAnchor)     // 親のアンカー基準位置
		+ Vector2(targetAnchoredPos);            // そのアンカーからのオフセット

	// ② 現在のアンカー設定で、同じワールド位置になるanchoredPositionを再計算
	Vector2 myAnchorCenter = (Vector2(anchorMin) + Vector2(anchorMax)) * 0.5f;
	Vector2 myPivotOffset = Vector2(size) * (Vector2(pivot) - Vector2(0.5f, 0.5f));
	anchoredPosition = newWorldPos - (parentPos + parentSize * myAnchorCenter) - myPivotOffset;
}


void RectTransform::SetAnchoredPositionByTransform(Transform* transform)
{
	// 3D Transformの位置に基づいて、RectTransformのアンカーポジションを設定
	RectTransformUtils::SetAnchordPositionBy3DTransform(this, transform);
}


Vector2 RectTransform::GetWorldSize()
{
	Update(0.0f);
	return worldSize;
}

float RectTransform::GetWorldAngle() const
{
	return worldAngle;
}

void RectTransform::Rotate(Vector2& point, Vector2 center, float angle)
{
	point.x -= center.x;
	point.y -= center.y;

	float cos{ cosf(XMConvertToRadians(angle)) };
	float sin{ sinf(XMConvertToRadians(angle)) };
	float tx{ point.x }, ty{ point.y };
	point.x = cos * tx + -sin * ty;
	point.y = sin * tx + cos * ty;

	point.x += center.x;
	point.y += center.y;
}