#include "pch.h"
#include "Shader.h"
#include "Engine/Core/Misc.h"
#include <fstream>
#include <sstream>
#include <filesystem>

#include "Engine/Editor/Console.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Rendering/Material.h"
#include "Engine/Core/EnginePaths.h"

#include <memory>
using namespace std;

void LoadShaderFile(const char* filePath, std::vector<BYTE>& data)
{
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		std::string errorMsg = "Failed to open shader file: " + std::string(filePath) + "\n";
		errorMsg += "Current directory: " + std::filesystem::current_path().string();
		_ASSERT_EXPR_A(false, errorMsg.c_str());
	}

	size_t size = file.tellg();
	file.seekg(0, std::ios::beg);

	data.resize(size);
	file.read(reinterpret_cast<char*>(data.data()), size);
	// file はスコープを抜けると自動的に close されるため安全
}

HRESULT CreateVertexShaderFromCSO(ID3D11Device* device, const char* cso_name, ID3D11VertexShader** vertex_shader,
	ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements)
{
	std::vector<BYTE> data;
	LoadShaderFile(cso_name, data);

	HRESULT hr{ S_OK };
	hr = device->CreateVertexShader(data.data(), data.size(), nullptr, vertex_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	if (input_layout)
	{
		hr = device->CreateInputLayout(input_element_desc, num_elements,
			data.data(), data.size(), input_layout);
	}
	return hr;
}

HRESULT CreatePixelShaderFromCSO(ID3D11Device* device, const char* cso_name, ID3D11PixelShader** pixel_shader)
{
	std::vector<BYTE> data;
	LoadShaderFile(cso_name, data);

	HRESULT hr{ S_OK };
	hr = device->CreatePixelShader(data.data(), data.size(), nullptr, pixel_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	return hr;
}

HRESULT CreateGeometryShaderFromCSO(ID3D11Device* device, const char* cso_name, ID3D11GeometryShader** geometryShader)
{
	std::vector<BYTE> data;
	LoadShaderFile(cso_name, data);

	HRESULT hr{ S_OK };
	hr = device->CreateGeometryShader(data.data(), data.size(), nullptr, geometryShader);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	return hr;
}

HRESULT CreateComputeShaderFromCSO(ID3D11Device* device, const char* cso_name, ID3D11ComputeShader** computeShader)
{
	std::vector<BYTE> data;
	LoadShaderFile(cso_name, data);

	HRESULT hr{ S_OK };
	hr = device->CreateComputeShader(data.data(), data.size(), nullptr, computeShader);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	return hr;
}

bool Shader::LoadFromFile(const std::string& path)
{
	std::filesystem::path fsPath(path);
#ifndef _DEBUG
	// リリースビルドではCSOファイルのみ読み込む
	if (fsPath.extension() != ".cso") 
	{
		// CSOファイルのパスを生成
		fsPath = std::filesystem::path(EnginePaths::ShadersDataDir) / (fsPath.stem().string() + ".cso");
	}
#endif // !_DEBUG

	// パスを保存
	_path = fsPath.string();

	Microsoft::WRL::ComPtr<ID3DBlob> blob;

	if (fsPath.extension() == ".cso") 
	{
		// CSOファイルの読み込み
		D3DReadFileToBlob(fsPath.wstring().c_str(), blob.ReleaseAndGetAddressOf());
		/*m_CsoPath = path;
		LoadShaderFile(path.c_str(), m_CSOData);*/
	}
	else
	{
		// HLSLファイルのコンパイル
		if (CompileShader(_path, "main", GetShaderTarget(_path), blob.ReleaseAndGetAddressOf()))
		{
			// CSOファイルの保存
			auto outPath = std::filesystem::path(EnginePaths::ShadersDataDir) / (fsPath.stem().string() + ".cso");
			D3DWriteBlobToFile(blob.Get(), outPath.wstring().c_str(), TRUE);
		}
		else
		{
			return false;
		}
	}

	// シェーダーリフレクションの取得
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pReflection;
	HRESULT hr = D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&pReflection));

	// シェーダーの情報を取得
	D3D11_SHADER_DESC shaderDesc{};
	pReflection->GetDesc(&shaderDesc);

	// シェーダーの生成
	bool result = ReflectAndCreateShader(Graphics::GetDevice(), _path, "main", GetShaderTarget(_path),
		pReflection.Get(), &shaderDesc,
		blob->GetBufferPointer(), blob->GetBufferSize());

	return result;
}

