#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

namespace CurryEngine::EditorAssistant
{
    class AIChatWindow final
    {
    public:
        enum class Mode { Demo, Gateway };

        explicit AIChatWindow(Mode mode = Mode::Demo);
        AIChatWindow(const AIChatWindow&) = delete;
        AIChatWindow& operator=(const AIChatWindow&) = delete;

        // ウィンドウが閉じている間も、メインスレッドで毎フレーム呼ぶ。
        void Draw(bool& open);

    private:
        enum class Role { User, Assistant };

        struct Message
        {
            Role role;
            std::string text;
        };

        struct PendingReply
        {
            std::string text;
        };

        void NewChat();
        void DrawHistory(float inset, float width, float footerHeight);
        void DrawComposer(float inset, float width, float height);
        void DrawFooter(float inset, float width) const;
        void Send();
        void PollReply();

        static void DrawBubble(const std::string& text, float width);
        static bool DrawActionButton(bool stop, float size);

        Mode m_mode;
        std::array<char, 4096> m_input{};
        std::vector<Message> m_messages;
        std::shared_ptr<PendingReply> m_pending;
        const char* m_notice = nullptr;
        double m_demoReplyAt = 0.0;
        bool m_gatewayBusy = false;
        bool m_followLatest = true;
        bool m_scrollToBottom = false;
        bool m_focusInput = false;
    };
}
