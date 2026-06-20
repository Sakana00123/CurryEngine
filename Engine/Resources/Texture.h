#pragma once

#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
using namespace DirectX;

#include <wrl.h>
using namespace Microsoft::WRL;

#include "Resource.h"
#include <string>
#include <map>
using namespace std;


#include "Engine/Core/Misc.h"

/**
 * @file
 * @brief テクスチャ読み込み/生成/参照を扱うユーティリティと `Texture` 資源クラス。
 * @details WIC/DDS ローダを用いたファイル読み込み、ダミーテクスチャ生成、解放処理、
 *          および `ID3D11ShaderResourceView` の取得インタフェースを提供します。
 */

/**
 * @brief 読み込み済み SRV の簡易キャッシュ。
 * @details 同一ファイルパスに対して SRV を再利用する目的のマップです。
 */
static map<wstring, ComPtr<ID3D11ShaderResourceView>> resources;

/**
 * @brief テクスチャファイルを読み込みます。
 * @param device D3D11 デバイス。
 * @param filename ファイルパス（UTF-16）。
 * @param shader_resource_view 生成された SRV の出力先。
 * @param texture2d_desc 読み込んだテクスチャの記述子（出力）。
 * @return 成功時 S_OK、失敗時は DirectX のエラーコード。
 */
HRESULT LoadTextureFromFile(ID3D11Device* device, const wchar_t* filename,
	ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc);

/**
 * @brief すべてのテクスチャ（キャッシュ）を解放します。
 */
void ReleaseAllTextures();

/**
 * @brief 指定色のダミーテクスチャを生成します。
 * @param device D3D11 デバイス。
 * @param shader_resource_view 生成された SRV の出力先。
 * @param value RGBA を 0xAABBGGRR 形式で指定。
 * @param dimension 正方テクスチャの一辺のピクセル数。
 * @return 成功時 S_OK、失敗時は DirectX のエラーコード。
 */
HRESULT MakeDummyTexture(ID3D11Device* device, ID3D11ShaderResourceView** shader_resource_view,
	DWORD value/*0xAABBGGRR*/, UINT dimension);

/**
 * @brief メモリ上の画像からテクスチャを生成します。
 * @param device D3D11 デバイス。
 * @param data 画像データ先頭ポインタ。
 * @param size バイトサイズ。
 * @param shaderResourceView 生成された SRV の出力先。
 * @return 成功時 S_OK、失敗時は DirectX のエラーコード。
 */
HRESULT LoadTextureFromMemory(ID3D11Device* device, const void* data, size_t size, ID3D11ShaderResourceView** shaderResourceView);


/**
 * @brief GPUリソースの基底クラス。
 * @details GPUリソースの共通インタフェースを定義します。将来的に、テクスチャ以外の資源もここから派生させることができます。
 */
class GpuResource
{
public:
	virtual ~GpuResource() = default;
	// GPUリソースの解放
	//virtual void Release() = 0;
};

// テクスチャのセマンティクス（用途）を表す列挙型
enum class TextureSemantic
{
	Default,    // 通常のカラー、メイン画像
	Depth,      // 深度 (ShadowMap, Z-Buffer)
	Normal,     // 法線 (G-Buffer, NormalMap)
	Position,   // 座標 (G-Buffer)
	Velocity,   // モーションベクトル
	Metallic,   // メタリック
	Roughness,  // ラフネス
	AmbientOcclusion, // AO
	Custom      // ユーザー定義
};

// テクスチャの次元を表す列挙型
enum class TextureDimension
{
	Unknown,
	Texture1D, // 1D テクスチャ (ラインテクスチャ、データバッファ)
	Texture2D, // 2D テクスチャ (一般的なテクスチャ、レンダーターゲット、深度ステンシルバッファ)
	Texture3D, // 3D テクスチャ (ボリュームテクスチャ、3D データ)
	TextureCube, // キューブマップ (環境マッピング、スカイボックス)
	Texture2DArray, // 2Dテクスチャ配列 (カスケードシャドウ、インスタンシングなど)
	TextureCubeArray, // キューブマップ配列
	// その他の特殊な次元タイプを追加可能
};

//enum class TextureFilterMode
//{
//	Point = 0, // 補間なし (ドット絵、データバッファ)
//	Bilinear, // 線形補間 (一般的なテクスチャ)
//	Anisotropic // 異方性フィルタリング (斜めからのテクスチャ表示に有効)
//};
//
//enum class TextureWrapMode
//{
//	Repeat = 0, // 繰り返し (デフォルト)
//	Clamp,      // クランプ (端の色を伸ばす)
//	Mirror,     // 反転繰り返し (ミラーリング)
//	Border      // ボーダー (指定色で境界を塗りつぶす)
//};

/**
 * @brief テクスチャのインタフェースクラス。
 * @details GPUリソースとしてのテクスチャの共通インタフェースを定義します。
 */
class Texture : public GpuResource
{
public:
	Texture() = default;
	virtual ~Texture() = default;
	
	/**
	 * @brief シェーダリソースビューを取得します。
	 * @param semantic テクスチャのセマンティクス（用途）。継承先で、セマンティクスに応じた異なる SRV を返すことができます。
	 * @return テクスチャの SRV。セマンティクスに応じた SRV を返すことができますが、現状は単一の SRV を返します。
	 */
	virtual ID3D11ShaderResourceView* GetSRV(TextureSemantic semantic) const = 0;