bool Shader::LoadFromFile(ID3D11Device* device, const std::string& filePath, const std::string& entryPoint, const std::string& shaderTarget)
{
	// HLSLファイルのコンパイル
	ID3DBlob* blob;
	CompileShader(filePath, entryPoint, shaderTarget, &blob);

	// シェーダーリフレクションの取得
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pReflection;
	HRESULT hr = D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&pReflection));

	// シェーダーの情報を取得
	D3D11_SHADER_DESC shaderDesc{};
	pReflection->GetDesc(&shaderDesc);

	// シェーダーの生成
	return ReflectAndCreateShader(device, filePath, entryPoint, shaderTarget, pReflection.Get(), &shaderDesc,
		blob->GetBufferPointer(), blob->GetBufferSize());
}

bool Shader::Reload(/*ID3D11Device* device*/)
{
	bool result = true;

#if 0
	// 全てのシェーダーステージをリロード
	for (auto& shaderDesc : m_Descs)
	{
		if (!shaderDesc.filePath.empty())
		{
			result &= LoadFromFile(shaderDesc.filePath);
		}
	}
#else
	// パスが設定されていればリロード
	if (!_path.empty())
	{
		std::filesystem::path fsPath(_path);
#ifdef _DEBUG
		if (fsPath.extension() == ".cso")
		{
			// CSOファイルのパスをHLSLファイルに変換
			fsPath = std::filesystem::path("./Shader") / fsPath.filename().replace_extension(".hlsl");
		}
#endif // _DEBUG
		result = LoadFromFile(fsPath.string());
	}
#endif // 0

	// 変更フラグを立てる
	if (result) m_IsDirty = true;

	// オーナーのマテリアルをリロード
	if (Material* owner = GetOwner()) 
	{
		owner->Reload(Graphics::GetDevice());
	}

	return result;
}

void Shader::ClearConstantBufferLayouts()
{
	m_ReflectionData.constantBufferLayouts.clear();
}

const ShaderReflectionData::ConstantBufferLayout* Shader::GetConstantBufferLayout(const std::string& name) const
{
	for (const ShaderReflectionData::ConstantBufferLayout& cbufferLayout : m_ReflectionData.constantBufferLayouts)
	{
		if (cbufferLayout.name == name) 
		{
			return &cbufferLayout;
		}
	}
	return nullptr;
}

const std::vector<ShaderReflectionData::ConstantBufferLayout>& Shader::GetAllConstantBufferLayouts() const
{
	return m_ReflectionData.constantBufferLayouts;
}

void Shader::SetNullShader(ID3D11DeviceContext* immediateContext)
{
	immediateContext->PSSetShader(nullptr, nullptr, 0);
	immediateContext->VSSetShader(nullptr, nullptr, 0);
	immediateContext->CSSetShader(nullptr, nullptr, 0);
	immediateContext->GSSetShader(nullptr, nullptr, 0);
	immediateContext->HSSetShader(nullptr, nullptr, 0);
	immediateContext->DSSetShader(nullptr, nullptr, 0);
	immediateContext->IASetInputLayout(nullptr);
}

bool Shader::CompileShader(const std::string& filePath, const std::string& entryPoint, const std::string& shaderTarget, ID3DBlob** outBlob)
{
	UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // DEBUG

	// HLSLファイルのコンパイル
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(
		std::wstring(filePath.begin(), filePath.end()).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint.c_str(), shaderTarget.c_str(), compileFlags, 0, outBlob, &errorBlob
	);

	// コンパイルエラー
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
			LOG_ERROR(static_cast<char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
		}
		return false;
	}
	if (errorBlob) errorBlob->Release();

	return true;
}

bool Shader::ReflectAndCreateShader(ID3D11Device* device, const std::string& filePath, const std::string& entryPoint, const std::string& shaderTarget, 
	ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc, const void* pShaderBytecode, size_t BytecodeLength)
{
	HRESULT hr = S_OK;
	// 定数バッファの情報をクリア
	ClearConstantBufferLayouts();

	// 定数バッファの情報を取得
	ReflectConstantBufferLayouts(pReflection, shaderDesc);

	// テクスチャとサンプラのバインド情報を取得
	ReflectTextureAndSamplerBindings(pReflection, shaderDesc);


	// シェーダー情報の保存
	m_Desc = { filePath, entryPoint, shaderTarget };

	// シェーダーオブジェクトの生成
	if (!CreateShader(device, pReflection, shaderDesc, pShaderBytecode, BytecodeLength))
	{
		LOG_ERROR("CreateShader faild!");
		return false;
	}

	//LOG_INFO("Shader Loaded");
	//LOG_INFO(std::format("Shader Loaded: {} (EntryPoint: {}, Target: {})", filePath, entryPoint, shaderTarget));
	return hr == S_OK;
}

