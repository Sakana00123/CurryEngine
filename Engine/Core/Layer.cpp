#include "pch.h"
#include "Layer.h"
#include "Engine/Utils/JsonFileHandler.h"

void LayerManager::SetLayerName(Layer layer, const std::string& name)
{
    m_layerNames[layer] = name;
}

Layer LayerManager::GetLayerByName(const std::string& name) const
{
    for (int i = 0; i < MAX_LAYERS; ++i)
        if (m_layerNames[i] == name) return static_cast<Layer>(i);
    return 0; // Default
}

void LayerManager::SetLayerCollision(Layer a, Layer b, bool enabled)
{
    if (enabled)
    {
		m_collisionMatrix[a] |= ToMask(b); // aがbと衝突するように設定
		m_collisionMatrix[b] |= ToMask(a); // bがaと衝突するように設定
    }
    else
    {
		m_collisionMatrix[a] &= ~ToMask(b); // aがbと衝突しないように設定
		m_collisionMatrix[b] &= ~ToMask(a); // bがaと衝突しないように設定
	}
}

bool LayerManager::GetLayerCollision(Layer a, Layer b) const
{
	// 衝突マトリクスは対称なので、aがbと衝突するかをチェックすれば十分
	return (m_collisionMatrix[a] & ToMask(b)) != 0; // aがbと衝突するかどうかをチェック
}

LayerMask LayerManager::GetCollisionMask(Layer layer) const
{
	return m_collisionMatrix[layer]; // 指定されたレイヤーの衝突マスクを返す
}

json LayerManager::Serialize() const
{
    json j;
	// レイヤー名のシリアライズ
    for (int i = 0; i < MAX_LAYERS; ++i)
    {
		json layerJson;
		layerJson["id"] = i;
		layerJson["name"] = m_layerNames[i];
		j["layers"].push_back(layerJson);
    }
	// 衝突マトリクスのシリアライズ
    for (int i = 0; i < MAX_LAYERS; ++i)
    {
        json collisionJson;
        collisionJson["layer"] = i;
        collisionJson["mask"] = GetCollisionMask(i);
        j["collisionMatrix"].push_back(collisionJson);
    }

    return j;
}

void LayerManager::Deserialize(const json& j)
{
	// レイヤー名のデシリアライズ
    if (j.contains("layers"))
    {
        for (const auto& layerJson : j["layers"])
        {
            int id = layerJson["id"].get<int>();
            std::string name = layerJson["name"].get<std::string>();
            if (id >= 0 && id < MAX_LAYERS)
                m_layerNames[id] = name;
        }
	}
    // 衝突マトリクスのデシリアライズ
    if (j.contains("collisionMatrix"))
    {
		// 衝突マトリクスは対称なので、片方だけ読み込めば十分
        for (const auto& collisionJson : j["collisionMatrix"])
        {
            int layer = collisionJson["layer"].get<int>();
            LayerMask mask = collisionJson["mask"].get<LayerMask>();
            if (layer >= 0 && layer < MAX_LAYERS)
                m_collisionMatrix[layer] = mask;
		}
	}

}

