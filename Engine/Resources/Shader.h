#pragma once

#include <d3d11.h>
#include <vector>
#include <string>
#include <wrl.h>
#include "Resource.h"
#include <d3dcompiler.h>
#include <d3d11shader.h>

/**
 * @file
 * @brief シェーダの読み込み/反映/リフレクション情報を扱うヘッダ。
 * @details HLSL からの読み込み、CSO からの生成、ステージ別の設定や
 *          定数バッファレイアウトのリフレクション取得などを提供します。
 */

/**
 * @brief シェーダバイナリ（CSO など）をファイルから読み込みます。
 * @param filePath ファイルパス。
 * @param data 読み込んだバイト列の出力先。
 */
void LoadShaderFile(const char* filePath, std::vector<BYTE>& data);

/**
 * @brief CSO から頂点シェーダを生成します。
 * @param device D3D11 デバイス。
 * @param cso_name CSO ファイルパス。
 * @param vertex_shader 生成された VS の出力先。
 * @param input_layout 入力レイアウトの出力先。
 * @param input_element_desc 入力レイアウト定義。
 * @param num_elements 要素数。
 * @return 成功時 S_OK、失敗時は DirectX エラーコード。
 */
HRESULT CreateVertexShaderFromCSO(ID3D11Device* device, const char* cso_name, ID3D11VertexShader** vertex_shader,
	ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements);

/**
 * @brief CSO からピクセルシェーダを生成します。
 */
HRESULT CreatePixelShaderFromCSO(ID3D11Device* device, const char* cso_name, ID3D11PixelShader** pixel_shader);

/**
 * @brief CSO からジオメトリシェーダを生成します。
 */
HRESULT CreateGeometryShaderFromCSO(ID3D11Device* device, const char* cso_name, ID3D11GeometryShader** geometryShader);

/**
 * @brief CSO からコンピュートシェーダを生成します。
 */
HRESULT CreateComputeShaderFromCSO(ID3D11Device* device, const char* cso_name, ID3D11ComputeShader** computeShader);

/**
 * @brief シェーダの種類。
 */
enum class ShaderType
{
	Pixel,    //!< ピクセルシェーダ
	Vertex,   //!< 頂点シェーダ
	Geometry, //!< ジオメトリシェーダ
	Hull,     //!< ハルシェーダ
	Domain,   //!< ドメインシェーダ
	Compute,  //!< コンピュートシェーダ
	EnumCount,
};

/**
 * @brief シェーダステージごとの記述情報。
 */
struct ShaderStageDesc
{
	std::string filePath;   // HLSLファイル
	std::string entryPoint; // "main"
	std::string target;     // "vs_5_0", "ps_5_0" …
};

/**
 * @brief シェーダのリフレクション情報。
 */
struct ShaderReflectionData
{
	/**
	 * @brief 定数バッファ内の変数情報。
	 */
	struct ShaderVariable
	{
		std::string name;           //!< 変数名
		size_t offset;              //!< cbuffer 内オフセット（バイト）
		size_t size;                //!< サイズ（バイト）
		D3D11_SHADER_TYPE_DESC typeDesc;//!< 型情報
	};
	/**
	 * @brief 定数バッファのレイアウト情報。
	 */
	struct ConstantBufferLayout
	{
		std::string name;                  //!< 定数バッファ名
		UINT slot;                         //!< バインドスロット
		size_t size;                       //!< 総バイトサイズ
		std::vector<ShaderVariable> variables; //!< 内包する変数一覧
	};
	/**
	 * @brief テクスチャのリフレクション情報。
	 */
	struct TextureInfo
	{
		std::string name;				// !< テクスチャ名
		UINT bindPoint;					// !< バインドスロット
		UINT bindCount;					// !< バインド数
		D3D_SRV_DIMENSION dimension;	// !< テクスチャの次元（D3D_SRV_DIMENSION）
	};
	/**
	 * @brief サンプラーのリフレクション情報。
	 */
	struct SamplerInfo
	{
		std::string name;			// !< サンプラー名	
		UINT bindPoint; 			// !< バインドスロット
		UINT bindCount; 			// !< バインド数
	};

	std::vector<ConstantBufferLayout> constantBufferLayouts; // 定数バッファレイアウト一覧
	std::vector<TextureInfo> textureInfos; 			  // テクスチャバインド情報一覧
	std::vector<SamplerInfo> samplerInfos; 			  // サンプラバインド情報一覧
};

