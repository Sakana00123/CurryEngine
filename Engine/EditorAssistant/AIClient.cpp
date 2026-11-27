#include "pch.h"
#include "AIClient.h"
#include "GatewayTcpClient.h"
#include "GatewayProtocol.h"

namespace CurryEngine::EditorAssistant
{

	AIMessage AIMessage::Create(AIClient* client, const std::string& type, const nlohmann::json& payload)
	{
		AIMessage request;
		if (client)
		{
			request.requestId = std::to_string(client->GetNextRequestId());
		}
		else
		{
			request.requestId = "0"; // クライアントがnullptrの場合はリクエストIDを0に設定
		}
		request.type = type;
		request.payload = payload;
		return request;
	}

	bool AIMessage::IsValid() const
	{
		return !requestId.empty() && !type.empty();
	}

	AIClient::~AIClient()
	{
		Stop();
	}

	bool AIClient::Start()
	{
		std::lock_guard lock(m_mutex);

		if (m_running)
			return false;

		m_stopping = false;
		m_busy = false;
		m_pending.reset();
		m_completed.reset();

		try
		{
			m_worker = std::thread(
				&AIClient::WorkerLoop, this);
		}
		catch (...)
		{
			return false;
		}

		m_running = true;
		return true;
	}

	void AIClient::Stop()
	{
		{
			std::lock_guard lock(m_mutex);

			if (!m_running)
				return;

			m_stopping = true;
		}

		m_cv.notify_one();

		if (m_worker.joinable())
			m_worker.join();

		{
			std::lock_guard lock(m_mutex);

			m_pending.reset();
			m_completed.reset();
			m_busy = false;
			m_running = false;
		}
	}


	bool AIClient::Connect(const std::string& host, std::uint16_t port)
	{
		m_tcpClient = std::make_unique<GatewayTcpClient>();
		if (!m_tcpClient->Connect(host.c_str(), port))
		{
			LOG_ERROR(u8"[AIClient] AIサーバーへの接続に失敗しました。");
			return false;
		}

		Start();

		return true;
	}

	bool AIClient::Disconnect()
	{
		if (!m_tcpClient)
		{
			LOG_ERROR(u8"[AIClient] GatewayTcpClientがnullptrです。");
			return false;
		}
		m_tcpClient->Disconnect();

		Stop();
		return true;
	}

	bool AIClient::SendPrompt(const AIMessage& message, const ResponseCallback& callback)
	{
		{
			std::lock_guard lock(m_mutex);

			if (!m_running || m_stopping || m_busy)
				return false;

			if (!callback || message.requestId.empty())
				return false;

			Request request;
			request.message = message;
			request.callback = callback;

			m_pending = std::move(request);
			m_busy = true;
		}

		m_cv.notify_one();

		return true;
	}



	void AIClient::Update()
	{
		std::optional<Completion> completion;

		{
			std::lock_guard lock(m_mutex);

			if (!m_completed)
				return;

			completion = std::move(m_completed);
			m_completed.reset();
		}

		if (!completion.has_value())
			return;

		if (!completion->result.success)
		{
			std::string errorMessage = completion->result.error.empty() ? "不明なエラー" : completion->result.error;
			std::u8string errorMessageUtf8 = std::u8string(errorMessage.begin(), errorMessage.end());
			LOG_ERROR(u8"[AIClient] AIサーバーからの応答にエラーが含まれています: " + errorMessageUtf8);
		}

		// mutexを解放してから実行
		try
		{
			if (completion->callback)
				completion->callback(completion->result.message);
		}
		catch (const std::exception& e)
		{
			std::string errorMessage = e.what();
			std::u8string errorMessageUtf8 = std::u8string(errorMessage.begin(), errorMessage.end());
			LOG_ERROR(u8"[AIClient] コールバックの実行中に例外が発生しました: " + errorMessageUtf8);
		}
		catch (...)
		{
			// 不明な例外が発生した場合はログに記録する
			LOG_ERROR(u8"[AIClient] コールバックの実行中に不明な例外が発生しました。");
		}

		{
			std::lock_guard lock(m_mutex);
			m_busy = false;
		}
	}


	bool AIClient::IsConnected() const
	{
		if (!m_tcpClient)
		{
			LOG_ERROR(u8"[AIClient] GatewayTcpClientがnullptrです。");
			return false;
		}
		return m_tcpClient->IsConnected();
	}

	bool AIClient::IsRunning() const
	{
		if (!m_tcpClient)
		{
			LOG_ERROR(u8"[AIClient] GatewayTcpClientがnullptrです。");
			return false;
		}

		std::lock_guard lock(m_mutex);
		return m_running && !m_stopping;
	}

	bool AIClient::IsProcessing() const
	{
		if (!m_tcpClient)
		{
			LOG_ERROR(u8"[AIClient] GatewayTcpClientがnullptrです。");
			return false;
		}

		std::lock_guard lock(m_mutex);
		return m_busy;
	}

	RequestId AIClient::GetNextRequestId()
	{
		return m_nextRequestId++;
	}


	void AIClient::WorkerLoop()
	{
		if (!m_tcpClient)
		{
			LOG_ERROR(u8"[AIClient] GatewayTcpClientがnullptrです。");
			return;
		}

		while (true)
		{
			Request request;

			// リクエストを待つ
			{
				std::unique_lock lock(m_mutex);

				m_cv.wait(lock, [this]
					{
						return m_stopping || m_pending.has_value();
					});

				if (m_stopping)
					break;

				request = std::move(*m_pending);
				m_pending.reset();
			}

			AIResult result;

			// 未接続なら接続する
			if (!m_tcpClient->IsConnected())
			{
				if (!m_tcpClient->Connect("127.0.0.1", 8765))
				{
					result.error = "Gatewayへの接続に失敗";
				}
			}

			if (m_tcpClient->IsConnected())
			{
				nlohmann::json jsonRequest = {
					{"request_id", request.message.requestId},
					{"type", request.message.type},
					{"payload", request.message.payload}
				};

				nlohmann::json jsonResponse;

				if (!GatewayProtocol::SendJsonData(
					*m_tcpClient, jsonRequest))
				{
					result.error = "送信に失敗";
				}
				else if (!GatewayProtocol::ReceiveJsonData(
					*m_tcpClient, jsonResponse))
				{
					result.error = "受信に失敗";
				}
				else
				{
					// 応答を検証する
					try
					{
						const auto id =
							jsonResponse.at("request_id")
							.get<std::string>();

						const auto type =
							jsonResponse.at("type")
							.get<std::string>();

						if (id != request.message.requestId)
						{
							result.error =
								"request_idが一致しません";
						}
						else if (type == "chat_response")
						{
							result.message.payload =
								jsonResponse.at("payload");

							result.success = true;
						}
						else if (type == "error")
						{
							result.error =
								jsonResponse.at("payload")
								.at("message")
								.get<std::string>();
						}
						else
						{
							result.error = "不正な応答タイプ";
						}
					}
					catch (const nlohmann::json::exception& e)
					{
						result.error = std::format("応答JSONが不正です: {}", e.what());
					}
				}
			}

			// エラーが発生した場合は切断してログに記録
			if (!result.success)
			{
				Disconnect();
				LOG_ERROR(u8"[AIClient] AIサーバーとの通信中にエラーが発生しました: " + std::u8string(result.error.begin(), result.error.end()));
				return;
			}

			// 完了結果を保存
			{
				std::lock_guard lock(m_mutex);

				if (m_stopping)
					break;

				m_completed = Completion{
					std::move(result),
					std::move(request.callback)
				};
			}
		}
	}
}
