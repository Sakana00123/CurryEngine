#include "pch.h"
#include "AnimationTimelineEditor.h"
#ifdef USE_IMGUI
#include <imgui_internal.h>
#include "Engine/Core/Reflection/Meta.h"
#include <Engine\Resources\AssetDatabase.h>

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
				// 選択中キーが削除された場合、選択を解除する
                bool isSelectedKeyDeleted = m_selectedKey && m_selectedKey->trackIndex == trackIndex && (m_selectedKey->keyIndex == k || (m_selectedKey->keyIndex >= 0 && m_selectedKey->keyIndex < track.keys.size()));
                if (isSelectedKeyDeleted)
                {
                    m_selectedKey.reset();
				}
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

		static const char* typeLabels[] = { "Custom", "Sound", "Particle" };

        for (size_t i = 0; i < tracks.size(); ++i)
        {
            // トラック名ラベル
            ImGui::SetCursorScreenPos(ImVec2(origin.x, y));
            
			static constexpr float optionButtonWidth = 20.0f;
			static constexpr float spacing = 4.0f;
			ImGui::BeginChild(("TrackLabelChild" + std::to_string(i)).c_str(), ImVec2(kLabelWidth, kTrackHeight), false);
            ImGui::SetNextItemWidth(kLabelWidth - spacing);
			ImGui::PushID(static_cast<int>(i));
            std::string labelStr = tracks[i].name.empty() ? "Track " + std::to_string(i) : tracks[i].name;
            if (labelStr.length() > 10)
            {
                labelStr = labelStr.substr(0, 7) + "...";
			}
			labelStr += "(" + std::string(typeLabels[static_cast<int>(tracks[i].type)]) + ")";
            if (ImGui::Button(labelStr.c_str()))
            {
                ImGui::OpenPopup(("TrackOptionsPopup" + std::to_string(i)).c_str());
			}
            if (ImGui::BeginPopup(("TrackOptionsPopup" + std::to_string(i)).c_str()))
            {
				// トラック名の編集
                char label[128];
                strncpy_s(label, tracks[i].name.c_str(), sizeof(label));
                if (ImGui::InputText(("##TrackName" + std::to_string(i)).c_str(), label, sizeof(label)))
                {
                    tracks[i].name = label;
                }
				// イベントタイプの選択
                const char* eventTypes[] = { "Custom", "SoundEffect", "ParticleEffect" };
                int currentTypeIndex = static_cast<int>(tracks[i].type);
				ImGui::Text("Event Type");
				ImGui::SameLine();
                if (ImGui::Combo(("##Event Type" + std::to_string(i)).c_str(), &currentTypeIndex, eventTypes, IM_ARRAYSIZE(eventTypes)))
                {
                    tracks[i].type = static_cast<CurryEngine::Resources::AnimationEventType>(currentTypeIndex);
				}
				// トラック削除
                if (ImGui::Button("Delete Track"))
                {
                    tracks.erase(tracks.begin() + i);
                    ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
					ImGui::PopID();
					ImGui::EndChild();
                    break; // イテレータ無効化のためこの行の描画を打ち切り
				}
                ImGui::EndPopup();
			}
			ImGui::PopID();
			ImGui::EndChild();
            

            DrawTrackRow(dl, ImVec2(timelineOrigin.x, y), width, i);
            y += kTrackHeight;
        }

        // 選択中キーのインスペクタ
        if (m_selectedKey)
        {
            if (m_selectedKey->trackIndex >= tracks.size() || m_selectedKey->keyIndex >= tracks[m_selectedKey->trackIndex].keys.size())
            {
                m_selectedKey.reset();
            }
            else
            {
				auto& track = tracks[m_selectedKey->trackIndex];
                auto& key = track.keys[m_selectedKey->keyIndex];
                ImGui::SetCursorScreenPos(ImVec2(origin.x, y + 10));
                ImGui::Separator();

				static std::string searchFunctionClass = "";
                ImGui::Text("Search Function Class");
				ImGui::SameLine();
				ImGui::PushItemWidth(200);
				char searchBuf[128];
				strncpy_s(searchBuf, searchFunctionClass.c_str(), sizeof(searchBuf));
				if (ImGui::InputText("##SearchFunctionClass", searchBuf, sizeof(searchBuf))) searchFunctionClass = searchBuf;
				ImGui::PopItemWidth();
				ImGui::SameLine();
                if (ImGui::Button("Search"))
                {
                    // 検索処理を実装する
                    if (auto* classMeta = ReflectionRegistry::FindClass(searchFunctionClass))
                    {
                        // クラスが見つかった場合の処理
                        ImGui::OpenPopup("FunctionListPopup");
                    }
                    else
                    {
                        // クラスが見つからなかった場合の処理
						ImGui::OpenPopup("FunctionNotFoundPopup");
                    }
                }
                if (ImGui::BeginPopup("FunctionListPopup"))
                {
                    if (auto* classMeta = ReflectionRegistry::FindClass(searchFunctionClass))
                    {
                        ImGui::Text("%d Functions in class: %s", classMeta->methods.size(), searchFunctionClass.c_str());
						int methodIndex = 0;
                        for (const auto& method : classMeta->methods)
                        {
							ImGui::PushID(methodIndex++);
                            if (ImGui::Selectable(method.name.c_str()))
                            {
                                key.eventName = method.name;
                                ImGui::CloseCurrentPopup();
                            }
							ImGui::PopID();
                        }
                    }
                    if (ImGui::Button("Close"))
                    {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
				}
                if (ImGui::BeginPopup("FunctionNotFoundPopup"))
                {
                    ImGui::Text("Class not found: %s", searchFunctionClass.c_str());

					// 似ているクラス名の候補を表示する
                    std::vector<std::string> similarClasses;
                    for (const auto& [className, classMeta] : ReflectionRegistry::GetClassRegistry())
                    {
                        if (className.find(searchFunctionClass) != std::string::npos)
                        {
                            similarClasses.push_back(className);
                        }
                    }
                    if (!similarClasses.empty())
                    {
						// 似ているクラス名の候補を表示
						ImGui::Text("%d similar classes found:", similarClasses.size());
                        for (const auto& similarClass : similarClasses)
                        {
							ImGui::PushID(similarClass.c_str());
                            if (ImGui::Selectable(similarClass.c_str()))
                            {
                                searchFunctionClass = similarClass;
                                ImGui::CloseCurrentPopup();
							}
							ImGui::PopID();
                        }
                    }
                    else
                    {
                        ImGui::Text("No similar classes found.");
					}


					// 閉じるボタン
                    if (ImGui::Button("Close"))
                    {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
				}

				static std::unordered_map<std::string, std::string> assetIdToNameMap; // アセットIDからアセット名へのマッピング
                if (ImGui::Button("Update Asset Map"))
                {
                    assetIdToNameMap.clear();
				}

				// イベントタイプに応じたパラメータの編集
                switch (track.type)
                {
                case CurryEngine::Resources::AnimationEventType::Custom:
                {
                    // イベント名とパラメータの編集
                    char nameBuf[128];
                    strncpy_s(nameBuf, key.eventName.c_str(), sizeof(nameBuf));
                    if (ImGui::InputText("Event Name", nameBuf, sizeof(nameBuf))) key.eventName = nameBuf;
                    char paramBuf[128];
                    strncpy_s(paramBuf, key.stringParam.c_str(), sizeof(paramBuf));
                    if (ImGui::InputText("String Param", paramBuf, sizeof(paramBuf))) key.stringParam = paramBuf;
                    break;
                }
                case CurryEngine::Resources::AnimationEventType::SoundEffect:
                {
                    key.eventName = "PlaySound";
                    // サウンドの選択
                    if (assetIdToNameMap.empty())
                    {
                        // アセット名からIDへのマッピングを初期化
                        std::vector<CurryEngine::Resources::AssetMeta> soundMetas = CurryEngine::Resources::AssetDatabase::FindAllByType(AssetType::Sound);
                        for (const auto& meta : soundMetas)
                        {
                            assetIdToNameMap[meta.id.ToString()] = meta.path.filename().string();
                        }
                    }
                    // サウンドの選択コンボボックス
                    std::vector<std::string> soundNames;
                    for (const auto& [id, name] : assetIdToNameMap)
                    {
                        soundNames.push_back(name);
                    }
                    static int selectedSoundIndex = -1;
                    if (selectedSoundIndex < 0 && !key.stringParam.empty())
                    {
                        // 既存のstringParamからインデックスを設定
                        auto it = std::find_if(assetIdToNameMap.begin(), assetIdToNameMap.end(),
                            [&key](const auto& pair) { return pair.second == key.stringParam; });
                        if (it != assetIdToNameMap.end())
                        {
                            selectedSoundIndex = std::distance(assetIdToNameMap.begin(), it);
                        }
                    }
                    // コンボボックスの表示
					ImGui::Text("Sound: %s", assetIdToNameMap[key.stringParam].c_str());
					ImGui::SameLine();
                    if (ImGui::Button("...##Select Sound"))
                    {
                        ImGui::OpenPopup("SoundEffectPopup");
					}
                    if (ImGui::BeginPopup("SoundEffectPopup"))
                    {
                        for (int i = 0; i < soundNames.size(); ++i)
                        {
                            bool isSelected = (selectedSoundIndex == i);
                            ImGui::PushID(i);
                            if (ImGui::Selectable(soundNames[i].c_str(), isSelected))
                            {
                                selectedSoundIndex = i;
                                // 選択されたサウンドのIDをstringParamに設定
                                auto it = std::next(assetIdToNameMap.begin(), i);
                                key.stringParam = it->first; // アセットIDを格納
                            }
                            ImGui::PopID();
                            if (isSelected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndPopup();
					}
					break;
                }
				case CurryEngine::Resources::AnimationEventType::ParticleEffect:
                {
					key.eventName = "PlayParticle";
                    
					ImGui::Text("Particle Effect Selection is not implemented yet.");
					//// パーティクルの選択
     //               if (assetIdToNameMap.empty())
     //               {
     //                   // アセット名からIDへのマッピングを初期化
     //                   std::vector<CurryEngine::Resources::AssetMeta> particleMetas = CurryEngine::Resources::AssetDatabase::FindAllByType(AssetType::);
     //                   for (const auto& meta : particleMetas)
     //                   {
     //                       assetIdToNameMap[meta.id.ToString()] = meta.path.filename().string();
     //                   }
					//}
					//// パーティクルの選択コンボボックス
					//std::vector<std::string> particleNames;
     //               for (const auto& [id, name] : assetIdToNameMap)
     //               {
     //                   particleNames.push_back(name);
					//}
					//static int selectedParticleIndex = -1;
     //               if (selectedParticleIndex < 0 && !key.stringParam.empty())
     //               {
     //                   // 既存のstringParamからインデックスを設定
     //                   auto it = std::find_if(assetIdToNameMap.begin(), assetIdToNameMap.end(),
     //                       [&key](const auto& pair) { return pair.second == key.stringParam; });
     //                   if (it != assetIdToNameMap.end())
     //                   {
     //                       selectedParticleIndex = std::distance(assetIdToNameMap.begin(), it);
     //                   }
					//}
					//// コンボボックスの表示
					//if (ImGui::BeginCombo("Particle Effect", selectedParticleIndex >= 0 ? particleNames[selectedParticleIndex].c_str() : "Select Particle"))
					//{
     //                   for (int i = 0; i < particleNames.size(); ++i)
     //                   {
     //                       bool isSelected = (selectedParticleIndex == i);
     //                       ImGui::PushID(i);
     //                       if (ImGui::Selectable(particleNames[i].c_str(), isSelected))
     //                       {
     //                           selectedParticleIndex = i;
     //                           // 選択されたパーティクルのIDをstringParamに設定
     //                           auto it = std::next(assetIdToNameMap.begin(), i);
     //                           key.stringParam = it->first; // アセットIDを格納
     //                       }
     //                       ImGui::PopID();
     //                       if (isSelected)
     //                           ImGui::SetItemDefaultFocus();
     //                   }
     //                   ImGui::EndCombo();
					//}
                    break;
                }
                default:
                    break;
                }
            }
        }

        ImGui::Dummy(ImVec2(width + kLabelWidth, y - origin.y + 100));
    }
}
#endif // USE_IMGUI
