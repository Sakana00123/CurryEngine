#pragma once
#include "Resource.h"
#include "AssetId.h"
#include <any>
#include <vector>
#include <string>
#include "Engine/Core/Math/Vector3.h"

namespace CurryEngine::Resources
{
    /** @brief AnimationEventType を表す列挙型です。 */
    enum class AnimationEventType
    {
        Custom,
		SoundEffect,
		ParticleEffect,
	};

    /** @brief SoundEffectEventParam を表す構造体です。 */
    struct SoundEffectEventParam
    {
        std::string soundAssetId; // サウンドのアセットID
		float volume = 1.0f; // 音量 (0.0f ~ 1.0f)
	};

    /** @brief ParticleEffectEventParam を表す構造体です。 */
    struct ParticleEffectEventParam
    {
		std::string particleAssetId; // パーティクルのアセットID
		int targetNodeId = 0; // パーティクルを再生する対象ノードのID
		Vector3 offset = Vector3::Zero; // パーティクルの再生位置のオフセット
	};

    /** @brief AnimationEventKey を表す構造体です。 */
    struct AnimationEventKey
    {
        float time = 0.0f;
        std::string eventName = "";
		std::string paramType = ""; // 引数の型情報
		std::any paramValue; // 引数の値
    };

    /** @brief AnimationEventTrack を表す構造体です。 */
    struct AnimationEventTrack
    {
        std::string name;
        AnimationEventType type = AnimationEventType::Custom;
        std::vector<AnimationEventKey> keys{};
    };

    // 将来のカーブトラック追加時、共通の基底 or variant で
    // TrackType を持たせて統一的に扱えるようにする想定
    /** @brief TrackType を表す列挙型です。 */
    enum class TrackType
    {
        Event,
        FloatCurve,  // 将来: 速度カーブなど
    };

    /** @brief AnimationTimeline を表すクラスです。 */
    class AnimationTimeline : public Resource
    {
    public:
        /**
         * @brief AnimationTimeline を構築します。
         */
        AnimationTimeline() = default;

		// Resource interface
		/**
		 * @brief LoadFromFile に対応する値を取得します。
		 * @return 処理結果を返します。
		 */
		bool LoadFromFile(const std::string& path) override;

        // 保存
		/**
		 * @brief SaveToFile の処理を行います。
		 */
		bool SaveToFile(const std::filesystem::path& path) const;


        const AssetId& GetTargetClip() const { return m_targetClip; }
        void SetTargetClip(const AssetId& clipId) { m_targetClip = clipId; }

        float GetDuration() const { return m_duration; }
        void SetDuration(float duration) { m_duration = duration; }

        std::vector<AnimationEventTrack>& GetEventTracks() { return m_eventTracks; }
        const std::vector<AnimationEventTrack>& GetEventTracks() const { return m_eventTracks; }

        /**
         * @brief AddEventTrack の処理を行います。
         */
        AnimationEventTrack& AddEventTrack(const std::string& name);
        /**
         * @brief 指定された要素を削除します。
         */
        void RemoveEventTrack(size_t index);

    private:
        AssetId m_targetClip;
        float m_duration = 0.0f;
        std::vector<AnimationEventTrack> m_eventTracks;
    };



    /** @brief FiredAnimationEvent を表す構造体です。 */
    struct FiredAnimationEvent
    {
		AnimationEventType type;
        std::string eventName;
		std::string paramType; // 引数の型情報
		std::any paramValue; // 引数の値
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
                            outEvents.push_back({ track.type, key.eventName, key.paramType, key.paramValue });
            };

        if (looped) { checkRange(prevTime, timeline.GetDuration()); checkRange(0.0f, currentTime); }
        else { checkRange(prevTime, currentTime); }
    }
}
