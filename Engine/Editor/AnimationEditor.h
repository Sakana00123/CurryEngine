#pragma once
#include "Engine/Resources/AnimationClip.h"
#include <imgui.h>

struct DragState
{
	bool dragging = false;
	float grabOffsetTime = 0.0f; // 掴んだ位置とキーフレームの時間の差
};

struct SelectedKey
{
	ValueTrack* track = nullptr;
	int keyIndex = -1;

	bool IsValid() const { return track != nullptr && keyIndex >= 0; }
};

enum class SnapMode
{
	None,
	Frame,
	Beat
};

struct SnapSettings
{
	SnapMode mode = SnapMode::None;
	float fps = 60.0f; // フレームスナップ時のフレームレート
	float bpm = 120.0f; // ビートスナップ時のBPM
	int beatsPerMeasure = 4; // ビートスナップ時の小節あたりの拍数
	int subdivisions = 4; // ビートスナップ時の分割数（例: 4 = 1/4拍）
};

// タイムラインビューの設定(未使用)
struct TimelineView
{
	float pixelsPerSecond = 100.0f;
	float minPPS = 20.0f;
	float maxPPS = 500.0f;

	float length = 3.0f; // タイムラインの長さ（秒）
	ImVec2 origin; // タイムラインの描画開始位置
	ImVec2 size;   // タイムラインの描画サイズ
};

class AnimationEditor
{
public:
	/** @brief アニメーションエディタを開く。*/
	static void Open();

	/** @brief アニメーションエディタを閉じる。*/
	static void Close();
	
	/** @brief アニメーションエディタが開いているかどうかを取得。*/
	static bool IsOpen();

	/** @brief 編集中のアニメーションクリップを取得。*/
	static AnimationClip* GetAnimationClip();

	/** @brief 編集中のアニメーションクリップを設定。*/
	static void SetAnimationClip(std::unique_ptr<AnimationClip> clip);

	/** @brief アニメーションエディタの GUI を描画。*/
	static void DrawGUI();

private:

	/** @brief ツールバーを描画。*/
	static void DrawToolbar();

	/** @brief タイムラインエリアを描画。*/
	static void DrawTimelineArea();

	/** @brief トラックリストを描画。*/
	static void DrawTrackList();

	/** @brief タイムラインを描画。*/
	static void DrawTimeline();

	static void DrawKeys(ValueTrack* track, ImVec2 origin, float pixelsPerSecond, float trackY);

	/** @brief キーフレームを描画。*/
	static void DrawKeyframes();

	/** @brief インスペクタを描画。*/
	static void DrawInspector();

	/** @brief 秒線を描画。*/
	static void DrawSecondLines(float length, ImVec2 origin, ImVec2 size, float pixelsPerSecond);

	/** @brief フレームラインを描画。*/
	static void DrawFrameLines(float length, const SnapSettings& snap, ImVec2 origin, ImVec2 size, float pixelsPerSecond);

	/** @brief ビートラインを描画。*/
	static void DrawBeatLines(float length, const SnapSettings& snap, ImVec2 origin, ImVec2 size, float pixelsPerSecond);

	/** @brief マウスがタイムライン内にあるかどうかを判定。*/
	static bool IsMouseInTimeline(ImVec2 origin, ImVec2 size);
	/** @brief マウスがキーフレーム上にあるかどうかを判定。*/
	static bool IsMouseOverKey(ImVec2 keyPos, float radius);
	/** @brief 時間をスナップ。*/
	static float SnapTime(float time, const SnapSettings& snap);
	/** @brief X座標を時間に変換。*/
	static float XToTime(float x, ImVec2 origin, float pixelsPerSecond);
	/** @brief 時間をX座標に変換。*/
	static float TimeToX(float time, ImVec2 origin, float pixelsPerSecond);

	/** @brief ズーム操作を処理。*/
	static void HandleZoom(TimelineView& view);

	/** @brief アニメーションプレビューを描画。*/
	//static void DrawAnimationPreview();

	/** @brief トラック追加メニューを描画。*/
	//static void DrawAddTrackMenu();

	/** @brief 新しいアニメーションクリップを作成。*/
	//static void NewAnimationClip();

	/** @brief ファイルからアニメーションクリップを読み込み。*/
	//static void LoadAnimationClipFromFile();

	/** @brief アニメーションクリップを保存。*/
	//static void SaveAnimationClip();

	/** @brief アニメーションクリップを別名で保存。*/
	//static void SaveAnimationClipAs();

	/** @brief アニメーションクリップを glTF 形式でエクスポート。*/
	//static void ExportAnimationClipToGLTF();

	/** @brief 選択中のトラックを削除。*/
	//static void DeleteSelectedTrack();

	/** @brief 選択中のキーフレームを削除。*/
	//static void DeleteSelectedKeyframe();

	/** @brief アニメーションの再生を制御。*/
	//static void ControlAnimationPlayback();

	/** @brief 選択中のキーフレームの選択を解除。*/
	static void UnselectKey();

	/** @brief 選択中のキーフレームを削除。*/
	static void DeleteSelectedKey();


private:
	static inline bool isOpen = false;
	static inline std::unique_ptr<AnimationClip> animationClip;

	static inline bool isPlaying = false;
	static inline bool isLooping = false;
	static inline float pixelsPerSecond = 100.0f;

	static inline float currentTime = 0.0f;
	static inline float length = 3.0f;

	static inline TimelineView timelineView;// 未使用
	
	static inline DragState dragState;

	static inline ValueKeyframe* draggingKey = nullptr;

	
	static inline SelectedKey selectedKey;

	
	static inline SnapSettings snapSettings;
};