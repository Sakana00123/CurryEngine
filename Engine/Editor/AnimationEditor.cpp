#include "pch.h"
#include "AnimationEditor.h"
#include "Engine/Resources/AssetDatabase.h"
#include "Engine/Resources/AnimationClip.h"

// ------------------------------- メンバ関数群 ----------------------------------------

void AnimationEditor::Open()
{
	isOpen = true;
}

void AnimationEditor::Close()
{
	isOpen = false;
}

bool AnimationEditor::IsOpen()
{
	return isOpen;
}

AnimationClip* AnimationEditor::GetAnimationClip()
{
	return animationClip.get();
}

void AnimationEditor::SetAnimationClip(std::shared_ptr<AnimationClip> clip)
{
	animationClip = clip;
	if (animationClip)
	{
		length = animationClip->duration;
		currentTime = 0.0f;
	}
}

void AnimationEditor::OpenAsset(const std::filesystem::path& path)
{
	auto meta = CurryEngine::Resources::AssetDatabase::GetOrImport(path);
	if (!meta) 
	{
		LOG_ERROR("Failed to get or import asset meta: " + path.string());
		return;
	}
	auto& id = meta->id;
	auto clip = CurryEngine::Resources::AssetDatabase::LoadAsset<AnimationClip>(id);
	if (!clip)
	{
		LOG_ERROR("Failed to load animation asset: " + path.string());
		return;
	}
	assetPath = path;
	SetAnimationClip(clip);
	Open();
}

// ------------------------------- 汎用関数 ----------------------------------------
// 時間をX座標に変換
float AnimationEditor::XToTime(float x, ImVec2 origin, float pixelsPerSecond)
{
	return (x - origin.x) / pixelsPerSecond;
}

// X座標を時間に変換
float AnimationEditor::TimeToX(float time, ImVec2 origin, float pixelsPerSecond)
{
	return origin.x + time * pixelsPerSecond;
}

namespace
{
	// 1拍あたりの時間を取得
	float BeatTime(const SnapSettings & snap)
	{
		return 60.0f / snap.bpm;
	}

	// 1つの分割あたりの時間を取得
	float SubDivisionTime(const SnapSettings & snap)
	{
		return BeatTime(snap) / static_cast<float>(snap.subdivisions);
	}
}

// 時間をスナップ
float AnimationEditor::SnapTime(float time, const SnapSettings& snap)
{
	switch (snap.mode)
	{
	case SnapMode::Frame:
	{
		float frameDuration = 1.0f / snap.fps;
		return std::round(time / frameDuration) * frameDuration;
	}
	case SnapMode::Beat:
	{
		float divisionDuration = SubDivisionTime(snap);
		return std::round(time / divisionDuration) * divisionDuration;
	}
	default:
		return time;
	}
}


// マウスがタイムライン内にあるかどうかを判定
bool AnimationEditor::IsMouseInTimeline(ImVec2 origin, ImVec2 size)
{
	ImVec2 mousePos = ImGui::GetIO().MousePos;
	return mousePos.x >= origin.x && mousePos.x <= origin.x + size.x &&
		mousePos.y >= origin.y && mousePos.y <= origin.y + size.y;
}

// マウスがキーフレーム上にあるかどうかを判定
bool AnimationEditor::IsMouseOverKey(ImVec2 keyPos, float radius)
{
	ImVec2 mousePos = ImGui::GetIO().MousePos;
	float dx = mousePos.x - keyPos.x;
	float dy = mousePos.y - keyPos.y;
	return (dx * dx + dy * dy) <= (radius * radius);
}

// TODO: 未使用関数
void AnimationEditor::HandleZoom(TimelineView& view)
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.MouseWheel != 0.0f && IsMouseInTimeline(view.origin, view.size))
	{
		// ズームの中心をマウス位置に設定
		float mouseTime = XToTime(io.MousePos.x, view.origin, view.pixelsPerSecond);
		// ズーム倍率を計算
		float zoomFactor = 1.0f + io.MouseWheel * 0.1f; // ホイール1回転あたり10%のズーム
		// 新しい pixelsPerSecond を計算
		float newPPS = view.pixelsPerSecond * zoomFactor;
		// 最小・最大制限を適用
		newPPS = std::clamp(newPPS, view.minPPS, view.maxPPS);
		// pixelsPerSecond を更新
		view.pixelsPerSecond = newPPS;
		// ズーム中心を維持するために origin を調整
		float newOriginX = io.MousePos.x - mouseTime * view.pixelsPerSecond;
		view.origin.x = newOriginX;
	}
}

