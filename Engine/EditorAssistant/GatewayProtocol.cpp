#include "pch.h"
#include "GatewayProtocol.h"
#include "GatewayTcpClient.h"

#include <array>
#include <cstdint>
#include <string>

namespace CurryEngine::EditorAssistant
{
	namespace
	{
		constexpr std::uint32_t MAX_MESSAGE_SIZE = 1024 * 1024; // 1MiB

		using Header = std::array<std::uint8_t, sizeof(std::uint32_t)>;
	}

	bool GatewayProtocol::SendJsonData(GatewayTcpClient& client, const nlohmann::json& json)
	{
		if (!client.IsConnected())
		{
			LOG_ERROR(u8"[GatewayProtocol] サーバーに接続されていません。");
			return false;
		}
		if (!json.is_object())
		{
			LOG_ERROR(u8"[GatewayProtocol] 送信するJSONデータがオブジェクトではありません。");
			return false;
		}

		std::string payload = json.dump();

		if (payload.empty())
		{
			LOG_ERROR(u8"[GatewayProtocol] 送信するJSONデータが空です。");
			return false;
		}

		if (payload.size() > MAX_MESSAGE_SIZE)
		{
			std::string errorMessage = "送信するJSONデータが大きすぎます。サイズ: " + std::to_string(payload.size()) + " バイト";
			std::u8string u8ErrorMessage(errorMessage.begin(), errorMessage.end());
			LOG_ERROR(u8"[GatewayProtocol] " + u8ErrorMessage);
			return false;
		}

		const auto length = static_cast<std::uint32_t>(payload.size());

		// 4バイトのビッグエンディアン
		const Header header = {
			static_cast<std::uint8_t>(length >> 24),
			static_cast<std::uint8_t>(length >> 16),
			static_cast<std::uint8_t>(length >> 8),
			static_cast<std::uint8_t>(length)
		};

		// ヘッダーを送信
		if (!client.SendAll(header.data(), header.size()))
		{
			LOG_ERROR(u8"[GatewayProtocol] ヘッダーの送信に失敗しました。");
			return false;
		}
		// JSONペイロードを送信
		if (!client.SendAll(payload.data(), payload.size()))
		{
			LOG_ERROR(u8"[GatewayProtocol] JSONペイロードの送信に失敗しました。");
			return false;
		}
		return true;
	}
	bool GatewayProtocol::ReceiveJsonData(GatewayTcpClient& client, nlohmann::json& json)
	{
		Header header{};

		// ヘッダーを受信
		if (!client.ReceiveExact(header.data(), header.size()))
		{
			LOG_ERROR(u8"[GatewayProtocol] ヘッダーの受信に失敗しました。");
			return false;
		}

		const std::uint32_t length =
			(std::uint32_t(header[0]) << 24) |
			(std::uint32_t(header[1]) << 16) |
			(std::uint32_t(header[2]) << 8) |
			std::uint32_t(header[3]);

		if (length == 0)
		{
			LOG_ERROR(u8"[GatewayProtocol] 受信するJSONデータのサイズが0です。");
			return false;
		}

		if (length > MAX_MESSAGE_SIZE)
		{
			std::string errorMessage = "受信するJSONデータが大きすぎます。サイズ: " + std::to_string(length) + " バイト";
			std::u8string u8ErrorMessage(errorMessage.begin(), errorMessage.end());
			LOG_ERROR(u8"[GatewayProtocol] " + u8ErrorMessage);
			return false;
		}

		std::string payload(length, '\0');

		// JSONペイロードを受信
		if (!client.ReceiveExact(payload.data(), payload.size()))
		{
			LOG_ERROR(u8"[GatewayProtocol] JSONペイロードの受信に失敗しました。");
			return false;
		}

		try
		{
			auto parsed = nlohmann::json::parse(payload);

			if (!parsed.is_object())
			{
				LOG_ERROR(u8"[GatewayProtocol] 受信したJSONデータがオブジェクトではありません。");
				return false;
			}

			json = std::move(parsed);
			return true;
		}
		catch (const nlohmann::json::parse_error& e)
		{
			std::string errorMessage = "JSONの解析に失敗しました。エラー: " + std::string(e.what());
			std::u8string u8ErrorMessage(errorMessage.begin(), errorMessage.end());
			LOG_ERROR(u8"[GatewayProtocol] " + u8ErrorMessage);
			return false;
		}
	}

}
