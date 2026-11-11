#pragma once
#define WIN32_LEAN_AND_MEAN

#include <json.hpp>

namespace CurryEngine::EditorAssistant
{
	class GatewayTcpClient;

	class GatewayProtocol
	{
	public:

		/**
		 * @brief JSON形式のデータを送信する
		 * @param json 送信するJSONデータ
		 * @return 送信に成功した場合はtrue、失敗した場合はfalse
		 */
		static bool SendJsonData(GatewayTcpClient& client, const nlohmann::json& json);

		/**
		 * @brief JSON形式のデータを受信する
		 * @param json 受信したJSONデータを格納する変数
		 * @return 受信に成功した場合はtrue、失敗した場合はfalse
		 */
		static bool ReceiveJsonData(GatewayTcpClient& client, nlohmann::json& json);

	};
}
