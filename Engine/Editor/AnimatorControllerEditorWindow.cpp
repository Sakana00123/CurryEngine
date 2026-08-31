#include "pch.h"
#include "AnimatorControllerEditorWindow.h"
#include "Engine/Animation/AnimatorController.h"
#include <Engine\Resources\AssetMeta.h>
#include <Engine\Resources\AssetDatabase.h>

#ifdef USE_IMGUI

namespace CurryEngine::Editor
{
    AnimatorControllerEditorWindow::AnimatorControllerEditorWindow(std::shared_ptr<AnimatorController> controller)
    {
        if (controller)
        {
            // 初期化時にAnimatorControllerが指定されている場合、相互遷移のインデックスを更新
            UpdateTransitionOffsetIndices(controller);
        }
	}

    ImVec2 AnimatorControllerEditorWindow::WorldToScreen(const Vector2& worldPos, const ImVec2& canvasOrigin) const
    {
        return ImVec2(
            canvasOrigin.x + (worldPos.x + panOffset.x) * zoom,
            canvasOrigin.y + (worldPos.y + panOffset.y) * zoom
        );
    }

    Vector2 AnimatorControllerEditorWindow::ScreenToWorld(const ImVec2& screenPos, const ImVec2& canvasOrigin) const
    {
        return Vector2(
            (screenPos.x - canvasOrigin.x) / zoom - panOffset.x,
            (screenPos.y - canvasOrigin.y) / zoom - panOffset.y
        );
    }

    // ノード矩形の境界とセンター間直線の交点(矢印の始点/終点をノードの縁にクリップする)
    ImVec2 AnimatorControllerEditorWindow::GetNodeEdgePoint(const ImVec2& fromCenter, const ImVec2& toCenter, const ImVec2& nodeSize) const
    {
        ImVec2 dir(toCenter.x - fromCenter.x, toCenter.y - fromCenter.y);
        if (dir.x == 0.0f && dir.y == 0.0f) return fromCenter;

        float halfW = nodeSize.x * 0.5f;
        float halfH = nodeSize.y * 0.5f;
        float tx = (dir.x != 0.0f) ? halfW / std::fabs(dir.x) : FLT_MAX;
        float ty = (dir.y != 0.0f) ? halfH / std::fabs(dir.y) : FLT_MAX;
        float t = (std::min)(tx, ty);

        return ImVec2(fromCenter.x + dir.x * t, fromCenter.y + dir.y * t);
    }

    ImVec2 AnimatorControllerEditorWindow::GetOffsetNodeEdgePoint(const ImVec2& fromCenter, const ImVec2& toCenter, const ImVec2& nodeSize, float offset) const
    {
        ImVec2 dir(toCenter.x - fromCenter.x, toCenter.y - fromCenter.y);
		if (dir.x == 0.0f && dir.y == 0.0f) return fromCenter + ImVec2(offset, offset);

		// オフセット方向を計算するために、dirを正規化して垂直方向のベクトルを求める
		float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
		ImVec2 dirNormalized(dir.x / length, dir.y / length); // dirの正規化
		ImVec2 normal(-dirNormalized.y, dirNormalized.x); // 垂直方向の単位ベクトル
		// オフセットを加えた点を計算
		ImVec2 offsetPoint = fromCenter + normal * offset;

		float left = fromCenter.x - nodeSize.x * 0.5f;
		float right = fromCenter.x + nodeSize.x * 0.5f;
		float top = fromCenter.y - nodeSize.y * 0.5f;
		float bottom = fromCenter.y + nodeSize.y * 0.5f;

		float bestT = FLT_MAX;
		ImVec2 bestPoint = fromCenter;

        auto checkVerticalEdge = [&](float edgeX) {
			if (dir.x == 0.0f) return; // 垂直線は交差しない
            float t = (edgeX - offsetPoint.x) / dir.x;
			if (t < 0.0f) return; // 交点が線分の範囲外

			float y = offsetPoint.y + dir.y * t;
            if (y >= top && y <= bottom) {
				if (t < bestT) {
                    bestT = t;
                    bestPoint = ImVec2(edgeX, y);
                }
            }
			};
		auto checkHorizontalEdge = [&](float edgeY) {
            if (dir.y == 0.0f) return; // 水平線は交差しない
            float t = (edgeY - offsetPoint.y) / dir.y;
            if (t < 0.0f) return; // 交点が線分の範囲外
            float x = offsetPoint.x + dir.x * t;
            if (x >= left && x <= right) {
                if (t < bestT) {
                    bestT = t;
                    bestPoint = ImVec2(x, edgeY);
                }
            }
			};

		checkVerticalEdge(left);
		checkVerticalEdge(right);
		checkHorizontalEdge(top);
		checkHorizontalEdge(bottom);

		return bestPoint;
	}