void Shader::ReflectInputLayoutDesc(ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc, std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayoutDesc)
{
	for (UINT i = 0; i < shaderDesc->InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		pReflection->GetInputParameterDesc(i, &paramDesc);

		D3D11_INPUT_ELEMENT_DESC element{};
		element.SemanticName = paramDesc.SemanticName;
		element.SemanticIndex = paramDesc.SemanticIndex;
		element.InputSlot = 0;
		element.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		element.InstanceDataStepRate = 0;

		// コンポーネント数を調べる
		UINT compCount = 0;
		for (UINT bit = 0; bit < 4; ++bit)
		{
			if (paramDesc.Mask & (1 << bit))
				compCount++;
		}

		// フォーマットを決定する
		switch (compCount)
		{
		case 1:
			switch (paramDesc.ComponentType)
			{
			case D3D_REGISTER_COMPONENT_FLOAT32: element.Format = DXGI_FORMAT_R32_FLOAT; break;
			case D3D_REGISTER_COMPONENT_SINT32:  element.Format = DXGI_FORMAT_R32_SINT;  break;
			case D3D_REGISTER_COMPONENT_UINT32:  element.Format = DXGI_FORMAT_R32_UINT;  break;
			}
			break;

		case 2:
			switch (paramDesc.ComponentType)
			{
			case D3D_REGISTER_COMPONENT_FLOAT32: element.Format = DXGI_FORMAT_R32G32_FLOAT; break;
			case D3D_REGISTER_COMPONENT_SINT32:  element.Format = DXGI_FORMAT_R32G32_SINT;  break;
			case D3D_REGISTER_COMPONENT_UINT32:  element.Format = DXGI_FORMAT_R32G32_UINT;  break;
			}
			break;

		case 3:
			switch (paramDesc.ComponentType)
			{
			case D3D_REGISTER_COMPONENT_FLOAT32: element.Format = DXGI_FORMAT_R32G32B32_FLOAT; break;
			case D3D_REGISTER_COMPONENT_SINT32:  element.Format = DXGI_FORMAT_R32G32B32_SINT;  break;
			case D3D_REGISTER_COMPONENT_UINT32:  element.Format = DXGI_FORMAT_R32G32B32_UINT;  break;
			}
			break;

		case 4:
			switch (paramDesc.ComponentType)
			{
			case D3D_REGISTER_COMPONENT_FLOAT32: element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
			case D3D_REGISTER_COMPONENT_SINT32:  element.Format = DXGI_FORMAT_R32G32B32A32_SINT;  break;
			case D3D_REGISTER_COMPONENT_UINT32:  element.Format = DXGI_FORMAT_R32G32B32A32_UINT;  break;
			}
			break;
		}

		inputLayoutDesc.push_back(element);
	}
}

void Shader::ReflectConstantBufferLayouts(ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc)
{
	for (UINT i = 0; i < shaderDesc->ConstantBuffers; ++i)
	{
		ID3D11ShaderReflectionConstantBuffer* cb = pReflection->GetConstantBufferByIndex(i);
		D3D11_SHADER_BUFFER_DESC cbDesc;
		cb->GetDesc(&cbDesc);

		// TODO: この関数内で定数バッファ以外も処理する場合はここを変更
		if (cbDesc.Type != D3D_CT_CBUFFER)
		{
			LOG_WARNING(std::format("Skipping non-constant buffer: {}", cbDesc.Name));
			continue; // 定数バッファ以外はスキップ
		}

		ShaderReflectionData::ConstantBufferLayout layout;
		layout.name = cbDesc.Name;
		layout.size = cbDesc.Size;

		// バインドスロットを調べる
		for (UINT r = 0; r < shaderDesc->BoundResources; r++)
		{
			D3D11_SHADER_INPUT_BIND_DESC bindDesc;
			pReflection->GetResourceBindingDesc(r, &bindDesc);

			if (bindDesc.Type == D3D_SIT_CBUFFER && layout.name == bindDesc.Name)
			{
				layout.slot = bindDesc.BindPoint;
			}
		}

		// 変数情報を取得
		for (UINT j = 0; j < cbDesc.Variables; ++j)
		{
			ID3D11ShaderReflectionVariable* pVar = cb->GetVariableByIndex(j);
			ShaderReflectionData::ShaderVariable var;
			
			// 変数の基本情報を取得
			// TODO: テクスチャやサンプラなどのリソース変数も処理する場合はここを変更
			D3D11_SHADER_VARIABLE_DESC varDesc;
			pVar->GetDesc(&varDesc);
			var.name = varDesc.Name;
			var.size = varDesc.Size;
			var.offset = varDesc.StartOffset;
			var.defaultValue = varDesc.DefaultValue;
			
			// 変数の型情報を取得
			pVar->GetType()->GetDesc(&var.typeDesc);

			layout.variables.push_back(var);
		}
		m_ReflectionData.constantBufferLayouts.push_back(layout);
	}
}

