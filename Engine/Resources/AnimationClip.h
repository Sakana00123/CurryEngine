#pragma once
#include "Resource.h"


class GltfModelRenderer;

enum class TrackType : uint8_t
{
	Value, // 値トラック
	Event, // イベントトラック
	Curve, // カーブトラック
	State, // ステートトラック
};

struct AnimationContext
{
	// アニメーション評価に必要なデータをここに追加
	// 例: 対象オブジェクトの参照、ブレンド情報など
	GltfModelRenderer* targetModelRenderer = nullptr;
};

/** @brief キーフレームの基底構造体。*/
struct KeyframeBase
{
	float time = 0.0f; // キーフレームの時間（秒）
};

struct ValueKeyframe : public KeyframeBase
{
	float value = 0.0f; // キーフレームの値
};

struct EventKeyframe : public KeyframeBase
{
	std::string eventName; // イベント名
};

struct CurveKeyframe : public KeyframeBase
{
	float value = 0.0f; // キーフレームの値
	float inTangent = 0.0f; // 入力接線
	float outTangent = 0.0f; // 出力接線
};

struct StateKeyframe : public KeyframeBase
{
	std::string stateName; // ステート名
};


/** @brief アニメーショントラックの基底クラス。*/
class TrackBase
{
public:
	virtual ~TrackBase() = default;

	/** @brief トラックの種類を取得。*/
	virtual TrackType GetType() const = 0;

	/** @brief トラックの評価を行う。*/
	virtual void Evaluate(
		float prevTime,
		float currentTime,
		AnimationContext& context
	) = 0;

	/** @brief トラックの表示名を取得。*/
	virtual const char* GetDisplayName() const = 0;

	std::string name; // トラック名
};

class ValueTrack : public TrackBase
{
public:
	TrackType GetType() const override { return TrackType::Value; }

	void Evaluate(
		float prevTime,
		float currentTime,
		AnimationContext& context
	) override;

	const char* GetDisplayName() const override { return "Value Track"; }

	/** @brief キーフレームを追加し、時間でソートする。*/
	void AddKeyframe(float time, float value)
	{
		keys.push_back({ time, value });
		Sort();
	}

	/** @brief キーフレームを時間でソートする。*/
	void Sort();

	std::vector<ValueKeyframe> keys;
};

class EventTrack : public TrackBase
{
public:
	TrackType GetType() const override { return TrackType::Event; }
	void Evaluate(
		float prevTime,
		float currentTime,
		AnimationContext& context
	) override;
	const char* GetDisplayName() const override { return "Event Track"; }

	std::vector<EventKeyframe> keys;
};


class CurveTrack : public TrackBase
{
public:
	TrackType GetType() const override { return TrackType::Curve; }
	void Evaluate(
		float prevTime,
		float currentTime,
		AnimationContext& context
	) override;
	const char* GetDisplayName() const override { return "Curve Track"; }

	std::vector<CurveKeyframe> keys;
};


class StateTrack : public TrackBase
{
public:
	TrackType GetType() const override { return TrackType::State; }
	void Evaluate(
		float prevTime,
		float currentTime,
		AnimationContext& context
	) override;
	const char* GetDisplayName() const override { return "State Track"; }

	std::vector<StateKeyframe> keys;
};

struct Track
{
	TrackType type;
	std::string name;
	
	std::unique_ptr<ValueTrack> value;
	std::unique_ptr<EventTrack> event;
	std::unique_ptr<CurveTrack> curve;
	std::unique_ptr<StateTrack> state;
};

class AnimationClip/* : public Resource*/
{
public:
	std::string name;
	float length = 0.0f; // アニメーションの長さ（秒）
	float fps = 60.0f;    // フレームレート
	std::vector<Track> tracks;
	//std::vector<std::unique_ptr<TrackBase>> tracks; // トラックの一覧
};