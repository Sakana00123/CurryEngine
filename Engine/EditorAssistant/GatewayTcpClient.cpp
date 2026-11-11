#include "pch.h"
#include "GatewayTcpClient.h"
#pragma comment(lib, "Ws2_32.lib")

#define MIN_CHUNK_SIZE 65536

namespace CurryEngine::EditorAssistant
{
	GatewayTcpClient::~GatewayTcpClient()
	{
		Disconnect();
	}

	bool GatewayTcpClient::Connect(const char* host, std::uint16_t port)
	{
		Disconnect();

		// Winsock2の初期化
		WSADATA wsaData;
		int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (result != 0)
		{
			std::string errorMessage = "Winsockの初期化に失敗しました。エラーコード: " + std::to_string(result);
			std::u8string u8ErrorMessage(errorMessage.begin(), errorMessage.end());
			LOG_ERROR(u8"[GatewayTcpClient] " + u8ErrorMessage);
			return false;
		}
		m_winsockInitialized = true;

		// TCPソケットを作成
		m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		
		if (m_socket == INVALID_SOCKET)
		{
			std::string errorMessage = "ソケットの作成に失敗しました。エラーコード: " + std::to_string(WSAGetLastError());
			std::u8string u8ErrorMessage(errorMessage.begin(), errorMessage.end());
			LOG_ERROR(u8"[GatewayTcpClient] " + u8ErrorMessage);
			Disconnect();
			return false;
		}

		// 接続先アドレスを設定
		sockaddr_in serverAddr{};
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(port);

		if (inet_pton(AF_INET, host, &serverAddr.sin_addr) != 1)
		{
			LOG_ERROR(u8"[GatewayTcpClient] ホストアドレスの変換に失敗しました。");
			Disconnect();
			return false;
		}

		// TCPサーバーに接続
		if (connect(m_socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
		{
			std::string errorMessage = "サーバーへの接続に失敗しました。エラーコード: " + std::to_string(WSAGetLastError());
			std::u8string u8ErrorMessage(errorMessage.begin(), errorMessage.end());
			LOG_ERROR(u8"[GatewayTcpClient] " + u8ErrorMessage);
			Disconnect();
			return false;
		}

		LOG_INFO(u8"[GatewayTcpClient] サーバーに接続しました。");
		return true;
	}

	void GatewayTcpClient::Disconnect()
	{
		if (m_socket != INVALID_SOCKET)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}
		if (m_winsockInitialized)
		{
			WSACleanup();
			m_winsockInitialized = false;
		}
	}

	bool GatewayTcpClient::IsConnected() const
	{
		return m_socket != INVALID_SOCKET;
	}

	bool GatewayTcpClient::SendAll(const void* data, size_t size)
	{
		if (!IsConnected())
		{
			LOG_ERROR(u8"[GatewayTcpClient] サーバーに接続されていません。");
			return false;
		}

		if (!data && size > 0)
		{
			LOG_ERROR(u8"[GatewayTcpClient] 送信データが無効です。");
			return false;
		}

		const char* bytes = static_cast<const char*>(data);
		size_t sent = 0;

		while (sent < size)
		{
			const int chunk = static_cast<int>(std::min<size_t>(size - sent, MIN_CHUNK_SIZE));
			
			const int result = send(m_socket, bytes + sent, chunk, 0);

			if (result == SOCKET_ERROR)
			{
				std::string errorMessage = "データの送信に失敗しました。エラーコード: " + std::to_string(WSAGetLastError());
				std::u8string u8ErrorMessage(errorMessage.begin(), errorMessage.end());
				LOG_ERROR(u8"[GatewayTcpClient] " + u8ErrorMessage);
				return false;
			}

			if (result == 0)
			{
				LOG_ERROR(u8"[GatewayTcpClient] サーバーが切断されました。");
				return false;
			}

			sent += static_cast<size_t>(result);
		}

		return true;
	}

	bool GatewayTcpClient::ReceiveExact(void* buffer, size_t size)
	{
		if (!IsConnected())
		{
			LOG_ERROR(u8"[GatewayTcpClient] サーバーに接続されていません。");
			return false;
		}

		if (!buffer && size > 0)
		{
			LOG_ERROR(u8"[GatewayTcpClient] 受信バッファが無効です。");
			return false;
		}

		char* bytes = static_cast<char*>(buffer);
		size_t received = 0;

		while (received < size)
		{
			const int chunk = static_cast<int>(std::min<size_t>(size - received, MIN_CHUNK_SIZE));

			const int result = recv(m_socket, bytes + received, chunk, 0);

			if (result == SOCKET_ERROR)
			{
				std::string errorMessage = "データの受信に失敗しました。エラーコード: " + std::to_string(WSAGetLastError());
				std::u8string u8ErrorMessage(errorMessage.begin(), errorMessage.end());
				LOG_ERROR(u8"[GatewayTcpClient] " + u8ErrorMessage);
				return false;
			}
			else if (result == 0)
			{
				LOG_ERROR(u8"[GatewayTcpClient] サーバーが切断されました。");
				return false;
			}

			received += static_cast<size_t>(result);
		}
		return true;
	}
}
