#pragma once
#include <filesystem>
#include <string>
#include <queue>

enum class VcxprojType
{
	Engine,
	Game,
	Editor,
};

class VcxprojHelper
{
public:
	// シェーダーの登録をキューに追加（実行終了前にまとめて処理）
	static void EnqueueShaderRegistration(const std::filesystem::path& shaderPath);

	// シェーダーの登録解除（vcxprojからエントリを削除）
	static void EnqueueShaderUnregistration(const std::filesystem::path& shaderPath);


	// シェーダーがすでにプロジェクトに登録されているかどうか
	static bool IsShaderRegistered(const std::filesystem::path& shaderPath);

	// シェーダーの登録がキューに入っているかどうか
	static bool IsShaderRegistrationPending(const std::filesystem::path& shaderPath);


	// 登録待ちのシェーダーをまとめて処理（実行終了前に呼ぶこと）
	static void ProcessPendingShaderRegistrations();
	
	// シェーダーの登録解除をキューに追加（実行終了前にまとめて処理）
	static void ProcessPendingShaderUnregistrations();

private:

	// HLSLシェーダーをプロジェクトに登録（vcxprojにCompileItemを追加）
	static bool RegisterHLSLShader(const std::filesystem::path& shaderPath);

	// HLSLIファイルをプロジェクトに登録（vcxprojにNoneItemを追加）
	static bool RegisterHLSLIFile(const std::filesystem::path& shaderPath);

	// HLSLシェーダーをプロジェクトから登録解除（vcxprojからCompileItemを削除）
	static bool UnregisterHLSLShader(const std::filesystem::path& shaderPath);

	// HLSLIファイルをプロジェクトから登録解除（vcxprojからNoneItemを削除）
	static bool UnregisterHLSLIFile(const std::filesystem::path& shaderPath);

private:
	//static bool ModifyVcxproj(const std::string& projectPath, const std::string& itemPath, VcxprojType type, bool add);
	//static std::string GetVcxprojPath(VcxprojType type);
	//static std::string NormalizePath(const std::string& path);
	static constexpr const char* s_vcxprojPath = "CurryEngine.vcxproj";
	static std::queue<std::filesystem::path> s_pendingShaderRegistrations; // 登録待ちのシェーダーパス(実行終了後にまとめて処理)
	static std::queue<std::filesystem::path> s_pendingShaderUnregistrations; // 登録解除待ちのシェーダーパス(実行終了後にまとめて処理)
};