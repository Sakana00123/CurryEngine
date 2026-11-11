#pragma once

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <cstdint>
#include <cstddef>

namespace CurryEngine::EditorAssistant
{
	class GatewayTcpClient
	{
	public:
		GatewayTcpClient() = default;
		~GatewayTcpClient();

		GatewayTcpClient(const GatewayTcpClient&) = delete;
		GatewayTcpClient& operator=(const GatewayTcpClient&) = delete;

		/**
		 * @brief TCPサーバーに接続する
		 * @param host 接続先のホスト名またはIPアドレス
		 * @param port 接続先のポート番号
		 * @return 接続に成功した場合はtrue、失敗した場合はfalse
		 */
		bool Connect(const char* host, std::uint16_t port);

		/**
		 * @brief TCPサーバーから切断する
		 */
		void Disconnect();

		/**
		 * @brief TCPサーバーへの接続状態を確認する
		 * @return 接続されている場合はtrue、切断されている場合はfalse
		 */
		bool IsConnected() const;

		/**
		 * @brief TCPサーバーにデータを送信する
		 * @param data 送信するデータのポインタ
		 * @param size 送信するデータのサイズ（バイト単位）
		 * @return 送信に成功した場合はtrue、失敗した場合はfalse
		 */
		bool SendAll(const void* data, size_t size);

		/**
		 * @brief TCPサーバーからデータを受信する
		 * @param buffer 受信したデータを格納するバッファのポインタ
		 * @param size 受信するデータのサイズ（バイト単位）
		 * @return 受信に成功した場合はtrue、失敗した場合はfalse
		 */
		bool ReceiveExact(void* buffer, size_t size);

	private:
		// Winsock2のソケット
		SOCKET m_socket = INVALID_SOCKET;
		bool m_winsockInitialized = false;
	};
}