    // 遷移ラインのクリック判定用(ImGuiにはライン用InvisibleButtonが無いため自前で距離計算)
    float AnimatorControllerEditorWindow::DistancePointToSegment(const ImVec2& p, const ImVec2& a, const ImVec2& b) const
    {
        ImVec2 ab(b.x - a.x, b.y - a.y);
        float lenSq = ab.x * ab.x + ab.y * ab.y;
        float t = (lenSq > 0.0f) ? std::clamp(((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / lenSq, 0.0f, 1.0f) : 0.0f;
        ImVec2 proj(a.x + ab.x * t, a.y + ab.y * t);
        float dx = p.x - proj.x, dy = p.y - proj.y;
        return std::sqrt(dx * dx + dy * dy);
    }

	// 遷移ラインをずらすために、相互遷移があるtransitionのインデックスを計算して保持する
    void AnimatorControllerEditorWindow::UpdateTransitionOffsetIndices(std::shared_ptr<AnimatorController>& controller)
    {
		// 相互遷移のペアをキーとして、出現回数をカウントするマップを作成
		using TransitionPair = std::pair<int, int>; // <stateIndexA, stateIndexB> (小さい方のインデックスを先にする)
        std::map<TransitionPair, int> transitionCountMap;
        for (size_t i = 0; i < controller->transitions.size(); ++i)
        {
            int sourceIndex = controller->transitions[i].fromStateIndex;
			int targetIndex = controller->transitions[i].toStateIndex;
            if (sourceIndex < 0 || targetIndex < 0) continue;
			TransitionPair pair = TransitionPair((std::min)(sourceIndex, targetIndex), (std::max)(sourceIndex, targetIndex));
			transitionCountMap[pair]++;
            if (transitionCountMap[pair] > 1)
            {
				// 相互遷移がある場合、ずらす必要があるのでインデックスを保持
                transitionOffsetIndices.push_back(i);
            }
        }
	}

    void AnimatorControllerEditorWindow::DrawGrid(ImDrawList* drawList, const ImVec2& canvasOrigin, const ImVec2& canvasSize) const
    {
        drawList->AddRectFilled(canvasOrigin, ImVec2(canvasOrigin.x + canvasSize.x, canvasOrigin.y + canvasSize.y), IM_COL32(45, 45, 48, 255));

        const float baseGridStep = 32.0f;
        float gridStep = baseGridStep * zoom;
        while (gridStep < 8.0f) gridStep *= 2.0f; // ズームアウト時にグリッドが潰れないよう間引く

        // WorldToScreen = origin + (world + pan) * zoom なので、
        // world原点の格子線が画面上でどこに来るかは pan*zoom を gridStep で mod すれば求まる
        float offsetX = fmodf(panOffset.x * zoom, gridStep);
        float offsetY = fmodf(panOffset.y * zoom, gridStep);
        if (offsetX < 0.0f) offsetX += gridStep;
        if (offsetY < 0.0f) offsetY += gridStep;

        ImU32 lineColor = IM_COL32(60, 60, 64, 255);
        for (float x = offsetX; x < canvasSize.x; x += gridStep)
        {
            drawList->AddLine(ImVec2(canvasOrigin.x + x, canvasOrigin.y),
                ImVec2(canvasOrigin.x + x, canvasOrigin.y + canvasSize.y), lineColor);
        }
        for (float y = offsetY; y < canvasSize.y; y += gridStep)
        {
            drawList->AddLine(ImVec2(canvasOrigin.x, canvasOrigin.y + y),
                ImVec2(canvasOrigin.x + canvasSize.x, canvasOrigin.y + y), lineColor);
        }
    }

	void AnimatorControllerEditorWindow::DrawNodes(ImDrawList* drawList, const ImVec2& canvasOrigin, std::shared_ptr<AnimatorController>& controller, std::weak_ptr<RuntimeAnimatorController> runtimeController)
    {
        ImVec2 nodeSize(NodeWidth * zoom, NodeHeight * zoom);

        int currentStateIndex = -1;
        if (auto runtimeControllerPtr = runtimeController.lock())
        {
            currentStateIndex = runtimeControllerPtr->currentStateIndex;
        }

        for (int i = 0; i < (int)controller->states.size(); ++i)
        {
            const auto& state = controller->states[i];
			bool isCurrentState = (currentStateIndex == i);
            ImVec2 nodeCenter = WorldToScreen(state.editorPosition, canvasOrigin);
            ImVec2 nodeMin(nodeCenter.x - nodeSize.x * 0.5f, nodeCenter.y - nodeSize.y * 0.5f);
            ImVec2 nodeMax(nodeCenter.x + nodeSize.x * 0.5f, nodeCenter.y + nodeSize.y * 0.5f);

            // InvisibleButtonの設置とドラッグ/選択/右クリックメニュー処理
            HandleNodeInteraction(i, nodeCenter, canvasOrigin, controller);

            bool isSelected = (selectedStateIndex == i);
            bool isDefault = (i == controller->defaultStateIndex);
            bool isCreatingSource = (mode == InteractionMode::CreatingTransition && transitionSourceIndex == i);

            ImU32 fillColor = isDefault ? IM_COL32(90, 110, 70, 255) : IM_COL32(70, 70, 75, 255);
			if (isCurrentState) // 現在のステートを示す場合は色を変える
            {
				fillColor = isDefault ? IM_COL32(120, 150, 90, 255) : IM_COL32(100, 100, 105, 255);
			}
            ImU32 borderColor = isSelected ? IM_COL32(255, 180, 60, 255)
                : isCreatingSource ? IM_COL32(255, 220, 100, 255)
                : IM_COL32(20, 20, 20, 255);
            if (isCurrentState && !isSelected && !isCreatingSource)
            {
                borderColor = IM_COL32(255, 220, 100, 255); // 現在のステートを示す場合は色を変える
			}
            float rounding = 6.0f * zoom;

            drawList->AddRectFilled(nodeMin, nodeMax, fillColor, rounding);
            drawList->AddRect(nodeMin, nodeMax, borderColor, rounding, 0, isSelected ? 3.0f : 1.5f);

            if (isDefault)
            {
                // デフォルトステートを示す左向き三角マーカー(Unity風)
                float markerSize = 6.0f * zoom;
                ImVec2 tip(nodeMin.x - markerSize, nodeCenter.y);
                ImVec2 top(nodeMin.x, nodeCenter.y - markerSize);
                ImVec2 bottom(nodeMin.x, nodeCenter.y + markerSize);
                drawList->AddTriangleFilled(tip, top, bottom, IM_COL32(140, 200, 90, 255));
            }

            if (state.blendType != BlendTreeType::None)
            {
                // ブレンドツリーであることを示す右上の小さいマーカー
                float markerRadius = 4.0f * zoom;
                ImVec2 markerCenter(nodeMax.x - markerRadius - 4.0f * zoom, nodeMin.y + markerRadius + 4.0f * zoom);
                drawList->AddCircleFilled(markerCenter, markerRadius, IM_COL32(120, 170, 240, 255));
            }

            // ノード内にクリップしてタイトルを描画(ズームでフォントサイズは変えず可読性を優先)
            ImVec2 textSize = ImGui::CalcTextSize(state.name.c_str());
            ImVec2 textPos(nodeCenter.x - textSize.x * 0.5f, nodeCenter.y - textSize.y * 0.5f);
            drawList->PushClipRect(nodeMin, nodeMax, true);
            drawList->AddText(textPos, IM_COL32(240, 240, 240, 255), state.name.c_str());
            drawList->PopClipRect();
        }
    }

    void AnimatorControllerEditorWindow::HandleAnyStateInteraction(const ImVec2& canvasOrigin)
    {
        ImVec2 nodeSize(NodeWidth * zoom, NodeHeight * zoom);
        ImVec2 nodeScreenPos = WorldToScreen(anyStatePosition, canvasOrigin);
        nodeScreenPos.x -= nodeSize.x * 0.5f;
        nodeScreenPos.y -= nodeSize.y * 0.5f;

        ImGui::SetCursorScreenPos(nodeScreenPos);
        ImGui::PushID("AnyState");
        ImGui::InvisibleButton("anyStateNode", nodeSize);

        if (mode == InteractionMode::CreatingTransition && ImGui::IsItemClicked())
        {
            // AnyStateは遷移先(to)にはしない設計なので、ここではキャンセル扱いにする
            mode = InteractionMode::None;
            transitionSourceIndex = -2;
        }
        else
        {
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                ImVec2 delta = ImGui::GetIO().MouseDelta;
                anyStatePosition.x += delta.x / zoom;
                anyStatePosition.y += delta.y / zoom;
            }
            else if (ImGui::IsItemDeactivated() && !ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                selectedStateIndex = -1;
                selectedTransitionIndex = -1;
            }
        }

        if (ImGui::BeginPopupContextItem("anyStateContext"))
        {
            if (ImGui::MenuItem("Make Transition"))
            {
                mode = InteractionMode::CreatingTransition;
                transitionSourceIndex = -1;
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    void AnimatorControllerEditorWindow::DrawAnyStateNode(ImDrawList* drawList, const ImVec2& canvasOrigin, std::shared_ptr<AnimatorController>& controller)
    {
        ImVec2 nodeSize(NodeWidth * zoom, NodeHeight * zoom);
        ImVec2 nodeCenter = WorldToScreen(anyStatePosition, canvasOrigin);
        ImVec2 nodeMin(nodeCenter.x - nodeSize.x * 0.5f, nodeCenter.y - nodeSize.y * 0.5f);
        ImVec2 nodeMax(nodeCenter.x + nodeSize.x * 0.5f, nodeCenter.y + nodeSize.y * 0.5f);

        HandleAnyStateInteraction(canvasOrigin);

        bool isCreatingSource = (mode == InteractionMode::CreatingTransition && transitionSourceIndex == -1);
        ImU32 borderColor = isCreatingSource ? IM_COL32(255, 220, 100, 255) : IM_COL32(20, 20, 20, 255);

        drawList->AddRectFilled(nodeMin, nodeMax, IM_COL32(90, 70, 100, 255), 6.0f * zoom);
        drawList->AddRect(nodeMin, nodeMax, borderColor, 6.0f * zoom, 0, isCreatingSource ? 3.0f : 1.5f);

        const char* label = "Any State";
        ImVec2 textSize = ImGui::CalcTextSize(label);
        ImVec2 textPos(nodeCenter.x - textSize.x * 0.5f, nodeCenter.y - textSize.y * 0.5f);
        drawList->PushClipRect(nodeMin, nodeMax, true);
        drawList->AddText(textPos, IM_COL32(240, 240, 240, 255), label);
        drawList->PopClipRect();

        (void)controller; // 今はcontroller参照は使っていないが、将来AnyState位置を永続化する際に使う想定
    }

    void AnimatorControllerEditorWindow::HandleNodeInteraction(int stateIndex, const ImVec2& nodeCenterWorld, const ImVec2& canvasOrigin, std::shared_ptr<AnimatorController>& controller)
    {
        ImVec2 nodeSize(NodeWidth * zoom, NodeHeight * zoom);
        ImVec2 nodeScreenPos = WorldToScreen(controller->states[stateIndex].editorPosition, canvasOrigin);
        nodeScreenPos.x -= nodeSize.x * 0.5f;
        nodeScreenPos.y -= nodeSize.y * 0.5f;

        ImGui::SetCursorScreenPos(nodeScreenPos);
        ImGui::PushID(stateIndex);
        ImGui::InvisibleButton("node", nodeSize);

        // 遷移作成モード中にノード上でクリック → 遷移を確定
        if (mode == InteractionMode::CreatingTransition && ImGui::IsItemClicked())
        {
            if (transitionSourceIndex != stateIndex) // 自己遷移は許可しない設計にするならここで弾く
            {
                controller->transitions.push_back(AnimatorTransition{ transitionSourceIndex, stateIndex, 0.25f, {}, false, 0.0f });
				UpdateTransitionOffsetIndices(controller); // 相互遷移のインデックスを更新
            }
            mode = InteractionMode::None;
            transitionSourceIndex = -2;
        }
		else if (ImGui::IsWindowFocused())
        {
            if ((selectedStateIndex == stateIndex) && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                mode = InteractionMode::DraggingNode;
                ImVec2 delta = ImGui::GetIO().MouseDelta;
                controller->states[stateIndex].editorPosition.x += delta.x / zoom;
                controller->states[stateIndex].editorPosition.y += delta.y / zoom;
            }
            else if (ImGui::IsItemClicked())
            {
                selectedStateIndex = stateIndex;
                selectedTransitionIndex = -1;
                mode = InteractionMode::None;
            }
        }

        // 右クリックでコンテキストメニュー
        if (ImGui::BeginPopupContextItem("nodeContext"))
        {
            if (ImGui::MenuItem("Make Transition"))
            {
                mode = InteractionMode::CreatingTransition;
                transitionSourceIndex = stateIndex;
            }
            if (ImGui::MenuItem("Set As Default State"))
            {
                controller->defaultStateIndex = stateIndex;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete State"))
            {
                DeleteState(stateIndex, controller);
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }


    void AnimatorControllerEditorWindow::DrawTransitions(ImDrawList* drawList, const ImVec2& canvasOrigin, std::shared_ptr<AnimatorController>& controller)
    {
        ImVec2 nodeSize(NodeWidth * zoom, NodeHeight * zoom);
        ImVec2 mousePos = ImGui::GetIO().MousePos;
        bool clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

        for (int i = 0; i < (int)controller->transitions.size(); ++i)
        {
            auto& t = controller->transitions[i];

            ImVec2 fromCenter = (t.fromStateIndex == -1)
                ? WorldToScreen(anyStatePosition, canvasOrigin)
                : WorldToScreen(controller->states[t.fromStateIndex].editorPosition, canvasOrigin);
            ImVec2 toCenter = WorldToScreen((controller->states[t.toStateIndex].editorPosition), canvasOrigin);

            ImVec2 from;
            ImVec2 to;
            
            int index = -1;
            for (size_t j = 0; j < transitionOffsetIndices.size(); ++j)
            {
                if (transitionOffsetIndices[j] == i)
                {
                    index = transitionOffsetIndices[j];
                    break;
                }
            }
            // 相互遷移がある場合、遷移ラインをずらす
            if (index != -1)
            {
                // 遷移ラインの方向に垂直な方向にオフセットする
                float offsetAmount = 10.0f * zoom;
                from = GetOffsetNodeEdgePoint(fromCenter, toCenter, nodeSize, offsetAmount);
                to = GetOffsetNodeEdgePoint(toCenter, fromCenter, nodeSize, -offsetAmount);
            }
			else // 相互遷移がない場合は通常通りノードの縁にクリップ
            {
                from = GetNodeEdgePoint(fromCenter, toCenter, nodeSize);
				to = GetNodeEdgePoint(toCenter, fromCenter, nodeSize);
			}

            bool isSelected = (selectedTransitionIndex == i);
            ImU32 color = isSelected ? IM_COL32(255, 180, 60, 255) : IM_COL32(180, 180, 180, 255);

            drawList->AddLine(from, to, color, isSelected ? 3.0f : 2.0f);

            // 矢印head
            ImVec2 dir(to.x - from.x, to.y - from.y);
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 0.0f)
            {
                dir.x /= len; dir.y /= len;
                ImVec2 perp(-dir.y, dir.x);
                float arrowSize = 8.0f * zoom;
                ImVec2 tip = to;
                ImVec2 left(tip.x - dir.x * arrowSize + perp.x * arrowSize * 0.5f, tip.y - dir.y * arrowSize + perp.y * arrowSize * 0.5f);
                ImVec2 right(tip.x - dir.x * arrowSize - perp.x * arrowSize * 0.5f, tip.y - dir.y * arrowSize - perp.y * arrowSize * 0.5f);
                drawList->AddTriangleFilled(tip, left, right, color);
            }

            // クリック判定(ノードのInvisibleButtonに埋もれないよう、ノード領域外だけ判定)
            if (clicked && DistancePointToSegment(mousePos, from, to) < HitTestLineThreshold)
            {
                selectedTransitionIndex = i;
                selectedStateIndex = -1;
            }
        }
    }

    void AnimatorControllerEditorWindow::DrawTransitionPreview(ImDrawList* drawList, const ImVec2& canvasOrigin, std::shared_ptr<AnimatorController>& controller)
    {
        if (mode != InteractionMode::CreatingTransition) return;

        ImVec2 fromCenter = (transitionSourceIndex == -1)
            ? WorldToScreen(anyStatePosition, canvasOrigin)
            : WorldToScreen(controller->states[transitionSourceIndex].editorPosition, canvasOrigin);
		ImVec2 toCenter = ImGui::GetIO().MousePos;
		ImU32 color = IM_COL32(255, 220, 100, 200);

        drawList->AddLine(fromCenter, toCenter, color, 2.0f);

        // 矢印head
        ImVec2 dir(toCenter.x - fromCenter.x, toCenter.y - fromCenter.y);
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.0f)
        {
            dir.x /= len; dir.y /= len;
            ImVec2 perp(-dir.y, dir.x);
            float arrowSize = 8.0f * zoom;
            ImVec2 tip = toCenter;
            ImVec2 left(tip.x - dir.x * arrowSize + perp.x * arrowSize * 0.5f, tip.y - dir.y * arrowSize + perp.y * arrowSize * 0.5f);
            ImVec2 right(tip.x - dir.x * arrowSize - perp.x * arrowSize * 0.5f, tip.y - dir.y * arrowSize - perp.y * arrowSize * 0.5f);
            drawList->AddTriangleFilled(tip, left, right, color);
        }

		bool cancelTransition = false;
        // Escで作成キャンセル
		cancelTransition |= ImGui::IsKeyPressed(ImGuiKey_Escape);
		// 右クリックで作成キャンセル
		cancelTransition |= ImGui::IsMouseClicked(ImGuiMouseButton_Right);

        if (cancelTransition)
        {
            mode = InteractionMode::None;
            transitionSourceIndex = -2;
        }
    }

    void AnimatorControllerEditorWindow::HandleCanvasBackground(const ImVec2& canvasOrigin, const ImVec2& canvasSize, std::shared_ptr<AnimatorController>& controller)
    {
        ImGui::SetCursorScreenPos(canvasOrigin);
        // 右ボタンもActive扱いにしないと、右ドラッグでのパンがIsItemActive()に反映されない
        ImGui::InvisibleButton("canvasBg", canvasSize, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        ImGui::SetItemAllowOverlap();
        bool isHovered = ImGui::IsItemHovered();
        bool isActive = ImGui::IsItemActive();

        // --- パン(右ドラッグ) ---
        if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.0f))
        {
            ImVec2 delta = ImGui::GetIO().MouseDelta;
            panOffset.x += delta.x / zoom;
            panOffset.y += delta.y / zoom;
        }

        // 右ボタン押下位置を記録(離したときにパンだったかクリックだったかを距離で判定するため)
        if (isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            rightClickStartPos = ImGui::GetIO().MousePos;
        }

        // 右ボタンを離した瞬間、押下位置からほぼ動いていなければ「クリック」とみなしてメニューを開く
        const float dragThreshold = 4.0f;
        if (isHovered && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
        {
            ImVec2 mousePos = ImGui::GetIO().MousePos;
            float dx = mousePos.x - rightClickStartPos.x;
            float dy = mousePos.y - rightClickStartPos.y;
            if (std::sqrt(dx * dx + dy * dy) < dragThreshold)
            {
                pendingContextMenuScreenPos = mousePos;
                ImGui::OpenPopup("canvasContext");
            }
        }

        // ズーム(マウスホイール)
        if (isHovered && ImGui::GetIO().MouseWheel != 0.0f)
        {
            zoom = std::clamp(zoom + ImGui::GetIO().MouseWheel * 0.1f, 0.25f, 2.5f);
        }

        // 背景クリック→選択解除
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            selectedStateIndex = -1;
            selectedTransitionIndex = -1;
        }

        // コンテキストメニュー本体(OpenPopupは上で手動制御済みなのでBeginPopupのみでよい)
        if (ImGui::BeginPopup("canvasContext"))
        {
            if (mode == InteractionMode::CreatingTransition)
            {
                if (ImGui::MenuItem("Cancel Transition"))
                {
                    mode = InteractionMode::None;
                    transitionSourceIndex = -2;
                }
            }
            else if (ImGui::MenuItem("Create State"))
            {
                Vector2 worldPos = ScreenToWorld(pendingContextMenuScreenPos, canvasOrigin);
                controller->states.push_back(AnimatorState{ "NewState", CurryEngine::Resources::AssetId(), 1.0f, true, worldPos });
            }
            ImGui::EndPopup();
        }

        // Deleteキーで選択中のState/Transitionを削除
        if (ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            if (selectedStateIndex >= 0) DeleteState(selectedStateIndex, controller);
            else if (selectedTransitionIndex >= 0) DeleteTransition(selectedTransitionIndex, controller);
        }
    }

    void AnimatorControllerEditorWindow::DeleteState(int stateIndex, std::shared_ptr<AnimatorController>& controller)
    {
        if (stateIndex < 0 || stateIndex >= (int)controller->states.size()) return;

        controller->transitions.erase(
            std::remove_if(controller->transitions.begin(), controller->transitions.end(),
                [stateIndex](const AnimatorTransition& t) {
                    return t.fromStateIndex == stateIndex || t.toStateIndex == stateIndex;
                }),
            controller->transitions.end());

		// 削除したStateより後ろのStateのインデックスを1つずつ減らす
        for (auto& t : controller->transitions)
        {
            if (t.fromStateIndex > stateIndex) t.fromStateIndex--;
            if (t.toStateIndex > stateIndex) t.toStateIndex--;
        }
        controller->states.erase(controller->states.begin() + stateIndex);

        if (controller->defaultStateIndex >= (int)controller->states.size())
            controller->defaultStateIndex = 0;

        // 相互遷移のインデックスを更新
        UpdateTransitionOffsetIndices(controller);

        selectedStateIndex = -1;
    }

    void AnimatorControllerEditorWindow::DeleteTransition(int transitionIndex, std::shared_ptr<AnimatorController>& controller)
    {
        if (transitionIndex < 0 || transitionIndex >= (int)controller->transitions.size()) return;
        controller->transitions.erase(controller->transitions.begin() + transitionIndex);

        // 相互遷移のインデックスを更新
        UpdateTransitionOffsetIndices(controller);
        selectedTransitionIndex = -1;
    }


	void AnimatorControllerEditorWindow::Draw(bool* isOpen, std::shared_ptr<AnimatorController> controller, std::weak_ptr<RuntimeAnimatorController> runtimeController)
    {
        if (!controller) return;

        ImGui::SetNextWindowSize(ImVec2(1100, 650), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Animator Controller Editor", isOpen))
        {
            ImGui::End();
            return;
        }
        if (ImGui::Button("Save"))
        {
            controller->SaveToFile(controller->GetPath());
		}
        const float inspectorWidth = 340.0f;
        ImVec2 avail = ImGui::GetContentRegionAvail();

        // --- 左: キャンバス ---
        ImGui::BeginChild("AnimatorCanvas", ImVec2(avail.x - inspectorWidth - 4.0f, avail.y), true,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 canvasOrigin = ImGui::GetCursorScreenPos();
            ImVec2 canvasSize = ImGui::GetContentRegionAvail();

            drawList->PushClipRect(canvasOrigin, ImVec2(canvasOrigin.x + canvasSize.x, canvasOrigin.y + canvasSize.y), true);

            DrawGrid(drawList, canvasOrigin, canvasSize);

            // 背景のInvisibleButtonを先に敷く(パン/ズーム/選択解除/State作成)。
            // ノード側のInvisibleButtonを後で重ねて描くことで、重なった際はノードのヒットが優先される。
            HandleCanvasBackground(canvasOrigin, canvasSize, controller);

            DrawTransitions(drawList, canvasOrigin, controller);
            DrawTransitionPreview(drawList, canvasOrigin, controller);

            DrawAnyStateNode(drawList, canvasOrigin, controller);
            DrawNodes(drawList, canvasOrigin, controller, runtimeController);

            drawList->PopClipRect();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // --- 右: インスペクタ ---
        ImGui::BeginChild("AnimatorInspector", ImVec2(inspectorWidth, avail.y), true);
        DrawInspectorPanel(controller, runtimeController);
        ImGui::EndChild();

        ImGui::End();
    }


    void AnimatorControllerEditorWindow::DrawInspectorPanel(std::shared_ptr<AnimatorController>& controller, std::weak_ptr<RuntimeAnimatorController> runtimeController)
    {
        if (ImGui::BeginTabBar("InspectorTabs"))
        {
            if (ImGui::BeginTabItem("Selection"))
            {
                if (selectedStateIndex >= 0 && selectedStateIndex < (int)controller->states.size())
                {
                    DrawStateInspector(selectedStateIndex, controller);
                }
                else if (selectedTransitionIndex >= 0 && selectedTransitionIndex < (int)controller->transitions.size())
                {
                    DrawTransitionInspector(selectedTransitionIndex, controller);
                }
                else
                {
                    ImGui::TextDisabled("ノードまたは遷移を選択してください。");
                    ImGui::Spacing();
                    ImGui::TextWrapped("右クリックで State の作成、ノード上右クリックから Make Transition で遷移を作成できます。");
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Parameters"))
            {
                DrawParametersTab(controller, runtimeController);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }

    void AnimatorControllerEditorWindow::DrawStateInspector(int stateIndex, std::shared_ptr<AnimatorController>& controller)
    {
        AnimatorState& state = controller->states[stateIndex];
        ImGui::SeparatorText("State");

        char nameBuffer[128];
        strncpy_s(nameBuffer, state.name.c_str(), sizeof(nameBuffer));
        if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
        {
            state.name = nameBuffer;
        }

        const char* blendTypeNames[] = { "Single Clip", "Blend Tree (1D)", "Blend Tree (2D Freeform)" };
        int blendTypeIndex = static_cast<int>(state.blendType);
        if (ImGui::BeginCombo("Motion Type", blendTypeNames[blendTypeIndex]))
        {
            for (int i = 0; i < IM_ARRAYSIZE(blendTypeNames); ++i)
            {
                bool isSelected = (blendTypeIndex == i);
                if (ImGui::Selectable(blendTypeNames[i], isSelected))
                {
                    state.blendType = static_cast<BlendTreeType>(i);
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing();

        if (state.blendType == BlendTreeType::None)
            DrawSingleClipSection(state, controller);
        else
            DrawBlendTreeSection(state, controller);

        ImGui::Spacing();
        ImGui::InputFloat("Speed", &state.speed);
        ImGui::Checkbox("Loop", &state.loop);

        ImGui::Spacing();
        bool isDefault = (stateIndex == controller->defaultStateIndex);
        if (ImGui::Checkbox("Default State", &isDefault) && isDefault)
        {
            controller->defaultStateIndex = stateIndex;
        }

        ImGui::Spacing();
        if (ImGui::Button("Delete State", ImVec2(-1, 0)))
        {
            DeleteState(stateIndex, controller);
        }
    }

    void AnimatorControllerEditorWindow::DrawSingleClipSection(AnimatorState& state, std::shared_ptr<AnimatorController>& controller)
    {
        ImGui::Text("Clip: %s", GetClipDisplayName(controller, state.clipId).c_str());
        ImGui::SameLine();
        state.clipId = DrawClipPickerButton("SelectAnimationClipPopup", controller, state.clipId);
    }

    void AnimatorControllerEditorWindow::DrawBlendTreeSection(AnimatorState& state, std::shared_ptr<AnimatorController>& controller)
    {
        const bool is2D = (state.blendType == BlendTreeType::FreeformCartesian2D);

        auto drawParamCombo = [&](const char* label, int& paramIndex)
            {
                std::string current = (paramIndex >= 0 && paramIndex < (int)controller->parameters.size())
                    ? controller->parameters[paramIndex].name : "None";
                if (ImGui::BeginCombo(label, current.c_str()))
                {
                    for (int k = 0; k < (int)controller->parameters.size(); ++k)
                    {
                        // Trigger/Boolはブレンド軸として不向きなので候補から除外
                        if (controller->parameters[k].type == AnimatorParameter::Type::Trigger ||
                            controller->parameters[k].type == AnimatorParameter::Type::Bool)
                            continue;
                        bool isSelected = (paramIndex == k);
                        if (ImGui::Selectable(controller->parameters[k].name.c_str(), isSelected))
                            paramIndex = k;
                        if (isSelected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            };

        drawParamCombo(is2D ? "Blend Param X" : "Blend Param", state.blendParamXIndex);
        if (is2D) drawParamCombo("Blend Param Y", state.blendParamYIndex);

        ImGui::Spacing();
		ImGui::InputFloat("Blend Smooth Time", &state.blendSmoothTime);
        state.blendSmoothTime = (std::max)(0.0f, state.blendSmoothTime);

		ImGui::Spacing();
        ImGui::SeparatorText("Entries");

        // 1Dはしきい値順でないとComputeBlendWeights側の補間が破綻するため描画毎にソート
        if (state.blendType == BlendTreeType::Simple1D)
        {
            std::sort(state.blendEntries.begin(), state.blendEntries.end(),
                [](const BlendTreeEntry& a, const BlendTreeEntry& b) { return a.threshold < b.threshold; });
        }

        int removeIndex = -1;
        for (int i = 0; i < (int)state.blendEntries.size(); ++i)
        {
            ImGui::PushID(i);
            auto& entry = state.blendEntries[i];

            ImGui::Text("[%d] %s", i, GetClipDisplayName(controller, entry.clipId).c_str());
            ImGui::SameLine();
            entry.clipId = DrawClipPickerButton("SelectBlendEntryClipPopup", controller, entry.clipId);
            ImGui::SameLine();
            if (ImGui::SmallButton("Remove")) removeIndex = i;

            if (state.blendType == BlendTreeType::Simple1D)
                ImGui::InputFloat("Threshold", &entry.threshold);
            else
                ImGui::InputFloat2("Position (X, Y)", &entry.position.x);

            ImGui::Separator();
            ImGui::PopID();
        }
        if (removeIndex >= 0) state.blendEntries.erase(state.blendEntries.begin() + removeIndex);

        if (ImGui::Button(" + Entry", ImVec2(-1, 0)))
            state.blendEntries.push_back(BlendTreeEntry{});

        if (state.blendEntries.size() < 2)
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "ブレンドには2つ以上のEntryが必要です。");
    }

    void AnimatorControllerEditorWindow::DrawTransitionInspector(int transitionIndex, std::shared_ptr<AnimatorController>& controller)
    {
        AnimatorTransition& transition = controller->transitions[transitionIndex];
        ImGui::SeparatorText("Transition");

        std::string fromName = (transition.fromStateIndex == -1) ? "Any State"
            : (transition.fromStateIndex >= 0 && transition.fromStateIndex < (int)controller->states.size())
            ? controller->states[transition.fromStateIndex].name : "None";
        std::string toName = (transition.toStateIndex >= 0 && transition.toStateIndex < (int)controller->states.size())
            ? controller->states[transition.toStateIndex].name : "None";

        ImGui::Text("From: %s", fromName.c_str());
        ImGui::Text("To:   %s", toName.c_str());

        ImGui::InputFloat("Blend Duration", &transition.blendDuration);
        ImGui::Checkbox("Has Exit Time", &transition.hasExitTime);
        if (transition.hasExitTime)
        {
            ImGui::SliderFloat("Exit Time", &transition.exitTime, 0.0f, 1.0f);
        }

        ImGui::SeparatorText("Conditions");
        int removeIndex = -1;
        for (int j = 0; j < (int)transition.conditions.size(); ++j)
        {
            auto& condition = transition.conditions[j];
            ImGui::PushID(j);

            std::string parameterName = (condition.parameterIndex >= 0 && condition.parameterIndex < (int)controller->parameters.size())
                ? controller->parameters[condition.parameterIndex].name : "None";
            if (ImGui::BeginCombo("Parameter", parameterName.c_str()))
            {
                for (int k = 0; k < (int)controller->parameters.size(); ++k)
                {
                    bool isSelected = (condition.parameterIndex == k);
                    if (ImGui::Selectable(controller->parameters[k].name.c_str(), isSelected))
                    {
                        condition.parameterIndex = k;
                    }
                    if (isSelected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            bool isTrigger = (condition.parameterIndex >= 0 && condition.parameterIndex < (int)controller->parameters.size())
                && (controller->parameters[condition.parameterIndex].type == AnimatorParameter::Type::Trigger);

            if (!isTrigger)
            {
                const char* comparisonTypes[] = { "<", "<=", ">", ">=", "==", "!=" };
                int currentComparisonIndex = static_cast<int>(condition.comparison);
                if (ImGui::BeginCombo("Comparison", comparisonTypes[currentComparisonIndex]))
                {
                    for (int k = 0; k < IM_ARRAYSIZE(comparisonTypes); ++k)
                    {
                        bool isSelected = (currentComparisonIndex == k);
                        if (ImGui::Selectable(comparisonTypes[k], isSelected))
                        {
                            condition.comparison = static_cast<AnimatorCondition::Comparison>(k);
                        }
                        if (isSelected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::InputFloat("Value", &condition.value);
            }

            if (ImGui::SmallButton("Remove"))
            {
                removeIndex = j; // ループ中の直接eraseはイテレータ破壊を招くため、ループ後にまとめて処理
            }

            ImGui::Separator();
            ImGui::PopID();
        }
        if (removeIndex >= 0)
        {
            transition.conditions.erase(transition.conditions.begin() + removeIndex);
        }

        if (ImGui::Button(" + Condition", ImVec2(-1, 0)))
        {
            transition.conditions.push_back(AnimatorCondition{ -1, AnimatorCondition::Comparison::Equal, 0.0f });
        }

        ImGui::Spacing();
        if (ImGui::Button("Delete Transition", ImVec2(-1, 0)))
        {
            DeleteTransition(transitionIndex, controller);
        }
    }

    void AnimatorControllerEditorWindow::DrawParametersTab(std::shared_ptr<AnimatorController>& controller, std::weak_ptr<RuntimeAnimatorController> runtimeController)
    {
        for (int i = 0; i < (int)controller->parameters.size(); ++i)
        {
            ImGui::PushID(i);
            auto& parameter = controller->parameters[i];

            char buffer[128];
            strncpy_s(buffer, parameter.name.c_str(), sizeof(buffer));
            if (ImGui::InputText("Name", buffer, sizeof(buffer)))
            {
                parameter.name = buffer;
            }

            const char* parameterTypes[] = { "Float", "Int", "Bool", "Trigger" };
            int currentTypeIndex = static_cast<int>(parameter.type);
            if (ImGui::BeginCombo("Type", parameterTypes[currentTypeIndex]))
            {
                for (int j = 0; j < IM_ARRAYSIZE(parameterTypes); ++j)
                {
                    bool isSelected = (currentTypeIndex == j);
                    if (ImGui::Selectable(parameterTypes[j], isSelected))
                    {
                        parameter.type = static_cast<AnimatorParameter::Type>(j);
                    }
                    if (isSelected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
			bool runtimeControllerActive = !runtimeController.expired();
			float* runtimeParam = &parameter.defaultValue;
			if (auto runtimeCtrl = runtimeController.lock())
            {
				if (runtimeCtrl->parameterValues.find(parameter.name) != runtimeCtrl->parameterValues.end())
                {
                    runtimeParam = &runtimeCtrl->parameterValues[parameter.name];
				}
			}

            switch (parameter.type)
            {
            case AnimatorParameter::Type::Float:
            {
                ImGui::InputFloat("Default", runtimeParam);
                break;
            }
            case AnimatorParameter::Type::Int:
            {
                ImGui::InputInt("Default", reinterpret_cast<int*>(runtimeParam));
                break;
            }
            case AnimatorParameter::Type::Bool:
            {
				ImGui::Checkbox("Default", reinterpret_cast<bool*>(runtimeParam));
                break;
            }
            default:
                break;
            }

            if (ImGui::SmallButton("Remove Parameter"))
            {
                controller->parameters.erase(controller->parameters.begin() + i);
                ImGui::PopID();
                break;
            }

            ImGui::Separator();
            ImGui::PopID();
        }

        if (ImGui::Button(" + Parameter", ImVec2(-1, 0)))
        {
            controller->parameters.push_back(AnimatorParameter{ "NewParameter", AnimatorParameter::Type::Float, 0.0f });
        }
    }

    CurryEngine::Resources::AssetId AnimatorControllerEditorWindow::DrawClipPickerButton(
        const char* popupId, std::shared_ptr<AnimatorController>& controller, const CurryEngine::Resources::AssetId& currentClipId)
    {
        static std::unordered_map<CurryEngine::Resources::AssetId, std::shared_ptr<AnimationClip>> clipMap;
        CurryEngine::Resources::AssetId result = currentClipId;

        if (ImGui::SmallButton("..."))
        {
            ImGui::OpenPopup(popupId);
            clipMap.clear();
            std::vector<CurryEngine::Resources::AssetMeta> metas = CurryEngine::Resources::AssetDatabase::FindAllByType(AssetType::Animation);
            for (const auto& meta : metas)
                clipMap[meta.id] = CurryEngine::Resources::AssetDatabase::LoadAsset<AnimationClip>(meta.id);
        }
        if (ImGui::BeginPopup(popupId))
        {
            // 検索ボックス
            static char searchBuffer[128] = "";
            ImGui::InputText("##Search", searchBuffer, sizeof(searchBuffer));

            for (const auto& [clipId, clip] : clipMap)
            {
                if (!clip) continue;

				// 検索文字列が空でない場合、名前に検索文字列が含まれていないクリップはスキップ
                if (searchBuffer[0] != '\0' && clip->name.find(searchBuffer) == std::string::npos)
					continue;

                if (ImGui::Selectable(clip->name.c_str()))
                {
                    controller->animationClips[clipId] = clip;
                    result = clipId;
                }
            }
            ImGui::EndPopup();
        }
        return result;
    }

    std::string AnimatorControllerEditorWindow::GetClipDisplayName(
        std::shared_ptr<AnimatorController>& controller, const CurryEngine::Resources::AssetId& clipId) const
    {
        if (!clipId.IsValid()) return "None";
        auto it = controller->animationClips.find(clipId);
        return (it != controller->animationClips.end() && it->second) ? it->second->name : "None";
    }
}


#endif // USE_IMGUI