void LayerManager::DrawLayerSettingsGUI()
{
    // ImGuiを使用してレイヤー設定GUIを描画するコードをここに実装
    // レイヤー名の編集、衝突マトリクスの編集など
#ifdef USE_IMGUI
	if (m_isOpen)
    {
        ImGui::Begin("Layer Settings", &m_isOpen);



#if 1
		if (ImGui::CollapsingHeader("Edit Layer Names", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // レイヤー名の編集
            for (int i = 0; i < MAX_LAYERS; ++i)
            {
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "Layer %d", i);

                ImGui::Text(buffer);
                ImGui::SameLine();
                std::string label = "##LayerName" + std::to_string(i); // ラベルはユニークにする必要があるため、IDを付加
                ImGui::InputText(label.c_str(), &m_layerNames[i][0], m_layerNames[i].capacity());
            }
        }
        ImGui::Separator();
#endif // 0

        // -------------------------------------------------------------------
        // 使用中のレイヤー（名前が空でないもの）のインデックスを収集
        // -------------------------------------------------------------------
        std::vector<int> usedLayers;
        for (int i = 0; i < MAX_LAYERS; ++i)
            if (!m_layerNames[i].empty())
                usedLayers.push_back(i);

        const int numUsed = static_cast<int>(usedLayers.size());

		if (ImGui::CollapsingHeader("Collision Matrix", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // -------------------------------------------------------------------
            // ツールバー: Rename Layer
            // -------------------------------------------------------------------
            if (ImGui::Button("Rename Layer"))
            {
                if (m_selectedLayer >= 0)
                {
                    strncpy_s(m_renameBuffer, m_layerNames[m_selectedLayer].c_str(), sizeof(m_renameBuffer) - 1);
                    m_renameBuffer[sizeof(m_renameBuffer) - 1] = '\0';
                    ImGui::OpenPopup("##RenamePopup");
                }
            }

            // Rename ポップアップ
            if (ImGui::BeginPopupModal("##RenamePopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("New name:");
                ImGui::SetNextItemWidth(200.0f);
                ImGui::InputText("##RenameInput", m_renameBuffer, sizeof(m_renameBuffer));
                if (ImGui::Button("OK", ImVec2(80, 0)))
                {
                    if (m_selectedLayer >= 0 && m_selectedLayer < MAX_LAYERS)
                        m_layerNames[m_selectedLayer] = m_renameBuffer;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(80, 0)))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }

            ImGui::SameLine();

            // Add Layer ボタン
            ImVec2 buttonSize(30, 30);
            if (ImGui::Button("+", buttonSize))
            {
                for (int i = 0; i < MAX_LAYERS; ++i)
                {
                    if (m_layerNames[i].empty())
                    {
                        m_layerNames[i] = "Layer" + std::to_string(i);
                        // 新しいレイヤーはデフォルトで全てのレイヤーと衝突するように設定
                        m_collisionMatrix[i] = LayerMasks::Everything; // 全てのビットを立てる（全レイヤーと衝突）
                        break;
                    }
                }
            }

            ImGui::SameLine();

            // Remove Layer ボタン
            if (ImGui::Button("-", buttonSize))
            {
                if (usedLayers.size() <= 4) // 最低4レイヤーは残す(デフォルトの4レイヤーを削除させない)
                    return;

                // 最後の使用中レイヤーを削除
                for (int i = MAX_LAYERS - 1; i >= 0; --i)
                {
                    if (!m_layerNames[i].empty())
                    {
                        m_layerNames[i].clear();
                        m_collisionMatrix[i] = 0; // 衝突マトリクスもクリア
                        if (m_selectedLayer == i) m_selectedLayer = -1; // 選択中のレイヤーが削除された場合は選択解除
                        break;
                    }
                }
            }

            ImGui::Separator();


            // 衝突マトリクスの編集
            // -------------------------------------------------------------------
            // コリジョンマトリックス
            // -------------------------------------------------------------------
            const float cellSize = 26.0f;
            const float labelWidth = 160.0f;
            const float headerH = 76.0f; // 列ヘッダーの高さ（縦書きテキスト分）

            // スクロール領域
            ImGui::BeginChild("##MatrixScroll", ImVec2(0, 0), false,
                ImGuiWindowFlags_HorizontalScrollbar);

            ImDrawList* draw = ImGui::GetWindowDrawList();
            ImVec2      origin = ImGui::GetCursorScreenPos();

            // ---- 列ヘッダー描画 -----------------------------------------------
            // 列ヘッダーはラベル列の右から始まる
            for (int ci = 0; ci < numUsed; ++ci)
            {
                int col = usedLayers[ci];
                float x = origin.x + labelWidth + ci * cellSize;
                float y = origin.y;

                // セル背景
                draw->AddRectFilled(
                    ImVec2(x, y), ImVec2(x + cellSize, y + headerH),
                    IM_COL32(40, 40, 40, 255));
                draw->AddRect(
                    ImVec2(x, y), ImVec2(x + cellSize, y + headerH),
                    IM_COL32(70, 70, 70, 255));

                // テキストを90度回転して描画
                // ImGuiは標準回転サポートなし → AddTextでY方向に1文字ずつ描く代わりに
                // PushClipRect + ImFont描画で擬似縦書きする
                const std::string& name = m_layerNames[col];
                ImVec2 textSize = ImGui::CalcTextSize(name.c_str());

                // 縦書き: フォントを回転させることはできないため、
                // 横書きのまま clipRect で切り取り、斜め配置のみ行う
                // → 実用的な方法として、テキストを一文字ずつ縦に並べる
                float charY = y + 4.0f;
                float charX = x + (cellSize - ImGui::GetFontSize()) * 0.5f;
                for (char c : name)
                {
                    char buf[2] = { c, '\0' };
                    draw->AddText(ImVec2(charX, charY), IM_COL32(220, 220, 220, 255), buf);
                    charY += ImGui::GetFontSize() - 1.0f;
                    if (charY > y + headerH - ImGui::GetFontSize()) break; // はみ出し防止
                }

                //// ImPlotを使って縦書きテキストを描画
                //{
                //    if (ImPlot::BeginPlot("##MarkerStyles", ImVec2(-1, 0)))
                //    {

                //        ImPlot::PushStyleColor(ImPlotCol_InlayText, ImVec4(1, 1, 1, 1));
                //        ImPlot::PlotText(name.c_str(), /*textPos.x*/5, /*textPos.y*/6, ImVec2(0, 0), { ImPlotProp_Flags, ImPlotTextFlags_Vertical });
                //        ImPlot::PopStyleColor();

                //        ImPlot::EndPlot();
                //    }
                //}


                // ホバーで全名をツールチップ表示
                ImGui::SetCursorScreenPos(ImVec2(x, y));
                ImGui::InvisibleButton(("##hdr" + std::to_string(ci)).c_str(),
                    ImVec2(cellSize, headerH));
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", name.c_str());
            }

            // ---- 行ループ -------------------------------------------------------
            for (int ri = 0; ri < numUsed; ++ri)
            {
                int row = usedLayers[ri];

                float rowY = origin.y + headerH + ri * cellSize;

                // 行背景
                ImU32 rowBgCol = (ri % 2 == 0)
                    ? IM_COL32(35, 35, 35, 255)
                    : IM_COL32(42, 42, 42, 255);
                draw->AddRectFilled(
                    ImVec2(origin.x, rowY),
                    ImVec2(origin.x + labelWidth + numUsed * cellSize, rowY + cellSize),
                    rowBgCol);

                // ---- 行ラベルボタン --------------------------------------------
                bool isSelected = (m_selectedLayer == row);
                ImGui::PushID(row * 1000);

                ImGui::SetCursorScreenPos(ImVec2(origin.x, rowY));
                if (isSelected)
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
                else
                    ImGui::PushStyleColor(ImGuiCol_Button,
                        ri % 2 == 0
                        ? ImVec4(0.14f, 0.14f, 0.14f, 1.0f)
                        : ImVec4(0.16f, 0.16f, 0.16f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.40f, 0.40f, 1.0f));

                ImVec2 buttonSize(labelWidth - 4.0f, cellSize);

                // 行ラベルはクリックで選択、再度クリックで編集モードに入る
                if (m_selectedLayer == row && m_layerNameEditMode)
                {
                    // 編集モード: InputTextを表示して名前を編集
                    ImGui::SetCursorScreenPos(ImVec2(origin.x + 2.0f, rowY + 2.0f));
                    ImGui::SetNextItemWidth(buttonSize.x - 4.0f);

                    if (m_layerNameEditModeJustStarted)
                    {
                        // 編集モードに入った直後は、InputTextにフォーカスを当てる
                        ImGui::SetKeyboardFocusHere();
                        m_layerNameEditModeJustStarted = false; // フラグをリセット
                    }

                    // InputTextでレイヤー名を編集
                    if (ImGui::InputText("##EditLayerName", m_renameBuffer, IM_ARRAYSIZE(m_renameBuffer), ImGuiInputTextFlags_AutoSelectAll))
                    {
                        // 入力中はリアルタイムでレイヤー名を更新
                        m_layerNames[row] = m_renameBuffer;
                    }
                    if (ImGui::IsItemDeactivated()) // InputTextからフォーカスが外れたとき
                    {
                        m_layerNameEditMode = false; // 編集モードを終了
                    }
                }
                else
                {
                    bool canSelect = (row != m_defaultLayer && row != m_transparentFXLayer && row != m_ignoreRaycastLayer && row != m_uiLayer);
                    //bool canSelect = true;

                    if (ImGui::Button(m_layerNames[row].c_str(), buttonSize))
                    {
                        // 行ラベルがクリックされたときの処理

                        // デフォルトレイヤーは選択不可にする
                        if (canSelect)
                        {
                            m_selectedLayer = row;
                            m_layerNameEditMode = false; // 編集モードを終了
                        }
                    }
                    if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered() && canSelect)
                    {
                        // 行ラベルがダブルクリックされたときの処理
                        // ここでは、レイヤー名の編集モードに入るためのフラグを切り替えるだけにする
                        m_selectedLayer = row;
                        m_layerNameEditMode = true;
                        m_layerNameEditModeJustStarted = true; // 編集モード開始フラグをセット
                        strncpy_s(m_renameBuffer, m_layerNames[row].c_str(), sizeof(m_renameBuffer) - 1);
                        m_renameBuffer[sizeof(m_renameBuffer) - 1] = '\0';
                    }
                }

                ImGui::PopStyleColor(3);

                // ---- セルループ ------------------------------------------------
                for (int ci = 0; ci < numUsed; ++ci)
                {
                    int col = usedLayers[ci];

                    float cx = origin.x + labelWidth + ci * cellSize;
                    ImVec2 cellMin(cx, rowY);
                    ImVec2 cellMax(cx + cellSize, rowY + cellSize);
                    ImVec2 center((cellMin.x + cellMax.x) * 0.5f,
                        (cellMin.y + cellMax.y) * 0.5f);

                    bool checked = GetLayerCollision(static_cast<Layer>(row),
                        static_cast<Layer>(col));

                    // セル背景
                    ImU32 cellBg = checked
                        ? IM_COL32(100, 72, 16, 255)
                        : rowBgCol;
                    draw->AddRectFilled(cellMin, cellMax, cellBg);
                    draw->AddRect(cellMin, cellMax, IM_COL32(60, 60, 60, 255));

                    // チェックボックス枠
                    const float half = 6.5f;
                    ImVec2 boxMin(center.x - half, center.y - half);
                    ImVec2 boxMax(center.x + half, center.y + half);

                    ImU32 borderCol = checked
                        ? IM_COL32(220, 180, 80, 255)
                        : IM_COL32(110, 110, 110, 255);
                    draw->AddRect(boxMin, boxMax, borderCol, 1.0f);

                    // チェックマーク
                    if (checked)
                    {
                        ImVec2 p0(boxMin.x + 2.0f, center.y);
                        ImVec2 p1(center.x - 1.0f, boxMax.y - 2.0f);
                        ImVec2 p2(boxMax.x - 2.0f, boxMin.y + 2.0f);
                        draw->AddLine(p0, p1, IM_COL32(255, 255, 255, 255), 1.5f);
                        draw->AddLine(p1, p2, IM_COL32(255, 255, 255, 255), 1.5f);
                    }

                    // クリック判定
                    ImGui::SetCursorScreenPos(cellMin);
                    std::string cellId = "##c" + std::to_string(ri) + "_" + std::to_string(ci);
                    ImGui::InvisibleButton(cellId.c_str(), ImVec2(cellSize, cellSize));

                    if (ImGui::IsItemClicked())
                        SetLayerCollision(static_cast<Layer>(row),
                            static_cast<Layer>(col), !checked);

                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("%s  x  %s",
                            m_layerNames[row].c_str(), m_layerNames[col].c_str());
                        draw->AddRect(cellMin, cellMax,
                            IM_COL32(220, 180, 80, 200), 0.0f, 0, 2.0f);
                    }
                }

                ImGui::PopID();
            }

            // ImGuiにスクロール領域の実サイズを伝える
            ImGui::SetCursorScreenPos(ImVec2(
                origin.x,
                origin.y + headerH + numUsed * cellSize + 4.0f));
            ImGui::Dummy(ImVec2(labelWidth + numUsed * cellSize, 1.0f));

            ImGui::EndChild();

            // -------------------------------------------------------------------
            // フッター: Reset
            // -------------------------------------------------------------------
            ImGui::Separator();

            if (ImGui::Button("Reset"))
            {
                // 全レイヤーのコリジョンをデフォルト（Everything）に戻す
                for (int i = 0; i < MAX_LAYERS; ++i)
                    m_collisionMatrix[i] = LayerMasks::Everything;
            }
        }


	    ImGui::End();
    }
#endif // USE_IMGUI
}

LayerManager::LayerManager()
{
    // デフォルトのレイヤー名を設定
    m_layerNames[0] = "Default";
    m_layerNames[1] = "TransparentFX";
	m_layerNames[2] = "Ignore Raycast";
	m_layerNames[3] = "UI";
	m_layerNames[4] = "Player"; // 以降は必要に応じてユーザーが追加していく想定（レイヤー5以降は初期状態では空のまま）

	// 衝突マトリクスの初期化（全てのレイヤーが衝突するように設定）
    for (int i = 0; i < MAX_LAYERS; ++i)
		m_collisionMatrix[i] = LayerMasks::Everything;
}