// ------------------------------- メイン関数群 ----------------------------------------

void AnimationEditor::DrawGUI()
{
	// GUI 描画ロジックをここに実装
	ImGui::Begin("Animation Editor", &isOpen);

	// 初期化
	if (!animationClip)
	{
		// 初期化処理
		animationClip = std::make_unique<AnimationClip>();
		animationClip->duration = 3.0f;
		//animationClip->samplingRate = 60.0f;
#if 0
		animationClip->tracks.clear();
		for (int i = 0; i < 3; ++i)
		{
			// 仮に3つのトラックを追加
			// 実際には具体的なトラッククラスを実装して追加する必要があります
			Track track;
			track.type = TrackType::Value;
			track.name = "Value Track " + std::to_string(i + 1);
			track.value = std::make_unique<ValueTrack>();
			for (int k = 0; k <= 10; ++k)
			{
				ValueKeyframe key;
				key.time = static_cast<float>(k) * 0.3f; // 0.0, 0.3, 0.6, ..., 3.0
				key.value = static_cast<float>(k); // 0, 1, 2, ..., 10
				track.value->keys.push_back(key);
			}
			animationClip->tracks.push_back(std::move(track));
		}
#endif // 0


		length = animationClip->duration; // デフォルトのアニメーション長さ
		currentTime = 0.0f;
	}

	// ツールバー
	DrawToolbar();

	ImGui::Separator();

	// タイムラインエリア
	DrawTimelineArea();

	ImGui::Separator();

	// インスペクタ
	DrawInspector();
	
	ImGui::End();
}


void AnimationEditor::DrawToolbar()
{
	if (ImGui::Button("Save") && animationClip)
	{
		if (assetPath.empty() || !animationClip->SaveToFile(assetPath))
		{
			LOG_ERROR("Failed to save animation asset.");
		}
	}
	ImGui::SameLine();
	ImGui::TextDisabled("%s", assetPath.empty() ? "Untitled" : assetPath.filename().string().c_str());
	ImGui::SameLine();
	// ツールバー描画ロジックをここに実装

	// 再生制御
	if (isPlaying)
	{
		// 再生中の場合、時間を進める
		currentTime += ImGui::GetIO().DeltaTime;

		// アニメーションの長さを超えた場合の処理
		if (currentTime > length)
		{
			if (isLooping)
			{
				currentTime = 0.0f; // ループする場合は時間をリセット
			}
			else
			{
				currentTime = length; // 停止する場合は最大時間に固定
				isPlaying = false;    // 再生停止
			}
		}
	}

	if (ImGui::Button("Play"))
	{
		// 再生ボタンが押されたときの処理
		isPlaying = true;
	}
	ImGui::SameLine();

	if (ImGui::Button("Pause"))
	{
		// 一時停止ボタンが押されたときの処理
		isPlaying = false;
	}

	ImGui::SameLine();
	if (ImGui::Button("Stop"))
	{
		// 停止ボタンが押されたときの処理
		currentTime = 0.0f; // 時間をリセット
		isPlaying = false;
	}

	ImGui::SameLine();
	// ループ切替
	ImGui::Checkbox("Loop", &isLooping);

	ImGui::SameLine();

	// スナップモード選択
	ImGui::SetNextItemWidth(100.0f);
	const char* snapModeNames[] = { "None", "Frame", "Beat" };
	int currentSnapMode = static_cast<int>(snapSettings.mode);
	if (ImGui::Combo("Snap Mode", &currentSnapMode, snapModeNames, IM_ARRAYSIZE(snapModeNames)))
	{
		snapSettings.mode = static_cast<SnapMode>(currentSnapMode);
	}
	switch (snapSettings.mode)
	{
	case SnapMode::Frame:
		{
			ImGui::SameLine();
			ImGui::SetNextItemWidth(75.0f);
			ImGui::DragFloat("##FPS", &snapSettings.fps, 1.0f, 1.0f, 5000.0f, "FPS: %.2f");
			break;
		}
	case SnapMode::Beat:
		{
			ImGui::SameLine();
			ImGui::SetNextItemWidth(75.0f);
			ImGui::DragFloat("##BPM", &snapSettings.bpm, 5.0f, 1.0f, FLT_MAX, "BPM: %.2f");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(75.0f);
			static const char* divisions[] = { "1/1", "1/2", "1/4", "1/8", "1/16" };
			static constexpr int divisionValues[] = { 1, 2, 4, 8, 16 };
			int currentDivisionIndex = 2; // デフォルトは1/4
			// 現在の分割数に対応するインデックスを見つける
			for (size_t i = 0; i < IM_ARRAYSIZE(divisionValues); ++i)
			{
				if (divisionValues[i] == snapSettings.subdivisions)
				{
					currentDivisionIndex = static_cast<int>(i);
					break;
				}
			}
			// コンボボックスで分割数を選択
			if (ImGui::Combo("##SubDivisions", &currentDivisionIndex, divisions, IM_ARRAYSIZE(divisions)))
			{
				// 選択された分割数を設定
				snapSettings.subdivisions = divisionValues[currentDivisionIndex];
			}
			break;
		}
	default:
		break;
	}

	ImGui::SameLine();

	// 時間スライダー
	ImGui::SetNextItemWidth(100.0f);
	ImGui::SliderFloat(
		"Time",
		&currentTime, // 現在の時間を指すポインタを指定
		0.0f,
		length
	);

	ImGui::SameLine();
	ImGui::Text("%.2f / %.2f sec",
		currentTime,
		length
	);
}

