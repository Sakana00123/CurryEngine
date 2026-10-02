#pragma once
#include <filesystem>
#include <string>
#include <queue>

/** @brief VcxprojType を表す列挙型です。 */
enum class VcxprojType
{
	Engine,
	Game,
	Editor,
};

/** @brief VcxprojHelper を表すクラスです。 */
class VcxprojHelper
{
public:
	// シェーダーの登録をキューに追加（実行終了前にまとめて処理）
	/**
	 * @brief EnqueueShaderRegistration の処理を行います。
	 */
	static void EnqueueShaderRegistration(const std::filesystem::path& shaderPath);

	// シェーダーの登録解除（vcxprojからエントリを削除）
	/**
	 * @brief EnqueueShaderUnregistration の処理を行います。
	 */
	static void EnqueueShaderUnregistration(const std::filesystem::path& shaderPath);


	// シェーダーがすでにプロジェクトに登録されているかどうか
	/**
	 * @brief IsShaderRegistered の条件を満たすか判定します。
	 * @return 処理結果を返します。
	 */
	static bool IsShaderRegistered(const std::filesystem::path& shaderPath);

	// シェーダーの登録がキューに入っているかどうか
	/**
	 * @brief IsShaderRegistrationPending の条件を満たすか判定します。
	 * @return 処理結果を返します。
	 */
	static bool IsShaderRegistrationPending(const std::filesystem::path& shaderPath);


	// 登録待ちのシェーダーをまとめて処理（実行終了前に呼ぶこと）
	/**
	 * @brief ProcessPendingShaderRegistrations の処理を行います。
	 */
	static void ProcessPendingShaderRegistrations();
	
	// シェーダーの登録解除をキューに追加（実行終了前にまとめて処理）
	/**
	 * @brief ProcessPendingShaderUnregistrations の処理を行います。
	 */
	static void ProcessPendingShaderUnregistrations();

private:

	// HLSLシェーダーをプロジェクトに登録（vcxprojにCompileItemを追加）
	/**
	 * @brief 指定された要素を登録します。
	 */
	static bool RegisterHLSLShader(const std::filesystem::path& shaderPath);

	// HLSLIファイルをプロジェクトに登録（vcxprojにNoneItemを追加）
	/**
	 * @brief 指定された要素を登録します。
	 */
	static bool RegisterHLSLIFile(const std::filesystem::path& shaderPath);

	// HLSLシェーダーをプロジェクトから登録解除（vcxprojからCompileItemを削除）
	/**
	 * @brief 指定された要素の登録を解除します。
	 */
	static bool UnregisterHLSLShader(const std::filesystem::path& shaderPath);

	// HLSLIファイルをプロジェクトから登録解除（vcxprojからNoneItemを削除）
	/**
	 * @brief 指定された要素の登録を解除します。
	 */
	static bool UnregisterHLSLIFile(const std::filesystem::path& shaderPath);

private:
	/**
	 * @brief ModifyVcxproj の処理を行います。
	 */
	//static bool ModifyVcxproj(const std::string& projectPath, const std::string& itemPath, VcxprojType type, bool add);
	/**
	 * @brief GetVcxprojPath に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	//static std::string GetVcxprojPath(VcxprojType type);
	/**
	 * @brief NormalizePath の処理を行います。
	 */
	//static std::string NormalizePath(const std::string& path);
	static constexpr const char* s_vcxprojPath = "CurryEngine.vcxproj";
	static std::queue<std::filesystem::path> s_pendingShaderRegistrations; // 登録待ちのシェーダーパス(実行終了後にまとめて処理)
	static std::queue<std::filesystem::path> s_pendingShaderUnregistrations; // 登録解除待ちのシェーダーパス(実行終了後にまとめて処理)
};