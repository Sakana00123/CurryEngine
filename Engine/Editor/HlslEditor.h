#pragma once

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI
#include <fstream>
#include <sstream>
#include <string>
#include <d3d11.h>
#include <wrl.h>
#include <functional>
#include "Engine/Resources/Shader.h"

class HlslEditor
{
	static inline bool isOpen = false;
	static inline ShaderType m_Type;
	static inline std::function<ID3D11PixelShader**()> getPs;
	static inline std::function<ID3D11VertexShader**()> getVs;
	static inline std::function<ID3D11GeometryShader**()> getGs;
	static inline std::function<ID3D11ComputeShader**()> getCs;
	static inline std::weak_ptr<Shader> editingShader;

	static inline const char* m_FilePath;

	static inline std::string shaderSource;
	static inline char shaderBuffer[65536]; //64KBまで
public:
	static void SetEditShader(const char* filePath, std::shared_ptr<Shader> shader) {
		m_FilePath = filePath;
		editingShader = shader;
		isOpen = true;
		LoadShaderSource(filePath);
	}
	static void SetEditShader(const char* filePath, std::function<ID3D11PixelShader**()> func) {
		m_FilePath = filePath;
		getPs = func;
		isOpen = true;
		LoadShaderSource(filePath);
	}
	static void SetEditShader(const char* filePath, std::function<ID3D11VertexShader** ()> func) {
		m_FilePath = filePath;
		getVs = func;
		isOpen = true;
		LoadShaderSource(filePath);
	}
	static void SetEditShader(const char* filePath, std::function<ID3D11GeometryShader** ()> func) {
		m_FilePath = filePath;
		getGs = func;
		isOpen = true;
		LoadShaderSource(filePath);
	}
	static void SetEditShader(const char* filePath, std::function<ID3D11ComputeShader** ()> func) {
		m_FilePath = filePath;
		getCs = func;
		isOpen = true;
		LoadShaderSource(filePath);
	}

	static void DrawGUI();

	static void Reset()
	{
		getPs = nullptr;
		getVs = nullptr;
		getGs = nullptr;
		getCs = nullptr;
		m_FilePath = nullptr;
	}

	static void Show() { isOpen = true; }

private:
	//シェーダーファイル読み込み
	static void LoadShaderSource(const char* path) {
		std::ifstream ifs(path);
		std::stringstream ss;
		ss << ifs.rdbuf();
		shaderSource = ss.str();
		strncpy_s(shaderBuffer, shaderSource.c_str(), sizeof(shaderBuffer));
	}
	//シェーダーファイル保存
	static void SaveShaderSource(const char* path) {
		std::ofstream ofs(path);
		ofs << shaderBuffer;
	}

	/// <summary>
	/// シェーダーをコンパイル
	/// </summary>
	/// <param name="filePath">コンパイル対象のHLSLファイルパス</param>
	/// <param name="entryPoint">HLSLのエントリポイント関数名</param>
	/// <param name="shaderModel">コンパイルするシェーダープロファイル(example: vs_5_0, ps_5_1)</param>
	/// <param name="blobOut">コンパイル済みシェーダーバイナリ</param>
	/// <returns></returns>
	static inline bool CompileShader(
		const std::wstring& filePath,
		const std::string& entryPoint,
		const std::string& shaderModel,
		ID3DBlob** blobOut);
};