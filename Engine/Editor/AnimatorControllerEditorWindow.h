#pragma once
#include "Engine/Resources/AnimatorController.h"
#include <memory>
#include "AnimationTimelineEditor.h"

#ifdef USE_IMGUI
namespace CurryEngine::Editor
{
    /** @brief AnimatorControllerEditorWindow を表すクラスです。 */
    class AnimatorControllerEditorWindow
    {
    public:
		/**
		 * @brief AnimatorControllerEditorWindow を構築します。
		 */
		AnimatorControllerEditorWindow(std::shared_ptr<AnimatorController> controller = nullptr);

		/**
		 * @brief 描画処理を行います。
		 */
		void Draw(bool* isOpen, std::shared_ptr<AnimatorController> controller, std::weak_ptr<RuntimeAnimatorController> runtimeController, RenderContext* context);

		// --- TimelineEditorの取得 ---
		AnimationTimelineEditor& GetTimelineEditor() { return timelineEditor; }

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
		int selectedBlendEntryIndex = -1; // BlendTreeのブレンドエントリのインデックス
		bool isDraggingBlendEntry = false;

		// transitionで、２つのノードで相互の遷移がある場合、nodeA→nodeBとnodeB→nodeAの両方の遷移を描画する際に、線が重なってしまうので、ずらすtransitionのインデックスを保持するための配列
		std::vector<int> transitionOffsetIndices;

		AnimationTimelineEditor timelineEditor;
		bool isTimelineEditorOpen = false;

        ImVec2 rightClickStartPos = { 0.0f, 0.0f };
        ImVec2 pendingContextMenuScreenPos = { 0.0f, 0.0f };

        /**
         * @brief Vector2 の処理を行います。
         */
        Vector2 anyStatePosition = Vector2(-220.0f, 40.0f);

        static constexpr float NodeWidth = 160.0f;
        static constexpr float NodeHeight = 44.0f;
        static constexpr float HitTestLineThreshold = 6.0f;

        // --- 座標変換 ---
        /**
         * @brief WorldToScreen の処理を行います。
         */
        ImVec2 WorldToScreen(const Vector2& worldPos, const ImVec2& canvasOrigin) const;
        /**
         * @brief ScreenToWorld の処理を行います。
         */
        Vector2 ScreenToWorld(const ImVec2& screenPos, const ImVec2& canvasOrigin) const;

        // --- 描画 ---
		/**
		 * @brief 描画処理を行います。
		 */
		void DrawControllerWindowContents(std::shared_ptr<AnimatorController>& controller, std::weak_ptr<RuntimeAnimatorController> runtimeController);
        /**
         * @brief 描画処理を行います。
         */
        void DrawGrid(ImDrawList* drawList, const ImVec2& canvasOrigin, const ImVec2& canvasSize) const;
        /**
         * @brief 描画処理を行います。
         */
        void DrawNodes(ImDrawList* drawList, const ImVec2& canvasOrigin, std::shared_ptr<AnimatorController>& controller, std::weak_ptr<RuntimeAnimatorController> runtimeController);
        /**
         * @brief 描画処理を行います。
         */
        void DrawAnyStateNode(ImDrawList* drawList, const ImVec2& canvasOrigin, std::shared_ptr<AnimatorController>& controller);
		/**
		 * @brief 描画処理を行います。
		 */
		void DrawTransitions(ImDrawList* drawList, const ImVec2& canvasOrigin, std::shared_ptr<AnimatorController>& controller);
        /**
         * @brief 描画処理を行います。
         */
        void DrawTransitionPreview(ImDrawList* drawList, const ImVec2& canvasOrigin, std::shared_ptr<AnimatorController>& controller);

        // --- 入力処理 ---
        /**
         * @brief HandleCanvasBackground の処理を行います。
         */
        void HandleCanvasBackground(const ImVec2& canvasOrigin, const ImVec2& canvasSize, std::shared_ptr<AnimatorController>& controller);
        /**
         * @brief HandleNodeInteraction の処理を行います。
         */
        void HandleNodeInteraction(int stateIndex, const ImVec2& nodeCenterWorld, const ImVec2& canvasOrigin, std::shared_ptr<AnimatorController>& controller);
        /**
         * @brief HandleAnyStateInteraction の処理を行います。
         */
        void HandleAnyStateInteraction(const ImVec2& canvasOrigin);

