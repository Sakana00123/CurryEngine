#pragma once

#include <string>
#include <json.hpp>
#include <memory>
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include "GatewayTcpClient.h"

namespace CurryEngine::EditorAssistant
{
	
	using RequestId = std::uint16_t;
	static constexpr RequestId INVALID_REQUEST_ID = 0;
	static constexpr RequestId MAX_REQUEST_ID = 65535;
	static constexpr RequestId MIN_REQUEST_ID = 1;
	static constexpr int MAX_REQUESTS = UINT16_MAX - 1; // 最大リクエスト数（65534）
	static constexpr int MAX_PAYLOAD_SIZE = 1024 * 1024; // 最大ペイロードサイズ（1MiB）

	/**
	 * @brief クライアントとサーバー間で送受信されるAIリクエストのメッセージ構造体
	 */
	struct AIMessage
	{
		std::string requestId;
		std::string type;
		nlohmann::json payload;

		/**
		 * @brief AIMessageを作成する
		 * @param client AIClientのインスタンス
		 * @param type リクエストの種類
		 * @param payload リクエストのペイロード（JSON形式）
		 * @return 作成されたAIMessage
		 */
		static AIMessage Create(class AIClient* client, const std::string& type, const nlohmann::json& payload);

		/**
		 * @brief AIMessageが有効かどうかを確認する
		 * @return 有効な場合はtrue、無効な場合はfalse
		 */
		bool IsValid() const;
	};

	/**
	 * @brief AIリクエストの結果を表す構造体
	 */
	struct AIResult
	{
		bool success = false;
		AIMessage message;
		std::string error;
	};

	/**
	 * @brief AIサーバーへの接続とリクエスト送信を行うクライアントクラス
	 */
	class AIClient
	{
	public:
		using ResponseCallback = std::function<void(const AIMessage&)>;

		static AIClient& Instance()
		{
			static AIClient instance;
			return instance;
		}

		AIClient() = default;
		~AIClient();
		AIClient(const AIClient&) = delete;
		AIClient& operator=(const AIClient&) = delete;

		/**
		 * @brief クライアントを開始する
		 * @return 開始に成功した場合はtrue、失敗した場合はfalse
		 */
		bool Start();

		/**
		 * @brief クライアントを停止する
		 */
		void Stop();

		/**
		 * @brief AIサーバーに接続する
		 * @param host 接続先のホスト名またはIPアドレス
		 * @param port 接続先のポート番号
		 * @return 接続に成功した場合は0、失敗した場合はエラーコードを返す
		 */
		bool Connect(const std::string& host = "127.0.0.1", std::uint16_t port = 8765);
		
		/**
		 * @brief AIサーバーから切断する
		 */
		bool Disconnect();

		/**
		 * @brief AIサーバーにリクエストを送信する
		 * @param message 送信するリクエストのAIMessage
		 * @param callback レスポンスを受信した際に呼び出される
		 * @return 送信に成功した場合はtrue、失敗した場合はfalse
		 */
		bool SendPrompt(const AIMessage& message, const ResponseCallback& callback = nullptr);

		/**
		 * @brief クライアントの状態を更新する
		 * @note この関数は定期的に呼び出す必要があります。AIサーバーからのレスポンスを受信するために使用されます。
		 */
		void Update();

		/**
		 * @brief AIサーバーへの接続状態を確認する
		 * @return 接続されている場合はtrue、切断されている場合はfalse
		 */
		bool IsConnected() const;

		/**
		 * @brief クライアントが実行中かどうかを確認する
		 * @return 実行中の場合はtrue、停止中の場合はfalse
		 */
		bool IsRunning() const;

		/**
		 * @brief リクエストの処理中かどうかを確認する
		 * @return 処理中の場合はtrue、処理が完了している場合はfalse
		 */
		bool IsProcessing() const;

		/**
		 * @brief 次のリクエストIDを取得する
		 * @return 次のリクエストID
		 */
		RequestId GetNextRequestId();

	private:
		struct Request
		{
			AIMessage message;
			ResponseCallback callback;
		};

		struct Completion
		{
			AIResult result;
			ResponseCallback callback;
		};

		void WorkerLoop();

		mutable std::mutex m_mutex;
		std::condition_variable m_cv;

		std::thread m_worker;

		std::optional<Request> m_pending;
		std::optional<Completion> m_completed;

		RequestId m_nextRequestId = 1;

		bool m_running = false;
		bool m_stopping = false;
		bool m_busy = false;


		std::unique_ptr<GatewayTcpClient> m_tcpClient;
	};
}