void AnimationEditor::DrawTimelineArea()
{
	// タイムラインエリア描画ロジックをここに実装
	ImVec2 size = ImGui::GetContentRegionAvail();

	float trackListWidth = 150.0f; // トラックリストの幅

	ImGui::BeginChild("TrackList", ImVec2(trackListWidth, size.y), true);
	DrawTrackList();
	ImGui::EndChild();

	ImGui::SameLine();

	// タイムラインとキーフレーム描画
	ImGui::BeginChild("Timeline", ImVec2(0, size.y), true);
	
	DrawTimeline();

	ImGui::EndChild();
}

void AnimationEditor::DrawTrackList()
{
#if 0
	int i = 0;
	// トラックリスト描画ロジックをここに実装
	for (const auto& track : animationClip->tracks)
	{
		ImGui::PushID(i++);
		bool isSelected = ImGui::Selectable(track.name.c_str(), selectedKey.track == (track.value.get()));

		if (isSelected)
		{
			// トラックが選択されたときの処理
			selectedKey.track = (track.value.get());
			selectedKey.keyIndex = -1; // キーフレームは未選択にする
		}
		ImGui::PopID();
	}
#endif // 0

}

void AnimationEditor::DrawTimeline()
{
	// タイムライン描画ロジックをここに実装

	ImDrawList* draw = ImGui::GetWindowDrawList();
	ImVec2 origin = ImGui::GetCursorScreenPos();
	ImVec2 size = ImGui::GetContentRegionAvail();

	// 背景
	draw->AddRectFilled(
		origin,
		ImVec2(origin.x + size.x, origin.y + size.y),
		IM_COL32(30, 30, 30, 255)
	);

#if 0
	// 時間目盛り
	for (int i = 0; static_cast<float>(i) <= length; ++i)
	{
		const float x = origin.x + static_cast<float>(i) * pixelsPerSecond;
		draw->AddLine(
			ImVec2(x, origin.y),
			ImVec2(x, origin.y + size.y),
			IM_COL32(80, 80, 80, 255)
		);
	}
#else
	// 時間目盛り（スナップ設定に応じて描画）
	switch (snapSettings.mode)
	{
	case SnapMode::Frame:
		DrawFrameLines(length, snapSettings, origin, size, pixelsPerSecond);
		break;
	case SnapMode::Beat:
		DrawBeatLines(length, snapSettings, origin, size, pixelsPerSecond);
		break;
	default:
		DrawSecondLines(length, origin, size, pixelsPerSecond);
		break;
	}

#endif // 0


	// クリック処理
	if (IsMouseInTimeline(origin, size))
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			// クリック位置から時間を計算して設定
			float t = XToTime(ImGui::GetIO().MousePos.x, origin, pixelsPerSecond);
			currentTime = std::clamp(t, 0.0f, length);
		}
	}

	// 現在時間カーソル
	float cursorX = origin.x + currentTime * pixelsPerSecond;
	draw->AddLine(
		ImVec2(cursorX, origin.y),
		ImVec2(cursorX, origin.y + size.y),
		IM_COL32(255, 0, 0, 255),
		2.0f
	);

	// 各トラックのキーフレームを描画
	DrawKeys(selectedKey.track, origin, pixelsPerSecond, origin.y + size.y / 2.0f);

}

