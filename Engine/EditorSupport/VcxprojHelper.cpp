#include "pch.h"
#include "VcxprojHelper.h"
#include "Engine/Editor/Console.h"
#include <fstream>
#include <sstream>
#include "VSProjectReloader.h"

std::queue<std::filesystem::path> VcxprojHelper::s_pendingShaderRegistrations;
std::queue<std::filesystem::path> VcxprojHelper::s_pendingShaderUnregistrations;

static std::string ShaderTypeString(const std::filesystem::path& path)
{
    // 拡張子を小文字で取得
	std::string filename = path.filename().string();
	std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
	// 後ろ7文字を見て判定（例: "shader.vs.hlsl" の場合は "vs.hlsl" を見る）
    if (filename.size() >= 7) {
        std::string suffix = filename.substr(filename.size() - 7);
        if (suffix == "vs.hlsl") return "Vertex";
        if (suffix == "ps.hlsl") return "Pixel";
        if (suffix == "gs.hlsl") return "Geometry";
        if (suffix == "hs.hlsl") return "Hull";
        if (suffix == "ds.hlsl") return "Domain";
        if (suffix == "cs.hlsl") return "Compute";
	}
	return "Unknown";
}

void VcxprojHelper::EnqueueShaderRegistration(const std::filesystem::path& shaderPath)
{
	// 登録解除待ちキューに同じパスが存在するかチェック
	bool foundInUnregistrations = false;
	std::queue<std::filesystem::path> tempQueue;
	while (!s_pendingShaderUnregistrations.empty()) {
		if (s_pendingShaderUnregistrations.front() == shaderPath) {
			// なにもしない（相殺させるため、登録解除待ちから削除して登録も行わない）
			foundInUnregistrations = true;
		}
		else {
			tempQueue.push(s_pendingShaderUnregistrations.front()); // 一時キューに保持
		}
		s_pendingShaderUnregistrations.pop();
	}
	s_pendingShaderUnregistrations = std::move(tempQueue); // 登録解除待ちキューを更新
    if (!foundInUnregistrations)
    {
        s_pendingShaderRegistrations.push(shaderPath);
    }
}

void VcxprojHelper::EnqueueShaderUnregistration(const std::filesystem::path& shaderPath)
{
    // 登録待ちキューに同じパスが存在するかチェック
	bool foundInRegistrations = false;
    std::queue<std::filesystem::path> tempQueue;
    while (!s_pendingShaderRegistrations.empty()) {
        if (s_pendingShaderRegistrations.front() == shaderPath) {
			// なにもしない（相殺させるため、登録待ちから削除して登録解除も行わない）
			foundInRegistrations = true;
        } else {
			tempQueue.push(s_pendingShaderRegistrations.front()); // 一時キューに保持
        }
        s_pendingShaderRegistrations.pop();
    }
	s_pendingShaderRegistrations = std::move(tempQueue); // 登録待ちキューを更新
	
    if (!foundInRegistrations)
    {
        s_pendingShaderUnregistrations.push(shaderPath);
    }
}

