#include "pch.h"
#include "HlslEditor.h"

#include "Engine/Rendering/Pipeline/Graphics.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

bool HlslEditor::CompileShader(const std::wstring& filePath, const std::string& entryPoint, const std::string& shaderModel, ID3DBlob** blobOut)
{
	UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // DEBUG

	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(
		filePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint.c_str(), shaderModel.c_str(), compileFlags, 0, blobOut, &errorBlob
	);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		return false;
	}
	if (errorBlob) errorBlob->Release();
	return true;
}

void HlslEditor::DrawGUI()
{
#ifdef USE_IMGUI
	if (isOpen)
	{
		ImGui::Begin("HLSL Editor", &isOpen);

		if (m_FilePath)
		{
			ImGui::Text("Editing: %s", m_FilePath);

			const char* types[] = { "Pixel","Vertex","Geometry","Compute" };
			ImGui::Combo("ShaderType", reinterpret_cast<int*>(&m_Type), types, IM_ARRAYSIZE(types));

			ImGui::Separator();

			ImGui::InputTextMultiline("##source", shaderBuffer, IM_ARRAYSIZE(shaderBuffer), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 30));

			ID3D11Device* device = Graphics::GetDevice();

			if (ImGui::Button("Compile & Apply"))
			{
				SaveShaderSource(m_FilePath);

				std::string entryPoint = "main";
				std::string shaderModel;
				switch (m_Type)
				{
				case ShaderType::Pixel:
					shaderModel = "ps_5_0";
					break;
				case ShaderType::Vertex:
					shaderModel = "vs_5_0";
					break;
				case ShaderType::Geometry:
					shaderModel = "gs_5_0";
					break;
				case ShaderType::Hull:
					shaderModel = "hs_5_0";
					break;
				case ShaderType::Domain:
					shaderModel = "ds_5_0";
					break;
				case ShaderType::Compute:
					shaderModel = "cs_5_0";
					break;
				default:
					break;
				}

				if (!editingShader.expired())
				{
					editingShader.lock()->LoadFromFile(device, m_FilePath, entryPoint, shaderModel);
				}
				else
				{
					//再コンパイルして差し替え
					ID3DBlob* blob = nullptr;
					
					if (CompileShader(std::wstring(m_FilePath, m_FilePath + strlen(m_FilePath)), entryPoint, shaderModel, &blob))
					{
						switch (m_Type)
						{
						case ShaderType::Pixel:
						{
							device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, getPs());
							break;
						}
						case ShaderType::Vertex:
						{
							device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, getVs());
							break;
						}
						case ShaderType::Geometry:
						{
							device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, getGs());
							break;
						}
						case ShaderType::Compute:
						{
							device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, getCs());
							break;
						}
						default:
							break;
						}
						blob->Release();
					}
				}
			}
		}

		ImGui::End();
	}
	else
	{
		Reset();
	}
#endif // USE_IMGUI
}