void AnimationEditor::DrawKeys(ValueTrack* track, ImVec2 origin, float pixelsPerSecond, float trackY)
{
	// キーフレーム描画ロジックをここに実装

	// トラックが存在しない場合は何もしない
	if (!track) return;

	ImDrawList* draw = ImGui::GetWindowDrawList();
	static constexpr float radius = 5.0f;

	// いずれかのキーフレームがホバーされているか
	bool hoveredAnyKey = false;
	bool clickedOnKey = false;

	// 各キーフレームを描画
	for (int i = 0; i < track->keys.size(); ++i)
	{
		// キーフレームの位置計算
		auto& key = track->keys[i];

		ImVec2 keyPos;
		keyPos.x = origin.x + key.time * pixelsPerSecond;
		keyPos.y = trackY;

		bool hovered = IsMouseOverKey(keyPos, radius);
		hoveredAnyKey |= hovered; // いずれかのキーフレームがホバーされているか
		bool clicked = hovered &&
			ImGui::IsMouseClicked(ImGuiMouseButton_Left);

		// ホバー時の処理
		if (hovered && !dragState.dragging)
		{
			// ホバー時のツールチップ表示
			ImGui::SetTooltip("Time: %.2f\nValue: %.2f", key.time, key.value);
		}

		// クリックで選択
		if (clicked)
		{
			selectedKey.track = track;
			selectedKey.keyIndex = i;
			clickedOnKey = true;
		}

		// 見た目
		const bool selected =
			(selectedKey.track == track &&
				selectedKey.keyIndex == i);

		const ImU32 color =
			selected ? IM_COL32(255, 200, 0, 255) :
			hovered ? IM_COL32(220, 220, 150, 255) :
			IM_COL32(200, 200, 100, 255);
		// キーフレームの描画
		draw->AddCircleFilled(keyPos, radius, color);

		// ドラッグ開始処理
		if (selected && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			dragState.dragging = true;
			draggingKey = &key;

			float mouseTime =
				XToTime(ImGui::GetIO().MousePos.x, origin, pixelsPerSecond);

			dragState.grabOffsetTime = mouseTime - key.time;
		}
		// ドラッグ処理
		if (dragState.dragging && draggingKey == &key &&
			ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			float mouseTime =
				XToTime(ImGui::GetIO().MousePos.x, origin, pixelsPerSecond);

			// スナップ処理
			mouseTime = SnapTime(mouseTime, snapSettings);

			if (snapSettings.mode != SnapMode::None)
			{
				// スナップが有効な場合、オフセットをリセット
				dragState.grabOffsetTime = 0.0f;
			}

			// キーフレームの時間を更新
			draggingKey->time =
				std::clamp(
					mouseTime - dragState.grabOffsetTime,
					0.0f,
					length
				);
		}
		// ドラッグ終了処理
		if (dragState.dragging && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			dragState.dragging = false;

			// 本体をソート
			track->Sort();

			// 選択インデックスを再取得
			for (int j = 0; j < track->keys.size(); ++j)
			{
				if (&track->keys[j] == draggingKey)
				{
					selectedKey.keyIndex = j;
					break;
				}
			}

			draggingKey = nullptr;
		}
	}

	// ダブルクリックで新しいキーフレームを追加
	if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
	{
		float mouseX = ImGui::GetIO().MousePos.x;
		float time = XToTime(mouseX, origin, pixelsPerSecond);
		// 新しいキーフレームを追加
		float value = 0.0f; // デフォルト値
		track->AddKeyframe(std::clamp(time, 0.0f, length), value);
	}
	else if (!hoveredAnyKey && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !clickedOnKey)
	{
		// キーフレーム以外の場所がクリックされた場合、キーフレーム選択を解除
		UnselectKey();
	}

	// Delete キーで選択中のキーフレームを削除
	if (ImGui::Shortcut(ImGuiKey_Delete))
	{
		DeleteSelectedKey();
	}

	//if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	//{
	//	// 右クリックメニュー
	//	ImGui::OpenPopup("KeyframeContextMenu");

	//	// コンテキストメニューの内容
	//	if (ImGui::BeginPopup("KeyframeContextMenu"))
	//	{
	//		if (selectedKey.IsValid())
	//		{
	//			if (ImGui::MenuItem("Delete", "Del"))
	//			{
	//				DeleteSelectedKey();
	//			}
	//			if (ImGui::MenuItem("Unselect"))
	//			{
	//				UnselectKey();
	//			}
	//		}
	//		ImGui::EndPopup();
	//	}
	//}
}