bool VcxprojHelper::IsShaderRegistered(const std::filesystem::path& shaderPath)
{
    auto vcxprojPath = std::filesystem::path(s_vcxprojPath);
    std::ifstream ifs(vcxprojPath);
    if (!ifs) return false;
    std::string content((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());
    ifs.close();
    std::string includeStr = shaderPath.string();
    std::ranges::replace(includeStr, '/', '\\');
    return content.find("Include=\"" + includeStr + "\"") != std::string::npos;
}

bool VcxprojHelper::IsShaderRegistrationPending(const std::filesystem::path& shaderPath)
{
    // 登録待ちキューに同じパスが存在するかチェック
    std::queue<std::filesystem::path> tempQueue = s_pendingShaderRegistrations; // キューをコピーして検索
    while (!tempQueue.empty()) {
        if (tempQueue.front() == shaderPath) {
            return true; // 登録待ちに存在
        }
        tempQueue.pop();
    }
    return false; // 登録待ちに存在しない
}

void VcxprojHelper::ProcessPendingShaderRegistrations()
{
    while (!s_pendingShaderRegistrations.empty()) {
        const auto& shaderPath = s_pendingShaderRegistrations.front();
		// hlsl と hlsli を分けて登録する
        if (shaderPath.extension() == ".hlsl") {
            if (!RegisterHLSLShader(shaderPath)) {
                LOG_ERROR("Failed to register shader: " + shaderPath.string());
            }
        }
        else if (shaderPath.extension() == ".hlsli") {
            if (!RegisterHLSLIFile(shaderPath)) {
                LOG_ERROR("Failed to register HLSLI file: " + shaderPath.string());
            }
        }
        else {
            LOG_ERROR("Unsupported shader file type: " + shaderPath.string());
		}
        s_pendingShaderRegistrations.pop();
    }
}

void VcxprojHelper::ProcessPendingShaderUnregistrations()
{
    // 登録解除の処理は未実装（必要に応じて RegisterHLSLShader と同様の手順で実装する）
    while (!s_pendingShaderUnregistrations.empty()) {
        const auto& shaderPath = s_pendingShaderUnregistrations.front();
		// hlsl と hlsli を分けて登録解除する
        if (shaderPath.extension() == ".hlsl") {
            UnregisterHLSLShader(shaderPath);
        }
        else if (shaderPath.extension() == ".hlsli") {
            UnregisterHLSLIFile(shaderPath);
        }
        else {
            LOG_ERROR("Unsupported shader file type for unregistration: " + shaderPath.string());
        }
        s_pendingShaderUnregistrations.pop();
    }
}

bool VcxprojHelper::RegisterHLSLShader(const std::filesystem::path& shaderPath)
{
    // --- 1. ファイル読み込み ---
	auto vcxprojPath = std::filesystem::path(s_vcxprojPath);
    std::ifstream ifs(vcxprojPath);
    if (!ifs) return false;
    std::string content((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());
    ifs.close();

    // --- 2. .vcxproj からの相対パスに変換 ---
    // Include属性は .vcxproj からの相対パスで書く
    //auto relativePath = std::filesystem::relative(shaderPath, vcxprojPath.parent_path());
    // Windows の区切り文字に統一
    std::string includeStr = shaderPath.string();
    std::ranges::replace(includeStr, '/', '\\');

    // --- 3. 重複チェック ---
    if (content.find("Include=\"" + includeStr + "\"") != std::string::npos) {
        return true; // 既に登録済み
    }

    // --- 4. 挿入XMLを組み立て ---
    std::string shaderType = ShaderTypeString(shaderPath);
    if (shaderType == "Unknown") {
        LOG_ERROR("Unrecognized shader type for file: " + shaderPath.string());
        return false;
	}
    std::string entry =
        "    <FxCompile Include=\"" + includeStr + "\">\n"
        "      <ShaderType Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">"
        + shaderType + "</ShaderType>\n"
        "      <ShaderType Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\">"
        + shaderType + "</ShaderType>\n"
        "    </FxCompile>\n";

    // --- 5. 既存の FxCompile ItemGroup の末尾に挿入 ---
    // "</FxCompile>" が最後に出現する位置の直後に差し込む
    auto insertPos = content.rfind("</FxCompile>");
    if (insertPos == std::string::npos) {
        // FxCompile エントリがまだ1つもない場合: </ItemGroup> の前に挿入
        // (FxCompile 専用 ItemGroup がある前提。なければ別途作成が必要)
        insertPos = content.rfind("</ItemGroup>");
        if (insertPos == std::string::npos) return false;
        content.insert(insertPos, entry);
    }
    else {
        // "</FxCompile>" の行末 (\n含む) の直後に挿入
        insertPos = content.find('\n', insertPos);
        if (insertPos == std::string::npos) return false;
        content.insert(insertPos + 1, entry);
    }

    // --- 6. 書き戻し ---
    std::ofstream ofs(vcxprojPath);
    if (!ofs) return false;
    ofs << content;
	ofs.close(); // flush と close を明示的に呼ぶことで、書き込み失敗を検出しやすくする

	//// --- 7. Visual Studio にリロードを促す ---
 //   if (!VSProjectReloader::ReloadProject(vcxprojPath.wstring())) {
 //       LOG_ERROR("Failed to reload project in Visual Studio: " + vcxprojPath.string());
 //       return false;
	//}

	return true;
}

bool VcxprojHelper::RegisterHLSLIFile(const std::filesystem::path& shaderPath)
{
    // --- 1. ファイル読み込み ---
    auto vcxprojPath = std::filesystem::path(s_vcxprojPath);
    std::ifstream ifs(vcxprojPath);
    if (!ifs) return false;
    std::string content((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());
    ifs.close();
    // --- 2. .vcxproj からの相対パスに変換 ---
    //auto relativePath = std::filesystem::relative(shaderPath, vcxprojPath.parent_path());
    std::string includeStr = shaderPath.string();
    std::ranges::replace(includeStr, '/', '\\');
    // --- 3. 重複チェック ---
    if (content.find("Include=\"" + includeStr + "\"") != std::string::npos) {
        return true; // 既に登録済み
    }
    // --- 4. 挿入XMLを組み立て ---
    std::string entry =
        "    <None Include=\"" + includeStr + "\" />\n";

    // --- 5. .hlsli の ItemGroup 末尾を特定して挿入 ---
    // ".hlsli" を含む <None Include> が最後に出現する行の直後に差し込む
    auto insertPos = std::string::npos;
    size_t searchFrom = 0;
    while (true) {
        auto pos = content.find(".hlsli\"", searchFrom);
        if (pos == std::string::npos) break;
        // その行末 (\n) を記録して次を探す
        insertPos = content.find('\n', pos);
        if (insertPos == std::string::npos) break;
        searchFrom = insertPos + 1;
    }

    if (insertPos == std::string::npos) {
        // .hlsli エントリが1つもない場合のフォールバック:
        // <None Include= が最後に出現する行末の直後
        searchFrom = 0;
        while (true) {
            auto pos = content.find("<None Include=", searchFrom);
            if (pos == std::string::npos) break;
            insertPos = content.find('\n', pos);
            if (insertPos == std::string::npos) break;
            searchFrom = insertPos + 1;
        }
    }

    if (insertPos == std::string::npos) return false;

    content.insert(insertPos + 1, entry);

    // --- 6. 書き戻し ---
    std::ofstream ofs(vcxprojPath);
    if (!ofs) return false;
    ofs << content;
	ofs.close();

 //   // --- 7. Visual Studio にリロードを促す ---
 //   if (!VSProjectReloader::ReloadProject(vcxprojPath.wstring())) {
 //       LOG_ERROR("Failed to reload project in Visual Studio: " + vcxprojPath.string());
 //       return false;
	//}

	return true;
}

bool VcxprojHelper::UnregisterHLSLShader(const std::filesystem::path& shaderPath)
{
	// RegisterHLSLShader と同様の手順で .vcxproj を読み込み、該当する <FxCompile Include="..."> エントリを削除して書き戻す
	// --- 1. ファイル読み込み ---
	auto vcxprojPath = std::filesystem::path(s_vcxprojPath);
	std::ifstream ifs(vcxprojPath);
	if (!ifs) return false;
	std::string content((std::istreambuf_iterator<char>(ifs)),
		std::istreambuf_iterator<char>());
	ifs.close();
	// --- 2. .vcxproj からの相対パスに変換 ---
	std::string includeStr = shaderPath.string();
	std::ranges::replace(includeStr, '/', '\\');
	// --- 3. エントリの位置を特定 ---
	std::string entryStart = "    <FxCompile Include=\"" + includeStr + "\"";
	size_t pos = content.find(entryStart);
	if (pos == std::string::npos) {
		return true; // エントリが見つからない場合は既に登録解除されているとみなす
	}
	// エントリの終了位置を見つける（次の </FxCompile> まで）
	size_t entryEnd = content.find("</FxCompile>", pos);
	if (entryEnd == std::string::npos) {
		return false; // 不正な形式
	}
	entryEnd = content.find('\n', entryEnd);
	if (entryEnd == std::string::npos) {
		return false; // 不正な形式
	}
	// --- 4. エントリを削除 ---
	content.erase(pos, entryEnd - pos + 1);

	// --- 5. 書き戻し ---
	std::ofstream ofs(vcxprojPath);
	if (!ofs) return false;
	ofs << content;
	ofs.close();

    return false;
}

bool VcxprojHelper::UnregisterHLSLIFile(const std::filesystem::path& shaderPath)
{
    // RegisterHLSLIFile と同様の手順で .vcxproj を読み込み、該当する <None Include="..."> エントリを削除して書き戻す
	// --- 1. ファイル読み込み ---
	auto vcxprojPath = std::filesystem::path(s_vcxprojPath);
	std::ifstream ifs(vcxprojPath);
	if (!ifs) return false;
	std::string content((std::istreambuf_iterator<char>(ifs)),
		std::istreambuf_iterator<char>());
	ifs.close();
	// --- 2. .vcxproj からの相対パスに変換 ---
	std::string includeStr = shaderPath.string();
	std::ranges::replace(includeStr, '/', '\\');
	// --- 3. エントリの位置を特定 ---
	std::string entryStart = "    <None Include=\"" + includeStr + "\"";
	size_t pos = content.find(entryStart);
	if (pos == std::string::npos) {
		return true; // エントリが見つからない場合は既に登録解除されているとみなす
	}
	// エントリの終了位置を見つける（行末まで）
	size_t entryEnd = content.find('\n', pos);
	if (entryEnd == std::string::npos) {
		return false; // 不正な形式
	}
	// --- 4. エントリを削除 ---
	content.erase(pos, entryEnd - pos + 1);

	// --- 5. 書き戻し ---
	std::ofstream ofs(vcxprojPath);
	if (!ofs) return false;
	ofs << content;
	ofs.close();

    return false;
}