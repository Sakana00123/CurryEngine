#pragma once
#include "Engine/Resources/AnimationEvent.h"
#include <memory>
#include <optional>
#ifdef USE_IMGUI
#include <imgui.h>
#include <Engine\Rendering\Pipeline\RenderContext.h>


namespace CurryEngine::Editor
{
    class AnimationTimelineEditor
    {
    public:
		AnimationTimelineEditor() { ResetStateFlags(); }

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
        float currentTime = 0.0f;
		float prevTime = 0.0f;
        float m_pixelsPerSecond = 150.0f;

		uint64_t m_states = 0; // ビットフラグで状態を管理するための変数
        enum class StateFlags : uint64_t
        {
			IsPlaying,
			IsLooping,
			IsFireEventEnabled,
            PrevMousePressed,
            IsPressingMouseOnRuler,
		};

		// ビットフラグの設定
        void SetStateFlag(StateFlags flag, bool value)
        {
            if (value)
                m_states |= (1Ui64 << static_cast<uint64_t>(flag)); // ビットを立てる
            else
                m_states &= ~(1Ui64 << static_cast<uint64_t>(flag)); // ビットを下げる
		}
		// ビットフラグの取得
		bool GetStateFlag(StateFlags flag) const { return (m_states & (1Ui64 << static_cast<uint64_t>(flag))) != 0; }

		// 状態フラグをリセットしてデフォルト値を設定
        void ResetStateFlags()
        {
            // フラグをリセット
            m_states = 0;
            // デフォルト値を設定
            SetStateFlag(StateFlags::IsLooping, true);
            SetStateFlag(StateFlags::IsFireEventEnabled, true);
        }

		bool isPreviewFocused = false;
        static constexpr float kTrackHeight = 28.0f;
        static constexpr float kLabelWidth = 140.0f;
    };
}
#endif // USE_IMGUI
