#pragma once
#include "Resource.h"
#include "AssetId.h"
#include <vector>
#include <string>

namespace CurryEngine::Resources
{
    struct AnimationEventKey
    {
        float time = 0.0f;
        std::string eventName = "";
        std::string stringParam = "";  // 任意: SEアセット名など
    };

    struct AnimationEventTrack
    {
        std::string name;
        std::vector<AnimationEventKey> keys{};
    };

    // 将来のカーブトラック追加時、共通の基底 or variant で
    // TrackType を持たせて統一的に扱えるようにする想定
    enum class TrackType
    {
        Event,
        FloatCurve,  // 将来: 速度カーブなど
    };

    class AnimationTimeline : public Resource
    {
    public:
        AnimationTimeline() = default;

		// Resource interface
		bool LoadFromFile(const std::string& path) override;

        // 保存
		bool SaveToFile(const std::filesystem::path& path) const;


        const AssetId& GetTargetClip() const { return m_targetClip; }
        void SetTargetClip(const AssetId& clipId) { m_targetClip = clipId; }

        float GetDuration() const { return m_duration; }
        void SetDuration(float duration) { m_duration = duration; }

        std::vector<AnimationEventTrack>& GetEventTracks() { return m_eventTracks; }
        const std::vector<AnimationEventTrack>& GetEventTracks() const { return m_eventTracks; }

        AnimationEventTrack& AddEventTrack(const std::string& name);
        void RemoveEventTrack(size_t index);

    private:
        AssetId m_targetClip;
        float m_duration = 0.0f;
        std::vector<AnimationEventTrack> m_eventTracks;
    };



    struct FiredAnimationEvent
    {
        std::string eventName;
        std::string stringParam;
    };

    // [prevTime, currentTime] を跨いだイベントキーを収集する。
    // looped==true の場合、currentTime < prevTime を「1周して巻き戻った」とみなし
    // [prevTime, duration] と [0, currentTime] の2区間として扱う。
    inline void CollectFiredEvents(
        const AnimationTimeline& timeline,
        float prevTime, float currentTime, bool looped,
        std::vector<FiredAnimationEvent>& outEvents)
    {
        auto checkRange = [&](float from, float to)
            {
                for (const auto& track : timeline.GetEventTracks())
                    for (const auto& key : track.keys)
                        if (key.time > from && key.time <= to)
                            outEvents.push_back({ key.eventName, key.stringParam });
            };

        if (looped) { checkRange(prevTime, timeline.GetDuration()); checkRange(0.0f, currentTime); }
        else { checkRange(prevTime, currentTime); }
    }
}