	/**
	 * @brief シェーダリソースビューのアドレスを取得します（API 呼び出し用）。
	 * @param semantic テクスチャのセマンティクス（用途）。継承先で、セマンティクスに応じた異なる SRV アドレスを返すことができます。
	 * @return テクスチャの SRV のアドレス。セマンティクスに応じた SRV アドレスを返すことができますが、現状は単一の SRV アドレスを返します。
	 */
	virtual ID3D11ShaderResourceView** GetSRVAddress(TextureSemantic semantic) = 0;

	/**
	 * @brief テクスチャの次元を取得します。
	 * @return テクスチャの次元。継承先で、テクスチャの種類に応じた次元を返すことができます。
	 */
	virtual TextureDimension GetDimension() const = 0;
};

/**
 * @brief 2D テクスチャのインタフェースクラス。
 * @details `Texture` を継承し、2D テクスチャ特有のインタフェースを定義します。
 */
class Texture2D : public Texture
{
public:
	Texture2D() = default;
	virtual ~Texture2D() = default;
	/** @brief 2D テクスチャ記述子を取得します。*/
	virtual const D3D11_TEXTURE2D_DESC& GetDesc() const = 0;
	/** @brief テクスチャの次元を取得します。*/
	TextureDimension GetDimension() const override { return TextureDimension::Texture2D; }
};

/**
 * @brief 生の SRV と記述子を持つ 2D テクスチャクラス。
 * @details 外部で生成された SRV と記述子を直接保持し、提供するためのクラスです。
 *          ファイルからの読み込みやダミーテクスチャ生成は行わず、単純に SRV と記述子を管理します。
 */
class RawTexture2D : public Texture2D
{
public:
	/** @brief コンストラクタ。SRV と記述子を直接受け取ります。*/
	RawTexture2D(ID3D11ShaderResourceView* srv, const D3D11_TEXTURE2D_DESC& desc)
		: m_Srv(srv), m_Desc(desc) {
	}
	RawTexture2D(const RawTexture2D& other)
		: m_Srv(other.m_Srv), m_Desc(other.m_Desc) {
	}
	RawTexture2D(RawTexture2D&& other) noexcept
		: m_Srv(std::move(other.m_Srv)), m_Desc(other.m_Desc) {
	}

	RawTexture2D& operator=(const RawTexture2D&) = delete;
	/** @brief デストラクタ。SRV は ComPtr で管理されているため、自動的に解放されます。*/
	virtual ~RawTexture2D() = default;

	/** @brief SRV を取得します。*/
	ID3D11ShaderResourceView* GetSRV(TextureSemantic semantic = TextureSemantic::Default) const override { return m_Srv.Get(); }
	/** @brief SRV のアドレスを取得します（API 呼び出し用）。*/
	ID3D11ShaderResourceView** GetSRVAddress(TextureSemantic semantic = TextureSemantic::Default) override { return m_Srv.GetAddressOf(); }

	const D3D11_TEXTURE2D_DESC& GetDesc() const override { return m_Desc; }

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_Srv;
	D3D11_TEXTURE2D_DESC m_Desc;
};


/**
 * @brief ファイルから読み込んだテクスチャを表すクラス。
 * @details `Resource` と `Texture` を継承し、ファイルパスからの読み込み、ダミーテクスチャ生成、解放処理、
 *          および SRV と記述子の管理を行います。
 */
class AssetTexture : public Texture2D, public Resource
{
public:
	/** @brief 既定コンストラクタ。*/
	AssetTexture() = default;

	/** @brief デストラクタ。内部 `Release()` を呼びます。*/
	virtual ~AssetTexture() noexcept override { Release(); }

	/** @brief パスからテクスチャを読み込みます。*/
	bool LoadFromFile(const std::string& filePath) override;
	/** @brief ワイド文字パス版の読み込み。*/
	bool Load(ID3D11Device* device, const std::wstring& filePath);

	/** @brief SRV と記述子を設定します。*/
	void SetSRV(ID3D11ShaderResourceView* srv, const D3D11_TEXTURE2D_DESC& desc) {
		m_Srv = srv;
		m_Desc = desc;
	}

	/**
	 * @brief ダミーテクスチャを生成します。
	 * @param device D3D11 デバイス。
	 * @param value RGBA を 0xAABBGGRR 形式で指定。
	 * @param dimension 正方テクスチャの一辺のピクセル数。
	 */
	bool MakeDummy(ID3D11Device* device, DWORD value = 0xFFFFFFFF, UINT dimension = 16);
	/** @brief SRV と記述子を解放します。*/
	void Release();

	/** @brief SRV を取得します。*/
	ID3D11ShaderResourceView* GetSRV(TextureSemantic semantic = TextureSemantic::Default) const override { return m_Srv.Get(); }
	/** @brief SRV のアドレスを取得します（API 呼び出し用）。*/
	ID3D11ShaderResourceView** GetSRVAddress(TextureSemantic semantic = TextureSemantic::Default) override { return m_Srv.GetAddressOf(); }
	/** @brief 2D テクスチャ記述子を取得します。*/
	const D3D11_TEXTURE2D_DESC& GetDesc() const override { return m_Desc; }

private:
	/** @brief シェーダリソースビュー。*/
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_Srv = nullptr;
	/** @brief テクスチャ記述子。*/
	D3D11_TEXTURE2D_DESC m_Desc{};
};