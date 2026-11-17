#include "pch.h"
#include "AIChatWindow.h"
#include "Engine/EditorAssistant/AIClient.h"
#include "imgui.h"

#include <algorithm>
#include <cfloat>
#include <string_view>
#include <utility>
#include <Engine\Scenes\SceneManager.h>
#include "Engine/Factory/GameObjectFactory.h"
#include "Engine/Utils/JsonUtils.h"

using json = nlohmann::json;

namespace CurryEngine::EditorAssistant
{
    AIChatWindow::AIChatWindow(Mode mode)
        : m_mode(mode)
    {
    }

    void AIChatWindow::Draw(bool& open)
    {
		// 返信待ちのメッセージがある場合は、ポーリングして返信を取得する
        PollReply();
        if (!open)
            return;

        ImGui::SetNextWindowSize(ImVec2(680.0f, 540.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(280.0f, 220.0f),
            ImVec2(FLT_MAX, FLT_MAX));

        if (ImGui::Begin("AI Chat", &open,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            const ImGuiStyle& style = ImGui::GetStyle();
            const float available = ImGui::GetContentRegionAvail().x;
            const float gutter = (std::min)(style.ScrollbarSize + style.ItemSpacing.x,
                available * 0.15f);
            const float maxWidth = ImGui::GetFontSize() * 34.0f;
            const float width = (std::max)(1.0f, (std::min)(maxWidth,
                available - 2.0f * gutter));
            const float inset = (available - width) * 0.5f;

			// 上部のツールバーを描画
			DrawToolbar(inset, width);

            const float line = ImGui::GetTextLineHeight();
            const float room = ImGui::GetContentRegionAvail().y;
            const float inputLines = (std::clamp)(room / (line * 3.5f), 1.0f, 3.0f);
            const float composerHeight = inputLines * line
                + 2.0f * style.FramePadding.y + 2.0f * style.WindowPadding.y;
            const float footerHeight =
                composerHeight + line + 2.0f * style.ItemSpacing.y;

            DrawHistory(inset, width, footerHeight);
            DrawComposer(inset, width, composerHeight);
            DrawFooter(inset, width);
        }
        ImGui::End();
    }

    void AIChatWindow::NewChat()
    {
        m_messages.clear();
        m_pending.reset();
        m_input.fill('\0');
        m_notice = nullptr;
        m_followLatest = true;
        m_scrollToBottom = true;
        m_focusInput = true;
    }

    void AIChatWindow::DrawToolbar(float inset, float width)
    {
        const ImGuiStyle& style = ImGui::GetStyle();
        const float left = ImGui::GetCursorPosX() + inset;

        ImGui::SetCursorPosX(left);
        if (ImGui::Button(reinterpret_cast<const char*>(u8"新規チャット")))
            NewChat();

        const float newChatWidth = ImGui::GetItemRectSize().x;
        const float labelWidth = (std::max)(
            ImGui::CalcTextSize("Chat").x,
            ImGui::CalcTextSize("Agent").x);

        // 矢印の幅も含め、どちらを選択しても同じ幅にする。
        const float comboWidth = labelWidth + ImGui::GetFrameHeight()
            + 2.0f * style.FramePadding.x + style.ItemInnerSpacing.x;
        const float comboX = left + width - comboWidth;

        // 同じ行に収まるときは「新規チャット」の右に並べる。
        if (comboX >= left + newChatWidth + style.ItemSpacing.x)
            ImGui::SameLine();

        ImGui::SetCursorPosX((std::max)(left, comboX));

        const char* const modeLabels[] = { "Chat", "Agent" };
        int selectedMode = m_mode == Mode::Agent ? 1 : 0;

        ImGui::BeginDisabled(m_gatewayBusy);
        ImGui::SetNextItemWidth(comboWidth);
        if (ImGui::Combo("##chat_mode", &selectedMode,
            modeLabels, IM_ARRAYSIZE(modeLabels)))
        {
            m_mode = selectedMode == 0 ? Mode::Chat : Mode::Agent;
        }
        ImGui::EndDisabled();
    }

    void AIChatWindow::DrawHistory(float inset, float width, float footerHeight)
    {
        if (ImGui::BeginChild("##history", ImVec2(0.0f, -footerHeight),
            ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground))
        {
            const ImVec2 start = ImGui::GetCursorPos();
            const float left = start.x + inset;
            const float gap = ImGui::GetTextLineHeight() * 0.55f;

            /*if (m_messages.empty())
            {
                const auto* greeting = reinterpret_cast<const char*>(
                    u8"今日は何を煮込みますか？");
                const ImVec2 text = ImGui::CalcTextSize(greeting, nullptr, false, width);
                const float height = ImGui::GetContentRegionAvail().y;
                ImGui::SetCursorPos(ImVec2(
                    left + (std::max)(0.0f, (width - text.x) * 0.5f),
                    start.y + (std::max)(0.0f, (height - text.y) * 0.5f)));
                ImGui::PushTextWrapPos(left + width);
                ImGui::TextUnformatted(greeting);
                ImGui::PopTextWrapPos();
            }*/

            for (const auto& message : m_messages)
            {
                ImGui::SetCursorPosX(left);
                if (message.role == Role::User)
                {
                    DrawBubble(message.text, width);
                }
                else
                {
                    ImGui::PushTextWrapPos(left + width);
                    ImGui::TextUnformatted(message.text.data(),
                        message.text.data() + message.text.size());
                    ImGui::PopTextWrapPos();
                }

                ImGui::Dummy(ImVec2(0.0f, gap));
            }

            if (m_scrollToBottom)
            {
                ImGui::SetScrollHereY(1.0f);
                m_scrollToBottom = false;
                m_followLatest = true;
            }
            else
            {
                m_followLatest = ImGui::GetScrollMaxY() - ImGui::GetScrollY()
                    <= ImGui::GetTextLineHeight();
            }
        }
        ImGui::EndChild();
    }

    void AIChatWindow::DrawBubble(const std::string& text, float width)
    {
        const ImGuiStyle& style = ImGui::GetStyle();
        const ImVec2 padding(style.FramePadding.x * 1.6f, style.FramePadding.y * 1.5f);
        const char* begin = text.data();
        const char* end = begin + text.size();
        const float maxTextWidth = (std::max)(1.0f, width * 0.82f - 2.0f * padding.x);
        const float textWidth = (std::min)(maxTextWidth,
            ImGui::CalcTextSize(begin, end).x + 1.0f);
        const ImVec2 measured = ImGui::CalcTextSize(begin, end, false, textWidth);
        const ImVec2 size(textWidth + 2.0f * padding.x, measured.y + 2.0f * padding.y);

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (std::max)(0.0f, width - size.x));
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        if (ImGui::IsRectVisible(size))
        {
            ImDrawList* draw = ImGui::GetWindowDrawList();
            draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y),
                ImGui::GetColorU32(ImGuiCol_FrameBgActive), ImGui::GetFontSize() * 0.8f);
            draw->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
                ImVec2(pos.x + padding.x, pos.y + padding.y),
                ImGui::GetColorU32(ImGuiCol_Text), begin, end, textWidth);
        }
        ImGui::Dummy(size);
    }

    void AIChatWindow::DrawComposer(float inset, float width, float height)
    {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + inset);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ImGui::GetFontSize() * 0.95f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);

        const bool visible = ImGui::BeginChild("##composer", ImVec2(width, height),
            ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();

        bool submitted = false;
        bool clicked = false;
        bool refocus = false;
        const bool waiting = m_pending != nullptr;
        if (visible)
        {
            const ImGuiStyle& style = ImGui::GetStyle();
            const ImVec2 available = ImGui::GetContentRegionAvail();
            const float buttonSize = ImGui::GetFrameHeight();
            const ImVec2 inputSize(
                (std::max)(1.0f, available.x - buttonSize - style.ItemSpacing.x),
                (std::max)(buttonSize, available.y));

            if (std::exchange(m_focusInput, false))
                ImGui::SetKeyboardFocusHere();

            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            const ImGuiIO& io = ImGui::GetIO();
            ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;

#if IMGUI_VERSION_NUM >= 19270
            inputFlags |= ImGuiInputTextFlags_WordWrap;

            // Shift + Enter の改行は Dear ImGui が処理する。
            // Ctrl + Enter は確定側に回し、下の送信判定から外す。
            const bool ctrlEnterForNewLine = !io.KeyCtrl;
#else
            // 旧版では Ctrl とフラグの組み合わせで改行が決まる。
            // フラグを切り替え、Shift が押されているときに改行させる。
            const bool ctrlEnterForNewLine = io.KeyShift == io.KeyCtrl;
#endif

            if (ctrlEnterForNewLine)
                inputFlags |= ImGuiInputTextFlags_CtrlEnterForNewLine;

            const bool confirmed = ImGui::InputTextMultiline(
                "##prompt", m_input.data(), m_input.size(), inputSize, inputFlags);

            submitted = confirmed && !io.KeyShift && !io.KeyCtrl;

            // Ctrl + Enter で確定された場合も入力欄にフォーカスを戻す。
            if (confirmed && !submitted)
                m_focusInput = true;
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();

            if (m_input[0] == '\0' && !ImGui::IsItemActive())
            {
                const ImVec2 min = ImGui::GetItemRectMin();
                const ImVec2 max = ImGui::GetItemRectMax();
                const ImVec4 clip(min.x, min.y, max.x, max.y);
                const auto* hint = reinterpret_cast<const char*>(u8"メッセージを入力…");
                ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
                    ImVec2(min.x + style.FramePadding.x, min.y + style.FramePadding.y),
                    ImGui::GetColorU32(ImGuiCol_TextDisabled), hint, nullptr,
                    (std::max)(1.0f, inputSize.x - 2.0f * style.FramePadding.x), &clip);
            }

            const float buttonY = ImGui::GetItemRectMax().y - buttonSize;
            ImGui::SameLine();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            pos.y = buttonY;
            ImGui::SetCursorScreenPos(pos);

            const bool hasText =
                std::string_view(m_input.data()).find_first_not_of(" \t\r\n") !=
                std::string_view::npos;
            const bool canSend = !waiting && !m_gatewayBusy && hasText;

            ImGui::BeginDisabled(!waiting && !canSend);
            clicked = DrawActionButton(waiting, buttonSize);
            ImGui::EndDisabled();

            refocus = submitted && !canSend;
            submitted = canSend && (submitted || clicked);
        }
        ImGui::EndChild();

        if (waiting && clicked)
        {
            m_pending.reset();
            m_notice = reinterpret_cast<const char*>(u8"応答の待機を停止しました");
            m_focusInput = true;
        }
        else if (submitted)
        {
            Send();
        }
        else if (refocus)
        {
            m_focusInput = true;
        }
    }

    bool AIChatWindow::DrawActionButton(bool stop, float size)
    {
        const ImVec4 foreground = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        const ImVec4 background = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        ImGui::PushStyleColor(ImGuiCol_Button, foreground);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
            ImGui::GetStyleColorVec4(ImGuiCol_CheckMark));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
            ImGui::GetStyleColorVec4(ImGuiCol_CheckMark));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, size * 0.5f);
        const bool pressed = ImGui::Button("##send_or_stop", ImVec2(size, size));
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        const ImVec2 p = ImGui::GetItemRectMin();
        const ImVec2 center(p.x + size * 0.5f, p.y + size * 0.5f);
        const float radius = size * 0.19f;
        const ImU32 color = ImGui::GetColorU32(background);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        if (stop)
        {
            draw->AddRectFilled(ImVec2(center.x - radius, center.y - radius),
                ImVec2(center.x + radius, center.y + radius), color, 1.0f);
        }
        else
        {
            const ImVec2 tip(center.x, center.y - radius);
            draw->AddLine(ImVec2(center.x, center.y + radius), tip, color, 2.0f);
            draw->AddLine(tip, ImVec2(center.x - radius, center.y), color, 2.0f);
            draw->AddLine(tip, ImVec2(center.x + radius, center.y), color, 2.0f);
        }

        ImGui::SetItemTooltip("%s", stop
            ? reinterpret_cast<const char*>(u8"応答の待機を停止")
            : reinterpret_cast<const char*>(u8"送信 (Enter)"));
        return pressed;
    }

    void AIChatWindow::DrawFooter(float inset, float width) const
    {
        const char* status = m_notice;
        if (m_pending)
            status = reinterpret_cast<const char*>(u8"応答を待っています…");
        else if (m_gatewayBusy)
            status = reinterpret_cast<const char*>(u8"前の通信の完了を待っています…");

        const auto* shortcut = reinterpret_cast<const char*>(
            u8"Enter で送信 · Shift+Enter で改行");
        const float left = ImGui::GetCursorPosX() + inset;
        const float shortcutWidth = ImGui::CalcTextSize(shortcut).x;
        const float statusWidth = status ? ImGui::CalcTextSize(status).x : 0.0f;

        if (status)
        {
            ImGui::SetCursorPosX(left);
            const ImVec2 pos = ImGui::GetCursorScreenPos();
            const float line = ImGui::GetTextLineHeight();
            const ImVec4 clip(pos.x, pos.y, pos.x + width, pos.y + line);
            ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
                pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), status, nullptr, 0.0f, &clip);
            ImGui::Dummy(ImVec2((std::min)(statusWidth, width), line));
            if (statusWidth > width)
                ImGui::SetItemTooltip("%s", status);
        }

        if (statusWidth + shortcutWidth + ImGui::GetStyle().ItemSpacing.x <= width)
        {
            if (status)
                ImGui::SameLine();
            ImGui::SetCursorPosX(left + width - shortcutWidth);
            ImGui::TextDisabled("%s", shortcut);
        }
    }

    bool AIChatWindow::SendRequest(
        const char* type, const nlohmann::json& payload,
        const std::shared_ptr<PendingReply>& reply)
    {
        auto& client = AIClient::Instance();
        const auto request = AIMessage::Create(&client, type, payload);
        const std::weak_ptr<PendingReply> target = reply;

        reply->payload = nullptr;
        if (!client.SendPrompt(request, [target](const AIMessage& response)
            {
                if (const auto pending = target.lock())
                    pending->payload = response.payload;
            }))
        {
            m_notice = reinterpret_cast<const char*>(
                u8"メッセージを送信できませんでした");
            return false;
        }

        m_gatewayBusy = true;
        return true;
    }

    void AIChatWindow::Send()
    {
        std::string prompt(m_input.data());
        auto reply = std::make_shared<PendingReply>();
        reply->prompt = prompt;
        if (m_mode == Mode::Agent &&
            SceneManager::state == SceneManager::State::Editing)
        {
            const auto* scene = SceneManager::GetCurrentScene();
            if (scene == SceneManager::GetLoadingSceneOrCurrentScene())
                reply->editSceneAtSend = scene;
        }
        m_focusInput = true;

        const char* type = m_mode == Mode::Agent ? "agent" : "chat";
        if (!SendRequest(type, { {"message", prompt} }, reply))
            return;

        m_messages.push_back({ Role::User, std::move(prompt) });
        m_pending = std::move(reply);
        m_input.fill('\0');
        m_notice = nullptr;
        m_scrollToBottom = true;
    }

    void AIChatWindow::PollReply()
    {
        auto& client = AIClient::Instance();
        client.Update();

        if (!m_gatewayBusy || client.IsProcessing())
            return;

        m_gatewayBusy = false;
        auto reply = std::move(m_pending);
        if (!reply)
            return;

        const auto tool = reply->payload.find("tool_call");
        if (m_mode == Mode::Agent && tool != reply->payload.end())
        {
            if (reply->awaitingFinal || !tool->is_string())
            {
                m_notice = reinterpret_cast<const char*>(u8"不正なツール呼び出しです");
                return;
            }

            const auto& toolName = tool->get_ref<const std::string&>();
            auto* scene = SceneManager::GetCurrentScene();

			// ツール呼び出しの引数を検証し、オブジェクトを作成するラムダ関数
            const auto applyCreateObject = [&](const json& payload) -> bool {
                const auto args = payload.find("arguments");
                if (args == payload.end() || !args->is_object())
                    return false;
                const auto nameArg = args->find("name");
                if (nameArg == args->end() || !nameArg->is_string())
                    return false;
                const auto& name = nameArg->get_ref<const std::string&>();
                if (name.find_first_not_of(' ') == std::string::npos ||
                    std::any_of(name.begin(), name.end(), [](unsigned char ch)
                        {
                            return ch < 0x20 || ch == 0x7f;
                        }))
                    return false;
                const auto kindArg = args->find("kind");
                if (kindArg == args->end() || !kindArg->is_string())
                    return false;
                const auto& kind = kindArg->get_ref<const std::string&>();
                const GameObject* created = nullptr;
                if (kind == "empty")
                    created = GameObjectFactory::Create(scene, name);
                else if (kind == "cube")
                    created = GameObjectFactory::CreateCube(scene, name);
                else if (kind == "sphere")
                    created = GameObjectFactory::CreateSphere(scene, name);
                else if (kind == "cylinder")
                    created = GameObjectFactory::CreateCylinder(scene, name);
                else
                    return false;
                std::string message = created->GetName();
                message += reinterpret_cast<const char*>(u8" を作成しました。");
                reply->payload = { {"message", std::move(message)} };
                return true;
            };

			// ツール呼び出しの引数を検証し、オブジェクトの変換を適用するラムダ関数
            const auto applyEditTransform = [&](const json& payload) -> bool {
                const auto args = payload.find("arguments");
                if (args == payload.end() || !args->is_object())
                    return false;
                const auto idArg = args->find("objectId");
                if (idArg == args->end() || !idArg->is_string())
                    return false;
                ObjectId id = ObjectId::FromString(idArg->get_ref<const std::string&>());
                const auto* object = scene->FindGameObjectById(id);
                if (!object)
                    return false;
                // 変換の設定
                std::string message = object->GetName() + "の";
                const auto positionArg = args->find("position");
                const auto localPositionArg = args->find("localPosition");
                const auto rotationArg = args->find("rotation");
                const auto localRotationArg = args->find("localRotation");
                const auto scaleArg = args->find("scale");
                const auto localScaleArg = args->find("localScale");

                if (positionArg != args->end() && positionArg->is_object())
                {
                    json j = *positionArg;
                    Vector3 position(
                        j.value("x", 0.0f),
                        j.value("y", 0.0f),
                        j.value("z", 0.0f));
                    object->GetTransform()->SetWorldPosition(position);

                    // メッセージを日本語でフォーマットして追加
                    std::string formatedPosition = std::format("({:.2f}, {:.2f}, {:.2f})", position.x, position.y, position.z);
                    std::string setPositionMessage = std::format("positionを {} に設定しました。", formatedPosition);
                    std::u8string messageUtf8 = std::u8string(setPositionMessage.begin(), setPositionMessage.end());
                    message += reinterpret_cast<const char*>(messageUtf8.c_str());
                }
                if (localPositionArg != args->end() && localPositionArg->is_object())
                {
                    json j = *localPositionArg;
                    Vector3 localPosition(
                        j.value("x", 0.0f),
                        j.value("y", 0.0f),
                        j.value("z", 0.0f));
                    object->GetTransform()->SetPosition(localPosition);

                    // メッセージを日本語でフォーマットして追加
                    std::string formatedLocalPosition = std::format("({:.2f}, {:.2f}, {:.2f})", localPosition.x, localPosition.y, localPosition.z);
                    std::string setLocalPositionMessage = std::format("localPositionを {} に設定しました。", formatedLocalPosition);
                    std::u8string messageUtf8 = std::u8string(setLocalPositionMessage.begin(), setLocalPositionMessage.end());
                    message += reinterpret_cast<const char*>(messageUtf8.c_str());
                }
                if (rotationArg != args->end() && rotationArg->is_object())
                {
                    json j = *rotationArg;
                    Vector3 rotation(
                        j.value("x", 0.0f),
                        j.value("y", 0.0f),
                        j.value("z", 0.0f));
                    object->GetTransform()->SetWorldRotation(rotation);

                    // メッセージを日本語でフォーマットして追加
                    std::string formatedRotation = std::format("({:.2f}, {:.2f}, {:.2f})", rotation.x, rotation.y, rotation.z);
                    std::string setRotationMessage = std::format("rotationを {} に設定しました。", formatedRotation);
                    std::u8string messageUtf8 = std::u8string(setRotationMessage.begin(), setRotationMessage.end());
                    message += reinterpret_cast<const char*>(messageUtf8.c_str());
                }
                if (localRotationArg != args->end() && localRotationArg->is_object())
                {
                    json j = *localRotationArg;
                    Vector3 localRotation(
                        j.value("x", 0.0f),
                        j.value("y", 0.0f),
                        j.value("z", 0.0f));
                    object->GetTransform()->SetRotation(localRotation);

                    // メッセージを日本語でフォーマットして追加
                    std::string formatedLocalRotation = std::format("({:.2f}, {:.2f}, {:.2f})", localRotation.x, localRotation.y, localRotation.z);
                    std::string setLocalRotationMessage = std::format("localRotationを {} に設定しました。", formatedLocalRotation);
                    std::u8string messageUtf8 = std::u8string(setLocalRotationMessage.begin(), setLocalRotationMessage.end());
                    message += reinterpret_cast<const char*>(messageUtf8.c_str());
                }
                if (scaleArg != args->end() && scaleArg->is_object())
                {
                    json j = *scaleArg;
                    Vector3 scale(
                        j.value("x", 1.0f),
                        j.value("y", 1.0f),
                        j.value("z", 1.0f));
                    object->GetTransform()->SetWorldScale(scale);

                    // メッセージを日本語でフォーマットして追加
                    std::string formatedScale = std::format("({:.2f}, {:.2f}, {:.2f})", scale.x, scale.y, scale.z);
                    std::string setScaleMessage = std::format("scaleを {} に設定しました。", formatedScale);
                    std::u8string messageUtf8 = std::u8string(setScaleMessage.begin(), setScaleMessage.end());
                    message += reinterpret_cast<const char*>(messageUtf8.c_str());
                }
                if (localScaleArg != args->end() && localScaleArg->is_object())
                {
                    json j = *localScaleArg;
                    Vector3 localScale(
                        j.value("x", 1.0f),
                        j.value("y", 1.0f),
                        j.value("z", 1.0f));
                    j.value("z", 1.0f);
                    object->GetTransform()->SetScale(localScale);

                    // メッセージを日本語でフォーマットして追加
                    std::string formatedLocalScale = std::format("({:.2f}, {:.2f}, {:.2f})", localScale.x, localScale.y, localScale.z);
                    std::string setLocalScaleMessage = std::format("localScaleを {} に設定しました。", formatedLocalScale);
                    std::u8string messageUtf8 = std::u8string(setLocalScaleMessage.begin(), setLocalScaleMessage.end());
                    message += reinterpret_cast<const char*>(messageUtf8.c_str());
                }

                reply->payload = { {"message", std::move(message)} };
                return true;
            };

			// ツール呼び出しの引数を検証し、シーン内のオブジェクトを名前で検索するラムダ関数
            const auto applyFindSceneObjects = [&](const json& payload) -> bool {
                const auto args = payload.find("arguments");
                if (args == payload.end() || !args->is_object())
                    return false;
                const auto nameArg = args->find("name");
                if (nameArg == args->end() || !nameArg->is_string())
                    return false;
                const auto& name = nameArg->get_ref<const std::string&>();
                if (name.find_first_not_of(' ') == std::string::npos)
                    return false;
                std::string message;
                int count = 0;
                for (const auto& object : scene->GetAllSceneObjects())
                {
                    if (!object || object->GetName() != name)
                        continue;
                    // メッセージにオブジェクトの情報を追加
                    if (!message.empty())
                        message += '\n';
                    message += std::format("{}  objectId: {}", name, object->GetId().ToString());
                    if (const auto* parent = object->GetParent())
                    {
                        message += reinterpret_cast<const char*>(u8"  親: ");
                        message += parent->GetName();
                    }
                    count++;
                }
                if (message.empty())
                    message = reinterpret_cast<const char*>(u8"指定された名前のGameObjectが見つかりません。");
                reply->payload = { {"message", std::move(message)} };
                return true;
            };

			// ツール呼び出しの種類に応じて処理を分岐
            if (toolName == "CreateObject")
			{ // オブジェクトを作成するツール呼び出し
                if (!scene || scene != reply->editSceneAtSend ||
                    SceneManager::state != SceneManager::State::Editing ||
                    SceneManager::GetLoadingSceneOrCurrentScene() != scene)
                {
                    m_notice = reinterpret_cast<const char*>(
                        u8"送信時のシーンを編集中に実行してください");
                    return;
                }

				// ツール呼び出しの引数を検証し、オブジェクトを作成する
                if (!applyCreateObject(reply->payload))
                {
                    m_notice = reinterpret_cast<const char*>(u8"引数が不正です");
                    return;
				}
            }
            else if (toolName == "EditTransform")
			{ // オブジェクトのTransformを編集するツール呼び出し
                if (!scene)
                {
                    m_notice = reinterpret_cast<const char*>(u8"現在のシーンがありません");
                    return;
                }

				// ツール呼び出しの引数を検証し、オブジェクトの変換を適用する
                if (!applyEditTransform(reply->payload))
                {
                    m_notice = reinterpret_cast<const char*>(u8"引数が不正です");
					return;
                }
            }
            else if (toolName == "GetSceneObjectCount")
			{ // シーン内のオブジェクト数を取得するツール呼び出し
                if (!scene)
                {
                    m_notice = reinterpret_cast<const char*>(u8"現在のシーンがありません");
                    return;
                }

                reply->awaitingFinal = true;
                if (SendRequest("agent_tool_result",
                    {
                        {"message", reply->prompt},
                        {"count", scene->GetSceneObjectsSize()},
                    }, reply))
                {
                    m_pending = std::move(reply);
                }
                return;
            }
            else if (toolName == "FindSceneObjects")
			{ // シーン内のオブジェクトを名前で検索するツール呼び出し
                if (!scene)
                {
                    m_notice = reinterpret_cast<const char*>(u8"現在のシーンがありません");
                    return;
                }

                const auto args = reply->payload.find("arguments");
                if (args == reply->payload.end() || !args->is_object())
                {
                    m_notice = reinterpret_cast<const char*>(u8"検索の引数が不正です");
                    return;
                }

                const auto nameArg = args->find("name");
                if (nameArg == args->end() || !nameArg->is_string() ||
                    nameArg->get_ref<const std::string&>().empty())
                {
                    m_notice = reinterpret_cast<const char*>(u8"検索する名前を指定してください");
                    return;
                }

                const auto& name = nameArg->get_ref<const std::string&>();
                std::string message{};
				int count = 0;
                std::string foundIdStr{ ObjectId::Invalid().ToString() };
                for (const auto& object : scene->GetAllSceneObjects())
                {
                    if (!object || object->GetName() != name)
                        continue;

					// メッセージにオブジェクトの情報を追加
                    if (!message.empty())
                        message += '\n';

                    foundIdStr = object->GetId().ToString();
                    message += std::format("{}  objectId: {}", name, foundIdStr);
                    if (const auto* parent = object->GetParent())
                    {
                        message += reinterpret_cast<const char*>(u8"  親: ");
                        message += parent->GetName();
                    }
					count++;
                }

                if (count == 1)
                {
					// 1つだけ見つかった場合、thenが指定されていればその処理を実行する
                    const auto thenArg = reply->payload.find("then");
                    if (thenArg == reply->payload.end())
                    {
                        // 名前検索だけの依頼では、これまでどおり候補を表示する。
                        if (message.empty())
                        {
                            message = reinterpret_cast<const char*>(
                                u8"指定された名前のGameObjectが見つかりません。");
                        }

                        reply->payload = { {"message", message} };
                    }
                    else
                    {
                        // 検索に続いて編集する場合は、送信時の編集シーンを確認する。
                        if (scene != reply->editSceneAtSend ||
                            SceneManager::state != SceneManager::State::Editing ||
                            SceneManager::GetLoadingSceneOrCurrentScene() != scene)
                        {
                            m_notice = reinterpret_cast<const char*>(
                                u8"送信時のシーンを編集中に実行してください");
                            return;
                        }

                        if (count != 1)
                        {
                            m_notice = count == 0
                                ? reinterpret_cast<const char*>(u8"指定されたGameObjectが見つかりません")
                                : reinterpret_cast<const char*>(u8"同名のGameObjectが複数あります。objectIdを指定してください");
                            return;
                        }

                        if (!thenArg->is_object() ||
                            !thenArg->contains("tool") ||
                            thenArg->at("tool") != "EditTransform")
                        {
                            m_notice = reinterpret_cast<const char*>(u8"後続のツール指定が不正です");
                            return;
                        }

                        const auto editArgs = thenArg->find("arguments");
                        if (editArgs == thenArg->end() || !editArgs->is_object() || editArgs->empty())
                        {
                            m_notice = reinterpret_cast<const char*>(u8"編集の引数が不正です");
                            return;
                        }

                        // 座標などの編集内容を残し、検索で特定した objectId を補う。
                        json resolvedArgs = *editArgs;
                        resolvedArgs["objectId"] = foundIdStr;

                        const json editPayload = { {"arguments", (resolvedArgs)} };
                        if (!applyEditTransform(editPayload))
                            return;
                    }
				}
            }
            else
            {
                m_notice = reinterpret_cast<const char*>(u8"不明なツールです");
                return;
            }
        }

        const auto text = reply->payload.find("message");
        if (text == reply->payload.end() || !text->is_string() ||
            text->get_ref<const std::string&>().empty())
        {
            m_notice = reinterpret_cast<const char*>(u8"有効な応答を受信できませんでした");
            return;
        }

        m_messages.push_back({ Role::Assistant, text->get<std::string>() });
        m_notice = nullptr;
        m_scrollToBottom = m_scrollToBottom || m_followLatest;
    }
}
