#pragma once
#include "Engine/Resources/AnimationEvent.h"
#include <optional>
#ifdef USE_IMGUI
#include <imgui.h>
#include <Engine\Rendering\Pipeline\RenderContext.h>


namespace CurryEngine::Editor
{
    class AnimationTimelineEditor
    {
    public:
        void SetTarget(std::shared_ptr<Resources::AnimationTimeline> timeline) { m_timeline = timeline; m_selectedKey.reset(); }
        void Draw(RenderContext* context); // ImGuiウィンドウ内から呼ぶ

		void RenderPreview(RenderContext* context); // プレビュー用の描画処理

		// プレビューウィンドウがフォーカスされているかどうかを取得
		bool IsPreviewFocused() const { return isPreviewFocused; }
    private:
        struct KeySelection { size_t trackIndex; size_t keyIndex; bool isDragging = false; bool acceptDrag = false; };

        void DrawToolbar();
        void DrawTrackList();
        void DrawTimelineArea();
        void DrawRuler(ImDrawList* dl, ImVec2 origin, float width);
        void DrawTrackRow(ImDrawList* dl, ImVec2 rowOrigin, float width, size_t trackIndex);
        float TimeToX(float time, float originX, float width) const;
        float XToTime(float x, float originX, float width) const;

        std::shared_ptr<Resources::AnimationTimeline> m_timeline = nullptr;
        std::optional<KeySelection> m_selectedKey;
        float m_playhead = 0.0f;
        float m_pixelsPerSecond = 150.0f;
		bool m_isPlaying = false;
		bool prevMousePressed = false;
		bool isPreviewFocused = false;
        static constexpr float kTrackHeight = 28.0f;
        static constexpr float kLabelWidth = 140.0f;
    };
}
#endif // USE_IMGUI
