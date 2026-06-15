#pragma once
#include "Graphic.h"
#include "Engine/Rendering/Material.h"

/**
 * @file
 * @brief 画像（スプライト）を描画する UI コンポーネント。
 * @details テクスチャとシェーダを `Material` で管理し、`RectTransform` に基づく矩形へ
 *          四角形ポリゴンの描画を行います。マスク描画（オプション）にも対応します。
 */
class Image : public Graphic
{
	C_REFLECT(Image)
	/**
	 * @brief 頂点レイアウト。
	 * @details スクリーン座標系の位置、頂点カラー、テクスチャ座標を保持します。
	 */
	struct Vertex
	{
		DirectX::XMFLOAT4 position; //!< 位置（NDC 空間に変換済みを詰めます）
		DirectX::XMFLOAT4 color;    //!< 頂点カラー
		DirectX::XMFLOAT2 texcoord; //!< テクスチャ座標（0-1）
	};
public:

	/**
	 * @brief 画像描画コンポーネントを生成します。
	 * @details 頂点バッファの初期化、ダミー/実テクスチャのセット、各種シェーダ/マテリアル設定を行います。
	 */
	Image();
	~Image() override = default;

	/**
	 * @brief 画像ソースを設定します。
	 * @param source 画像ファイルパス（ワイド文字列）。`nullptr` でダミー画像。
	 * @param reload すでに同じファイルがセットされている場合でも再読み込みするか。デフォルトは `false`。
	 * @details 指定されたファイルからテクスチャを読み込み、マテリアルの 0 番テクスチャへセットします。
	 *          `nullptr` の場合はダミー画像をセットします。
	 */
	void SetSource(const wchar_t* source, bool reload = false);

	/**
	 * @brief 画像ソースを取得します。
	 * @return 画像ファイルパス。ダミー画像の場合は `nullptr`。
	 */
	AssetTexture* GetTexture();

	/**
	 * @brief 初期化。矩形サイズをテクスチャサイズへ合わせます。
	 * @details マテリアルの 0 番テクスチャの幅・高さを参照し、`RectTransform` のサイズを設定します。
	 */
	void Initialize() override;

	/**
	 * @brief 描画処理。
	 * @param rtx 描画コンテキスト。
	 * @details `RectTransform` から算出した矩形を NDC に変換して頂点バッファへ書き戻し、
	 *          マテリアルを適用して四角形ストリップを描画します。マスクが有効な場合は
	 *          マスク用マテリアルも適用します。
	 */
	void Draw(RenderContext* rtx) override;

#ifdef USE_IMGUI
	/**
	 * @brief インスペクタ（エディタ）用のプロパティ描画。
	 * @details 画像の差し替え、色、UV、マスク画像やシェーダ編集などを行います。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

	// シリアライズ
	json Serialize() const override;

	// デシリアライズ
	void Deserialize(const json& j) override;

	/* 色をセットします。 */
	void SetColor(const Color& color) { this->color = color; }
	
	/* 色を取得します。 */
	Color GetColor() const { return color; }

	/* UVをセットします。 */
	void SetUV(const Vector2& uv) { this->uv = uv; }

	/* UVを取得します。 */
	Vector2 GetUV() const { return (Vector2)uv; }


public:
	/** @brief 乗算カラー。*/
	C_PROPERTY()
	Color color{ 1,1,1,1 };
public:
	/** @brief 切り出し矩形（左上座標とサイズ）。*/
	float sx, sy, sw, sh;
	/** @brief UV オフセット。*/
	C_PROPERTY()
	XMFLOAT2 uv = { 0,0 };
private:
	/*D3D11_TEXTURE2D_DESC texture2dDesc{};
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;*/

	/** @brief ベース画像用のマテリアル。*/
	Material material;
	/** @brief 四角形用の頂点バッファ。*/
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

	/** @brief マスク描画を有効化するか。*/
	C_PROPERTY()
	bool enableMask = false;

	/*Microsoft::WRL::ComPtr<ID3D11PixelShader> maskPixelShader;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> maskTexture;*/

	/** @brief マスク描画用のマテリアル（テクスチャスロット 1 を使用）。*/
	Material maskMaterial;
};