void Shader::ReflectTextureAndSamplerBindings(ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc)
{
	// テクスチャとサンプラのバインディング情報を取得
	for (UINT i = 0; i < shaderDesc->BoundResources; ++i)
	{
		D3D11_SHADER_INPUT_BIND_DESC bindDesc;
		pReflection->GetResourceBindingDesc(i, &bindDesc);
		if (bindDesc.Type == D3D_SIT_TEXTURE)
		{
			// テクスチャのバインド情報を保存
			ShaderReflectionData::TextureInfo texInfo;
			texInfo.name = bindDesc.Name;
			texInfo.bindPoint = bindDesc.BindPoint;
			texInfo.bindCount = bindDesc.BindCount; // 配列の場合は複数
			texInfo.dimension = bindDesc.Dimension; // D3D_SRV_DIMENSION
			m_ReflectionData.textureInfos.push_back(texInfo);
			
		}
		else if (bindDesc.Type == D3D_SIT_SAMPLER)
		{
			// サンプラのバインド情報を保存
			ShaderReflectionData::SamplerInfo samplerInfo;
			samplerInfo.name = bindDesc.Name;
			samplerInfo.bindPoint = bindDesc.BindPoint;
			samplerInfo.bindCount = bindDesc.BindCount; // 配列の場合は複数
			m_ReflectionData.samplerInfos.push_back(samplerInfo);
		}
	}
}

std::string Shader::GetShaderTarget(ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc)
{
	UINT major = D3D11_SHVER_GET_MAJOR(shaderDesc->Version);
	UINT minor = D3D11_SHVER_GET_MINOR(shaderDesc->Version);
	UINT type = D3D11_SHVER_GET_TYPE(shaderDesc->Version);

	switch (type)
	{
		case D3D11_SHVER_PIXEL_SHADER:
			return "ps_" + std::to_string(major) + "_" + std::to_string(minor);
		case D3D11_SHVER_VERTEX_SHADER:
			return "vs_" + std::to_string(major) + "_" + std::to_string(minor);
		case D3D11_SHVER_GEOMETRY_SHADER:
			return "gs_" + std::to_string(major) + "_" + std::to_string(minor);
		case D3D11_SHVER_COMPUTE_SHADER:
			return "cs_" + std::to_string(major) + "_" + std::to_string(minor);
		case D3D11_SHVER_HULL_SHADER:
			return "hs_" + std::to_string(major) + "_" + std::to_string(minor);
		case D3D11_SHVER_DOMAIN_SHADER:
			return "ds_" + std::to_string(major) + "_" + std::to_string(minor);
	default:
		break;
	}
	return "";
}

std::string Shader::GetShaderTarget(const std::string& path)
{
	std::filesystem::path fsPath(path);
	std::string stem = fsPath.stem().string(); // ファイル名（拡張子なし）

	if (stem.size() >= 2) 
	{
		std::string suffix = stem.substr(stem.size() - 2, 2); // ファイル名の末尾2文字を取得
		std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower); // 小文字に変換
		if (suffix == "vs") return "vs_5_0"; // バージョンは適宜変更
		if (suffix == "ps") return "ps_5_0";
		if (suffix == "gs") return "gs_5_0";
		if (suffix == "cs") return "cs_5_0";
		if (suffix == "hs") return "hs_5_0";
		if (suffix == "ds") return "ds_5_0";
	}
	return "";
}

Shader::Shader(ShaderType type) : Resource(), m_Type(type)
{
	
}

PixelShader::PixelShader() : Shader(ShaderType::Pixel)
{

}

