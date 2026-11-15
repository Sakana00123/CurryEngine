#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>
#include <json.hpp>

namespace CurryEngine::EditorAssistant
{
    /**
	 * @brief AIチャットウィンドウ
     */
    class AIChatWindow
    {
    public:
        enum class Mode { Chat, Agent };

        explicit AIChatWindow(Mode mode = Mode::Chat);
        AIChatWindow(const AIChatWindow&) = delete;
        AIChatWindow& operator=(const AIChatWindow&) = delete;

        /**
         * @brief ウィンドウを描画する
         * @param open ウィンドウが開いているかどうかのフラグ
		 */
        void Draw(bool& open);

    private:
        /**
         * @brief メッセージの送信者の役割
		 */
        enum class Role { User, Assistant };

        /**
		 * @brief メッセージ
         */
        struct Message
        {
            Role role;
            std::string text;
        };

		/**
         * @brief 返信待ちのメッセージ
		 */
        struct PendingReply
        {
            std::string prompt;
            nlohmann::json payload;
            bool awaitingFinal = false;
        };

        void NewChat();
        void DrawToolbar(float inset, float width);
        void DrawHistory(float inset, float width, float footerHeight);
        void DrawComposer(float inset, float width, float height);
        void DrawFooter(float inset, float width) const;
        bool SendRequest(const char* type, const nlohmann::json& payload, const std::shared_ptr<PendingReply>& reply);
        void Send();
        void PollReply();

        static void DrawBubble(const std::string& text, float width);
        static bool DrawActionButton(bool stop, float size);

        Mode m_mode;
        std::array<char, 4096> m_input{};
        std::vector<Message> m_messages;
        std::shared_ptr<PendingReply> m_pending;
        const char* m_notice = nullptr;
        bool m_gatewayBusy = false;
        bool m_followLatest = true;
        bool m_scrollToBottom = false;
        bool m_focusInput = false;
    };
}