class Material; // 前方宣言

/**
 * @brief シェーダ資源管理クラス。
 * @details HLSL/CSO の読み込み、シェーダオブジェクトと入力レイアウト生成、
 *          定数バッファレイアウトの取得、描画コンテキストへのバインド等を行います。
 */
class Shader : public Resource
{
public:
	Shader(ShaderType type);
	virtual ~Shader() override = default;

	/** @brief パスからシェーダを読み込みます（実装依存、複数ステージ対応）。*/
	bool LoadFromFile(const std::string& path) override;
	/** @brief シェーダを再読み込みします。*/
	bool Reload() override;
	/**
	 * @brief 指定の HLSL をコンパイルしてステージにロードします。
	 * @param device D3D11 デバイス。
	 * @param filePath HLSL ファイルパス。
	 * @param entryPoint エントリポイント名（例: "main"）。
	 * @param shaderTarget ターゲット（例: "vs_5_0"）。
	 */
	bool LoadFromFile(ID3D11Device* device, const std::string& filePath, const std::string& entryPoint/* = main*/, const std::string& shaderTarget);

	/** @brief 既存の CBuffer レイアウトを破棄します。*/
	void ClearConstantBufferLayouts();
	/** @brief 指定名の CBuffer レイアウトを取得します。*/
	const ShaderReflectionData::ConstantBufferLayout* GetConstantBufferLayout(const std::string& name) const;
	/** @brief すべての CBuffer レイアウトを取得します。*/
	const std::vector<ShaderReflectionData::ConstantBufferLayout>& GetAllConstantBufferLayouts() const;

	/** @brief シェーダをコンテキストにバインドします。*/
	virtual void Bind(ID3D11DeviceContext* immediateContext) = 0;
	/** @brief すべてのシェーダをアンバインドします。*/
	static void SetNullShader(ID3D11DeviceContext* immediateContext);

	/** @brief シェーダステージの記述情報を取得。*/
	const ShaderStageDesc& GetDesc() const { return m_Desc; }

	/** @brief シェーダステージの記述情報を設定。*/
	void SetDesc(const ShaderStageDesc& desc) { m_Desc = desc; }

	/** @brief シェーダが変更されたかどうかを設定（ホットリロード用）。*/
	void SetDirty(bool dirty) { m_IsDirty = dirty; }

	/** @brief シェーダが変更されたかどうかを取得（ホットリロード用）。*/
	bool IsDirty() const { return m_IsDirty; }

	/** @brief シェーダのリフレクション情報を取得。*/
	const ShaderReflectionData& GetReflectionData() const { return m_ReflectionData; }

	/** @brief シェーダの種類を取得。*/
	ShaderType GetType() const { return m_Type; }

	/** @brief このシェーダを所有する `Material` を取得。*/
	Material* GetOwner() const { return m_Owner; }

	/** @brief このシェーダの所有者を設定。*/
	void SetOwner(Material* owner) { m_Owner = owner; }

protected:
	/**
	 * @brief シェーダをコンパイルします。
	 * @return 成功で true。
	 */
	bool CompileShader(const std::string& filePath, const std::string& entryPoint, const std::string& shaderTarget,	ID3DBlob** outBlob);

	/**
	 * @brief リフレクションとシェーダ生成を行います。
	 */
	bool ReflectAndCreateShader(ID3D11Device* device, const std::string& filePath, const std::string& entryPoint, const std::string& shaderTarget,
		ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc, const void* pShaderBytecode, size_t BytecodeLength);
	/** @brief 入力レイアウトのリフレクション結果を取得。*/
	void ReflectInputLayoutDesc(ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc, std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayoutDesc);
	/** @brief 定数バッファレイアウトをリフレクト。*/
	void ReflectConstantBufferLayouts(ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc);
	/** @brief テクスチャとサンプラのバインディング情報をリフレクト。*/
	void ReflectTextureAndSamplerBindings(ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc);


	/** @brief リフレクションからシェーダターゲットを取得。*/
	std::string GetShaderTarget(ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc);
	/** @brief パスからシェーダターゲットを推定。*/
	std::string GetShaderTarget(const std::string& path);

private:
	/** @brief シェーダオブジェクトを生成します（派生クラスで実装）。*/
	virtual bool CreateShader(ID3D11Device* device, ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc, const void* pShaderBytecode, size_t BytecodeLength) = 0;

protected:
	/** @brief このシェーダを使用しているマテリアル（オーナー）。*/
	Material* m_Owner = nullptr;