bool PixelShader::CreateShader(ID3D11Device* device, ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc, const void* pShaderBytecode, size_t BytecodeLength)
{
	HRESULT hr = S_OK;
	// ピクセルシェーダーの生成
	hr = device->CreatePixelShader(pShaderBytecode, BytecodeLength, nullptr, m_PixelShader.ReleaseAndGetAddressOf());
	return hr == S_OK;
}

void PixelShader::Bind(ID3D11DeviceContext* immediateContext)
{
	if (auto ps = GetPS()) immediateContext->PSSetShader(ps, nullptr, 0);
}

VertexShader::VertexShader() : Shader(ShaderType::Vertex)
{
}

bool VertexShader::CreateShader(ID3D11Device* device, ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc, const void* pShaderBytecode, size_t BytecodeLength)
{
	HRESULT hr = S_OK;
	// 頂点シェーダーの生成
	hr = Graphics::GetDevice()->CreateVertexShader(pShaderBytecode, BytecodeLength, nullptr, m_VertexShader.ReleaseAndGetAddressOf());
	// 入力レイアウトの情報を取得
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	ReflectInputLayoutDesc(pReflection, shaderDesc, inputLayoutDesc);
	// 入力レイアウトの生成
	hr = device->CreateInputLayout(inputLayoutDesc.data(), static_cast<UINT>(inputLayoutDesc.size()),
		pShaderBytecode, BytecodeLength, m_InputLayout.ReleaseAndGetAddressOf());
	return hr == S_OK;
}

void VertexShader::Bind(ID3D11DeviceContext* immediateContext)
{
	if (auto vs = GetVS()) immediateContext->VSSetShader(vs, nullptr, 0);
	if (auto inputLayout = GetInputLayout()) immediateContext->IASetInputLayout(inputLayout);
}

ComputeShader::ComputeShader() : Shader(ShaderType::Compute)
{
}

bool ComputeShader::CreateShader(ID3D11Device* device, ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc, const void* pShaderBytecode, size_t BytecodeLength)
{
	HRESULT hr = S_OK;
	// コンピュートシェーダーの生成
	hr = device->CreateComputeShader(pShaderBytecode, BytecodeLength, nullptr, m_ComputeShader.ReleaseAndGetAddressOf());
	return hr == S_OK;
}

void ComputeShader::Bind(ID3D11DeviceContext* immediateContext)
{
	if (auto cs = GetCS()) immediateContext->CSSetShader(cs, nullptr, 0);
}

GeometryShader::GeometryShader() : Shader(ShaderType::Geometry)
{
}

bool GeometryShader::CreateShader(ID3D11Device* device, ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc, const void* pShaderBytecode, size_t BytecodeLength)
{
	HRESULT hr = S_OK;
	// ジオメトリシェーダーの生成
	hr = device->CreateGeometryShader(pShaderBytecode, BytecodeLength, nullptr, m_GeometryShader.ReleaseAndGetAddressOf());
	return hr == S_OK;
}

void GeometryShader::Bind(ID3D11DeviceContext* immediateContext)
{
	if (auto gs = GetGS()) immediateContext->GSSetShader(gs, nullptr, 0);
}

HullShader::HullShader() : Shader(ShaderType::Hull)
{
}

bool HullShader::CreateShader(ID3D11Device* device, ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc, const void* pShaderBytecode, size_t BytecodeLength)
{
	HRESULT hr = S_OK;
	// ハルシェーダーの生成
	hr = device->CreateHullShader(pShaderBytecode, BytecodeLength, nullptr, m_HullShader.ReleaseAndGetAddressOf());
	return hr == S_OK;
}

void HullShader::Bind(ID3D11DeviceContext* immediateContext)
{
	if (auto hs = GetHS()) immediateContext->HSSetShader(hs, nullptr, 0);
}

DomainShader::DomainShader() : Shader(ShaderType::Domain)
{
}

bool DomainShader::CreateShader(ID3D11Device* device, ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc, const void* pShaderBytecode, size_t BytecodeLength)
{
	HRESULT hr = S_OK;
	// ドメインシェーダーの生成
	hr = device->CreateDomainShader(pShaderBytecode, BytecodeLength, nullptr, m_DomainShader.ReleaseAndGetAddressOf());
	return hr == S_OK;
}

void DomainShader::Bind(ID3D11DeviceContext* immediateContext)
{
	if (auto ds = GetDS()) immediateContext->DSSetShader(ds, nullptr, 0);
}
