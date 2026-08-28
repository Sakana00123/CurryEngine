#pragma once
#include "Engine/Animation/AnimatorController.h"
#include <memory>

#ifdef USE_IMGUI
namespace CurryEngine::Editor
{
    class AnimatorControllerEditorWindow
    {
    public:
		AnimatorControllerEditorWindow(std::shared_ptr<AnimatorController> controller = nullptr);

        void Draw(bool* isOpen, std::shared_ptr<AnimatorController> controller);

    private:
        // --- キャンバス変換 ---
        ImVec2 panOffset = { 0.0f, 0.0f };
        float zoom = 1.0f;

        // --- 操作状態 ---
        enum class InteractionMode { None, DraggingNode, DraggingCanvas, CreatingTransition };
        InteractionMode mode = InteractionMode::None;

        int selectedStateIndex = -1;
        int selectedTransitionIndex = -1;
        int draggingNodeIndex = -1;
        int transitionSourceIndex = -2; // -1ならAnyState、-2は「未設定」

		// transitionで、２つのノードで相互の遷移がある場合、nodeA→nodeBとnodeB→nodeAの両方の遷移を描画する際に、線が重なってしまうので、ずらすtransitionのインデックスを保持するための配列
		std::vector<int> transitionOffsetIndices;

        ImVec2 rightClickStartPos = { 0.0f, 0.0f };
        ImVec2 pendingContextMenuScreenPos = { 0.0f, 0.0f };

        Vector2 anyStatePosition = Vector2(-220.0f, 40.0f);

        static constexpr float NodeWidth = 160.0f;
        static constexpr float NodeHeight = 44.0f;
        static constexpr float HitTestLineThreshold = 6.0f;

        // --- 座標変換 ---
        ImVec2 WorldToScreen(const Vector2& worldPos, const ImVec2& canvasOrigin) const;
        Vector2 ScreenToWorld(const ImVec2& screenPos, const ImVec2& canvasOrigin) const;

        // --- 描画 ---
        void DrawGrid(ImDrawList* drawList, const ImVec2& canvasOrigin, const ImVec2& canvasSize) const;
        void DrawNodes(ImDrawList* drawList, const ImVec2& canvasOrigin, std::shared_ptr<AnimatorController>& controller);
        void DrawAnyStateNode(ImDrawList* drawList, const ImVec2& canvasOrigin, std::shared_ptr<AnimatorController>& controller);
        void DrawTransitions(ImDrawList* drawList, const ImVec2& canvasOrigin, std::shared_ptr<AnimatorController>& controller);
        void DrawTransitionPreview(ImDrawList* drawList, const ImVec2& canvasOrigin, std::shared_ptr<AnimatorController>& controller);

        // --- 入力処理 ---
        void HandleCanvasBackground(const ImVec2& canvasOrigin, const ImVec2& canvasSize, std::shared_ptr<AnimatorController>& controller);
        void HandleNodeInteraction(int stateIndex, const ImVec2& nodeCenterWorld, const ImVec2& canvasOrigin, std::shared_ptr<AnimatorController>& controller);
        void HandleAnyStateInteraction(const ImVec2& canvasOrigin);

        // --- 編集操作 ---
        void DeleteState(int stateIndex, std::shared_ptr<AnimatorController>& controller);
        void DeleteTransition(int transitionIndex, std::shared_ptr<AnimatorController>& controller);

        // --- インスペクタ(既存Animator.cppのUIをここに移植) ---
        void DrawInspectorPanel(std::shared_ptr<AnimatorController>& controller);

		// --- インスペクタ描画 ---
        void DrawStateInspector(int stateIndex, std::shared_ptr<AnimatorController>& controller);
        void DrawTransitionInspector(int transitionIndex, std::shared_ptr<AnimatorController>& controller);
        void DrawParametersTab(std::shared_ptr<AnimatorController>& controller);

        // --- ヘルパー ---
		// 2つのノードの中心座標から、線分がノードの矩形に接する点を計算する
        ImVec2 GetNodeEdgePoint(const ImVec2& fromCenter, const ImVec2& toCenter, const ImVec2& nodeSize) const;
		// 2つのノードの中心座標からオフセットされた2点の線分が接続先のノードの矩形に接する点を計算する
		ImVec2 GetOffsetNodeEdgePoint(const ImVec2& fromCenter, const ImVec2& toCenter, const ImVec2& nodeSize, float offset) const;

		// 点pから線分abまでの距離を計算する
        float DistancePointToSegment(const ImVec2& p, const ImVec2& a, const ImVec2& b) const;
		void UpdateTransitionOffsetIndices(std::shared_ptr<AnimatorController>& controller);
    };
}
#endif // USE_IMGUI