	/** @brief シェーダの種類。*/
	ShaderType m_Type;

	/** @brief シェーダステージの記述情報。*/
	ShaderStageDesc m_Desc;

	/** @brief シェーダが変更されたかどうか（ホットリロード用）。*/
	bool m_IsDirty = false;

	/** @brief シェーダリフレクション情報。*/
	ShaderReflectionData m_ReflectionData;
};

class PixelShader : public Shader
{
public:
	PixelShader();
	~PixelShader() override = default;

	/** @brief ピクセルシェーダを取得。*/
	ID3D11PixelShader* GetPS() { return m_PixelShader.Get(); }

	/** @brief シェーダをコンテキストにバインドします。*/
	void Bind(ID3D11DeviceContext* immediateContext) override;

protected:
	/** @brief シェーダオブジェクトを生成します。*/
	bool CreateShader(ID3D11Device* device, ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc, const void* pShaderBytecode, size_t BytecodeLength) override;

private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
};

class VertexShader : public Shader
{
public:
	VertexShader();
	~VertexShader() override = default;

	/** @brief 頂点シェーダを取得。*/
	ID3D11VertexShader* GetVS() { return m_VertexShader.Get(); }
	/** @brief 入力レイアウトを取得。*/
	ID3D11InputLayout* GetInputLayout() { return m_InputLayout.Get(); }

	/** @brief シェーダをコンテキストにバインドします。*/
	void Bind(ID3D11DeviceContext* immediateContext) override;

protected:
	/** @brief シェーダオブジェクトを生成します。*/
	bool CreateShader(ID3D11Device* device, ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc, const void* pShaderBytecode, size_t BytecodeLength) override;

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
};

class ComputeShader : public Shader
{
public:
	ComputeShader();
	~ComputeShader() override = default;

	/** @brief コンピュートシェーダを取得。*/
	ID3D11ComputeShader* GetCS() { return m_ComputeShader.Get(); }

	/** @brief シェーダをコンテキストにバインドします。*/
	void Bind(ID3D11DeviceContext* immediateContext) override;

protected:
	/** @brief シェーダオブジェクトを生成します。*/
	bool CreateShader(ID3D11Device* device, ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc, const void* pShaderBytecode, size_t BytecodeLength) override;

private:
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_ComputeShader;
};

class GeometryShader : public Shader
{
public:
	GeometryShader();
	~GeometryShader() override = default;

	/** @brief ジオメトリシェーダを取得。*/
	ID3D11GeometryShader* GetGS() { return m_GeometryShader.Get(); }

	/** @brief シェーダをコンテキストにバインドします。*/
	void Bind(ID3D11DeviceContext* immediateContext) override;

protected:
	/** @brief シェーダオブジェクトを生成します。*/
	bool CreateShader(ID3D11Device* device, ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc, const void* pShaderBytecode, size_t BytecodeLength) override;

private:
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_GeometryShader;
};

class HullShader : public Shader
{
public:
	HullShader();
	~HullShader() override = default;

	/** @brief ハルシェーダを取得。*/
	ID3D11HullShader* GetHS() { return m_HullShader.Get(); }

	/** @brief シェーダをコンテキストにバインドします。*/
	void Bind(ID3D11DeviceContext* immediateContext) override;

protected:
	/** @brief シェーダオブジェクトを生成します。*/
	bool CreateShader(ID3D11Device* device, ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc, const void* pShaderBytecode, size_t BytecodeLength) override;

private:
	Microsoft::WRL::ComPtr<ID3D11HullShader> m_HullShader;
};

class DomainShader : public Shader
{
public:
	DomainShader();
	~DomainShader() override = default;

	/** @brief ドメインシェーダを取得。*/
	ID3D11DomainShader* GetDS() { return m_DomainShader.Get(); }

	/** @brief シェーダをコンテキストにバインドします。*/
	void Bind(ID3D11DeviceContext* immediateContext) override;

protected:
	/** @brief シェーダオブジェクトを生成します。*/
	bool CreateShader(ID3D11Device* device, ID3D11ShaderReflection* pReflection, D3D11_SHADER_DESC* shaderDesc, const void* pShaderBytecode, size_t BytecodeLength) override;

private:
	Microsoft::WRL::ComPtr<ID3D11DomainShader> m_DomainShader;
};