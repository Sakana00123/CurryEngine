#include "pch.h"
#include "AnimationTimelineEditor.h"
#ifdef USE_IMGUI
#include <imgui_internal.h>

namespace CurryEngine::Editor
{
    float AnimationTimelineEditor::TimeToX(float time, float originX, float) const
    {
        return originX + time * m_pixelsPerSecond;
    }

    float AnimationTimelineEditor::XToTime(float x, float originX, float) const
    {
        return (std::max)(0.0f, (x - originX) / m_pixelsPerSecond);
    }

    void AnimationTimelineEditor::Draw()
    {
        if (!m_timeline) { ImGui::TextDisabled("No Select"); return; }

        DrawToolbar();
        ImGui::Separator();

        ImGui::BeginChild("TimelineScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        DrawTimelineArea();
        ImGui::EndChild();
    }

    void AnimationTimelineEditor::DrawToolbar()
    {
        if (ImGui::Button("+ Track"))
        {
            m_timeline->AddEventTrack("New Track");
        }
        ImGui::SameLine();
        ImGui::PushItemWidth(80);
        ImGui::DragFloat("Playhead", &m_playhead, 0.01f, 0.0f, m_timeline->GetDuration());
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
        ImGui::DragFloat("Zoom", &m_pixelsPerSecond, 1.0f, 20.0f, 1000.0f);
        ImGui::PopItemWidth();
		ImGui::SameLine();
        if (ImGui::Button("Save##TimelineEditor"))
        {
			m_timeline->SaveToFile(m_timeline->GetPath());
        }
    }

    void AnimationTimelineEditor::DrawRuler(ImDrawList* dl, ImVec2 origin, float width)
    {
        const float duration = m_timeline->GetDuration();
        dl->AddRectFilled(origin, ImVec2(origin.x + width, origin.y + 20), IM_COL32(40, 40, 40, 255));

        for (float t = 0.0f; t <= duration + 0.001f; t += 0.5f)
        {
            float x = TimeToX(t, origin.x, width);
            dl->AddLine(ImVec2(x, origin.y), ImVec2(x, origin.y + 20), IM_COL32(120, 120, 120, 255));
            char buf[16]; snprintf(buf, sizeof(buf), "%.1fs", t);
            dl->AddText(ImVec2(x + 2, origin.y + 2), IM_COL32(200, 200, 200, 255), buf);
        }

        // プレイヘッド
        float px = TimeToX(m_playhead, origin.x, width);
        dl->AddLine(ImVec2(px, origin.y), ImVec2(px, origin.y + 20 + kTrackHeight * m_timeline->GetEventTracks().size()),
            IM_COL32(255, 60, 60, 255), 2.0f);
    }

    void AnimationTimelineEditor::DrawTrackRow(ImDrawList* dl, ImVec2 rowOrigin, float width, size_t trackIndex)
    {
        auto& track = m_timeline->GetEventTracks()[trackIndex];

        // 背景(縞模様)
        ImU32 bg = (trackIndex % 2 == 0) ? IM_COL32(50, 50, 50, 255) : IM_COL32(45, 45, 45, 255);
        dl->AddRectFilled(rowOrigin, ImVec2(rowOrigin.x + width, rowOrigin.y + kTrackHeight), bg);

        // 空白部分ダブルクリックで新規キー追加
        ImGui::SetCursorScreenPos(rowOrigin);
        ImGui::InvisibleButton(("track_bg_" + std::to_string(trackIndex)).c_str(), ImVec2(width, kTrackHeight));
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            float t = XToTime(ImGui::GetMousePos().x, rowOrigin.x, width);
			size_t k = track.keys.size();
            track.keys.push_back({ t, "NewEvent", "" });
            m_selectedKey = KeySelection{ trackIndex, k };
        }

        // キー描画
        for (size_t k = 0; k < track.keys.size(); ++k)
        {
            auto& key = track.keys[k];
            float x = TimeToX(key.time, rowOrigin.x, width);
            float cy = rowOrigin.y + kTrackHeight * 0.5f;
            bool isSelected = m_selectedKey && m_selectedKey->trackIndex == trackIndex && m_selectedKey->keyIndex == k;

            ImU32 col = isSelected ? IM_COL32(255, 200, 60, 255) : IM_COL32(80, 180, 255, 255);
            ImVec2 diamond[4] = {
                ImVec2(x, cy - 7), ImVec2(x + 7, cy), ImVec2(x, cy + 7), ImVec2(x - 7, cy)
            };
            dl->AddConvexPolyFilled(diamond, 4, col);

            // ドラッグ/選択/削除用のヒットエリア
            ImGui::SetCursorScreenPos(ImVec2(x - 7, cy - 7));
            ImGui::PushID(static_cast<int>(trackIndex * 1000 + k));
            ImGui::InvisibleButton("key", ImVec2(14, 14));
			bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly | ImGuiHoveredFlags_DelayShort);
			ImVec2 rectMin = ImGui::GetItemRectMin(); // ヒットエリアの最小座標
			ImVec2 rectMax = ImGui::GetItemRectMax(); // ヒットエリアの最大座標
			bool isRectHovered = ImGui::IsMouseHoveringRect(rectMin, rectMax);
            if (isRectHovered && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
				m_selectedKey = KeySelection{ trackIndex, k, true };
            }
            if (!ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                if (isSelected)
                {
                    m_selectedKey->isDragging = false;
                }
			}
			if (isSelected && m_selectedKey->isDragging)
            {
                key.time = XToTime(ImGui::GetMousePos().x, rowOrigin.x, width);
                key.time = std::clamp(key.time, 0.0f, m_timeline->GetDuration());
            }
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                if (hovered)
                {
                    m_selectedKey = KeySelection{ trackIndex, k };
                }
                else
                {
                    //m_selectedKey = std::nullopt;
                }
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Right))
            {
                track.keys.erase(track.keys.begin() + k);
                ImGui::PopID();
                break; // イテレータ無効化のためこの行の描画を打ち切り
            }
            ImGui::PopID();
        }
    }

    void AnimationTimelineEditor::DrawTimelineArea()
    {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float width = (std::max)(avail.x - kLabelWidth, m_timeline->GetDuration() * m_pixelsPerSecond + 40.0f);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 origin = ImGui::GetCursorScreenPos();
        ImVec2 timelineOrigin = ImVec2(origin.x + kLabelWidth, origin.y);

        DrawRuler(dl, timelineOrigin, width);

        float y = origin.y + 20;
        auto& tracks = m_timeline->GetEventTracks();
        for (size_t i = 0; i < tracks.size(); ++i)
        {
            // トラック名ラベル
            ImGui::SetCursorScreenPos(ImVec2(origin.x, y));
            ImGui::Text("%s", tracks[i].name.c_str());

            DrawTrackRow(dl, ImVec2(timelineOrigin.x, y), width, i);
            y += kTrackHeight;
        }

        // 選択中キーのインスペクタ
        if (m_selectedKey)
        {
            auto& key = tracks[m_selectedKey->trackIndex].keys[m_selectedKey->keyIndex];
            ImGui::SetCursorScreenPos(ImVec2(origin.x, y + 10));
            ImGui::Separator();
            char nameBuf[128];
            strncpy_s(nameBuf, key.eventName.c_str(), sizeof(nameBuf));
            if (ImGui::InputText("Event Name", nameBuf, sizeof(nameBuf))) key.eventName = nameBuf;
            char paramBuf[128];
            strncpy_s(paramBuf, key.stringParam.c_str(), sizeof(paramBuf));
            if (ImGui::InputText("String Param", paramBuf, sizeof(paramBuf))) key.stringParam = paramBuf;
        }

        ImGui::Dummy(ImVec2(width + kLabelWidth, y - origin.y + 100));
    }
}
#endif // USE_IMGUI
