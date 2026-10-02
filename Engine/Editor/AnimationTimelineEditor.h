#pragma once
#include "Engine/Resources/AnimationEvent.h"
#include <memory>
#include <optional>
#ifdef USE_IMGUI
#include <imgui.h>
#include <Engine\Rendering\Pipeline\RenderContext.h>
#include "Engine/Core/ObjectId.h"
#include "Engine/Resources/AnimatorController.h"


namespace CurryEngine::Editor
{
    /** @brief AnimationTimelineEditor を表すクラスです。 */
    class AnimationTimelineEditor
    {
    public:
		AnimationTimelineEditor() { ResetStateFlags(); }

        void SetTarget(std::shared_ptr<Resources::AnimationTimeline> timeline) { m_timeline = timeline; m_selectedKey.reset(); }
        /**
         * @brief 描画処理を行います。
         */
        void Draw(std::weak_ptr<RuntimeAnimatorController> runtimeController, RenderContext* context); // ImGuiウィンドウ内から呼ぶ

		/**
		 * @brief 描画処理を行います。
		 */
		void RenderPreview(RenderContext* context); // プレビュー用の描画処理

		// プレビューウィンドウがフォーカスされているかどうかを取得
		bool IsPreviewFocused() const { return isPreviewFocused; }
    private:
        struct KeySelection { size_t trackIndex; size_t keyIndex; bool isDragging = false; bool acceptDrag = false; };

        /**
         * @brief 描画処理を行います。
         */
        void DrawToolbar(std::weak_ptr<RuntimeAnimatorController> runtimeController);
        /**
         * @brief 描画処理を行います。
         */
        void DrawTrackList();
        /**
         * @brief 描画処理を行います。
         */
        void DrawTimelineArea(std::weak_ptr<RuntimeAnimatorController> runtimeController);
        /**
         * @brief 描画処理を行います。
         */
        void DrawRuler(ImDrawList* dl, ImVec2 origin, float width);
        /**
         * @brief 描画処理を行います。
         */
        void DrawTrackRow(ImDrawList* dl, ImVec2 rowOrigin, float width, size_t trackIndex);
        /**
         * @brief TimeToX の処理を行います。
         */
        float TimeToX(float time, float originX, float width) const;
        /**
         * @brief XToTime の処理を行います。
         */
        float XToTime(float x, float originX, float width) const;

        std::shared_ptr<Resources::AnimationTimeline> m_timeline = nullptr;
        std::optional<KeySelection> m_selectedKey;
        float currentTime = 0.0f;
		float prevTime = 0.0f;
        float m_pixelsPerSecond = 150.0f;
        //ObjectId selectedAnimatorId;

		uint64_t m_states = 0; // ビットフラグで状態を管理するための変数
        /** @brief StateFlags を表す列挙型です。 */
        enum class StateFlags : uint64_t
        {
			IsPlaying,
			IsLooping,
			IsFireEventEnabled,
            MousePressed,
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