void AnimationEditor::DrawKeyframes()
{
	// キーフレーム描画ロジックをここに実装
	ImDrawList* draw = ImGui::GetWindowDrawList();
	ImVec2 origin = ImGui::GetCursorScreenPos();
	ImVec2 size = ImGui::GetContentRegionAvail();

	
#if 0
	for (const auto& track : animationClip->tracks)
	{
		if (auto valueTrack = (track.value.get()))
		{
			// 各トラックのキーフレームを描画
			for (const auto& keyframe : valueTrack->keys)
			{
				// キーフレームの描画処理
				float x = origin.x + keyframe.time * pixelsPerSecond;
				float y = origin.y + 20.0f; // 仮のY位置

				draw->AddCircleFilled(
					ImVec2(x, y),
					5.0f,
					IM_COL32(200, 200, 100, 255)
				);
			}
		}
	}
#endif // 0

}

void AnimationEditor::DrawInspector()
{
	ImGui::PushID("AnimationInspector");
	if (selectedKey.IsValid())
	{
		auto& key = selectedKey.track->keys[selectedKey.keyIndex];
		ImGui::Text("Selected Keyframe:");
		ImGui::DragFloat("Time", &key.time, 0.01f, 0.0f, length);
		ImGui::DragFloat("Value", &key.value, 0.1f);
	}
	else
	{
		ImGui::Text("No selection");
	}
	ImGui::PopID();
}

void AnimationEditor::DrawSecondLines(float length, ImVec2 origin, ImVec2 size, float pixelsPerSecond)
{
	// 秒ライン描画ロジックをここに実装
	ImDrawList* draw = ImGui::GetWindowDrawList();
	int secondCount = static_cast<int>(length);
	// 秒ラインの描画
	for (int i = 0; i <= secondCount; ++i)
	{
		const float x = TimeToX(static_cast<float>(i), origin, pixelsPerSecond);
		draw->AddLine(
			ImVec2(x, origin.y),
			ImVec2(x, origin.y + size.y),
			IM_COL32(100, 100, 100, 80)
		);
	}
}


void AnimationEditor::DrawFrameLines(float length, const SnapSettings& snap, ImVec2 origin, ImVec2 size, float pixelsPerSecond)
{
	// フレームライン描画ロジックをここに実装
	ImDrawList* draw = ImGui::GetWindowDrawList();
	float frameTime = 1.0f / snap.fps;
	int frameCount = static_cast<int>(length / frameTime);

	// フレームラインの描画
	for (int i = 0; i <= frameCount; ++i)
	{
		const float t = static_cast<float>(i) * frameTime;
		const float x = TimeToX(t, origin, pixelsPerSecond);

		draw->AddLine(
			ImVec2(x, origin.y),
			ImVec2(x, origin.y + size.y),
			IM_COL32(100, 100, 100, 80)
		);
	}
}

void AnimationEditor::DrawBeatLines(float length, const SnapSettings& snap, ImVec2 origin, ImVec2 size, float pixelsPerSecond)
{
	// ビートライン描画ロジックをここに実装
	ImDrawList* draw = ImGui::GetWindowDrawList();
	float beatTime = BeatTime(snap);
	float divisionTime = SubDivisionTime(snap);
	int divisionCount = static_cast<int>(length / divisionTime);
	
	// ビートラインの描画
	for (int i = 0; i <= divisionCount; ++i)
	{
		const float t = static_cast<float>(i) * divisionTime;
		const float x = TimeToX(t, origin, pixelsPerSecond);

		bool isBeat = (i % snap.subdivisions) == 0;
		bool isMeasure = (i % (snap.subdivisions * snap.beatsPerMeasure)) == 0;

		const ImU32 color =
			isMeasure ?
			IM_COL32(150, 100, 100, 200) :
			isBeat ?
			IM_COL32(150, 100, 100, 100) :
			IM_COL32(100, 100, 100, 80);

		const float lineThickness =
			isMeasure ?
			3.0f :
			isBeat ?
			2.0f :
			1.0f;

		draw->AddLine(
			ImVec2(x, origin.y),
			ImVec2(x, origin.y + size.y),
			color,
			lineThickness
		);
	}
}

void AnimationEditor::UnselectKey()
{
	// 選択中のキーフレームの選択を解除
	selectedKey.keyIndex = -1;
	draggingKey = nullptr;
}

void AnimationEditor::DeleteSelectedKey()
{
	// 選択中のキーフレームを削除
	if (selectedKey.IsValid())
	{
		auto& keys = selectedKey.track->keys;
		keys.erase(keys.begin() + selectedKey.keyIndex);
		UnselectKey(); // 選択解除
	}
}