        // --- 編集操作 ---
        /**
         * @brief DeleteState の処理を行います。
         */
        void DeleteState(int stateIndex, std::shared_ptr<AnimatorController>& controller);
        /**
         * @brief DeleteTransition の処理を行います。
         */
        void DeleteTransition(int transitionIndex, std::shared_ptr<AnimatorController>& controller);

        // --- インスペクタ(既存Animator.cppのUIをここに移植) ---
        /**
         * @brief 描画処理を行います。
         */
        void DrawInspectorPanel(std::shared_ptr<AnimatorController>& controller, std::weak_ptr<RuntimeAnimatorController> runtimeController);

		// --- インスペクタ描画 ---
        /**
         * @brief 描画処理を行います。
         */
        void DrawStateInspector(int stateIndex, std::shared_ptr<AnimatorController>& controller, std::weak_ptr<RuntimeAnimatorController> runtimeController);
        /**
         * @brief 描画処理を行います。
         */
        void DrawTransitionInspector(int transitionIndex, std::shared_ptr<AnimatorController>& controller);
        /**
         * @brief 描画処理を行います。
         */
        void DrawParametersTab(std::shared_ptr<AnimatorController>& controller, std::weak_ptr<RuntimeAnimatorController> runtimeController);

        /**
         * @brief 描画処理を行います。
         */
        void DrawSingleClipSection(AnimatorState& state, std::shared_ptr<AnimatorController>& controller);
        /**
         * @brief 描画処理を行います。
         */
        void DrawBlendTreeSection(AnimatorState& state, std::shared_ptr<AnimatorController>& controller, std::weak_ptr<RuntimeAnimatorController> runtimeController);
        /**
         * @brief 描画処理を行います。
         */
        void DrawBlendSpace2D(AnimatorState& state, std::shared_ptr<AnimatorController>& controller, std::weak_ptr<RuntimeAnimatorController> runtimeController);
        /**
         * @brief 描画処理を行います。
         */
        CurryEngine::Resources::AssetId DrawClipPickerButton(const char* popupId, std::shared_ptr<AnimatorController>& controller, const CurryEngine::Resources::AssetId& currentClipId);
        /**
         * @brief GetClipDisplayName に対応する値を取得します。
         * @return 処理結果を返します。
         */
        std::string GetClipDisplayName(std::shared_ptr<AnimatorController>& controller, const CurryEngine::Resources::AssetId& clipId) const;

        /**
         * @brief 描画処理を行います。
         */
        void DrawTimelineSection(AnimatorState& state, std::shared_ptr<AnimatorController>& controller);
        /**
         * @brief 描画処理を行います。
         */
        CurryEngine::Resources::AssetId DrawTimelinePickerButton(const char* popupId, std::shared_ptr<AnimatorController>& controller, AnimatorState& state);
		/**
		 * @brief GetTimelineDisplayName に対応する値を取得します。
		 * @return 処理結果を返します。
		 */
		std::string GetTimelineDisplayName(std::shared_ptr<AnimatorController>& controller, const CurryEngine::Resources::AssetId& timelineId) const;

        // --- ヘルパー ---
		// 2つのノードの中心座標から、線分がノードの矩形に接する点を計算する
        /**
         * @brief GetNodeEdgePoint に対応する値を取得します。
         * @return 処理結果を返します。
         */
        ImVec2 GetNodeEdgePoint(const ImVec2& fromCenter, const ImVec2& toCenter, const ImVec2& nodeSize) const;
		// 2つのノードの中心座標からオフセットされた2点の線分が接続先のノードの矩形に接する点を計算する
		/**
		 * @brief GetOffsetNodeEdgePoint に対応する値を取得します。
		 * @return 処理結果を返します。
		 */
		ImVec2 GetOffsetNodeEdgePoint(const ImVec2& fromCenter, const ImVec2& toCenter, const ImVec2& nodeSize, float offset) const;

		// 点pから線分abまでの距離を計算する
        /**
         * @brief DistancePointToSegment の処理を行います。
         */
        float DistancePointToSegment(const ImVec2& p, const ImVec2& a, const ImVec2& b) const;
		/**
		 * @brief 状態を更新します。
		 */
		void UpdateTransitionOffsetIndices(std::shared_ptr<AnimatorController>& controller);
    };
}
#endif // USE_IMGUI

