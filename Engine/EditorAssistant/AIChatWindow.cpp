#include "pch.h"
#include "AIChatWindow.h"
#include "Engine/EditorAssistant/AIClient.h"
#include "imgui.h"

#include <algorithm>
#include <cfloat>
#include <string_view>
#include <utility>

namespace CurryEngine::EditorAssistant
{
    AIChatWindow::AIChatWindow(Mode mode)
        : m_mode(mode)
    {
    }

    void AIChatWindow::Draw(bool& open)
    {
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

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + inset);
            if (ImGui::Button(reinterpret_cast<const char*>(u8"新規チャット")))
                NewChat();

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
        else if (!status && m_mode == Mode::Demo)
            status = reinterpret_cast<const char*>(u8"デモ");

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

    void AIChatWindow::Send()
    {
        std::string prompt(m_input.data());
        auto reply = std::make_shared<PendingReply>();
        m_focusInput = true;

        if (m_mode == Mode::Gateway)
        {
            auto& client = AIClient::Instance();
            const auto request = AIMessage::Create(
                &client, "chat", nlohmann::json{ {"message", prompt} });
            const std::weak_ptr<PendingReply> target = reply;

            if (!client.SendPrompt(request, [target](const AIMessage& response)
                {
                    const auto pending = target.lock();
                    if (!pending)
                        return;

                    const auto value = response.payload.find("message");
                    if (value != response.payload.end() && value->is_string())
                        pending->text = value->get<std::string>();
                }))
            {
                m_notice = reinterpret_cast<const char*>(u8"メッセージを送信できませんでした");
                return;
            }

            m_gatewayBusy = true;
        }
        else
        {
            m_demoReplyAt = ImGui::GetTime() + 0.7;
        }

        m_messages.push_back({ Role::User, std::move(prompt) });
        m_pending = std::move(reply);
        m_input.fill('\0');
        m_notice = nullptr;
        m_scrollToBottom = true;
    }

    void AIChatWindow::PollReply()
    {
        if (m_mode == Mode::Gateway)
        {
            auto& client = AIClient::Instance();
            client.Update();
            if (!m_gatewayBusy || client.IsProcessing())
                return;
            m_gatewayBusy = false;
        }
        else
        {
            if (!m_pending || ImGui::GetTime() < m_demoReplyAt)
                return;

            m_pending->text = reinterpret_cast<const char*>(
                u8"選択したオブジェクトを移動するには、"
                u8"プロパティエディターで Transform を開き、"
                u8"Position の X・Y・Z を変更します。\n\n"
                u8"シーン上の移動ギズモでも位置を調整できます。");
        }

        if (!m_pending)
            return;

        if (m_pending->text.empty())
        {
            m_notice = reinterpret_cast<const char*>(u8"有効な応答を受信できませんでした");
        }
        else
        {
            m_messages.push_back({ Role::Assistant, std::move(m_pending->text) });
            m_notice = nullptr;
            m_scrollToBottom = m_scrollToBottom || m_followLatest;
        }

        m_pending.reset();
    }
}
