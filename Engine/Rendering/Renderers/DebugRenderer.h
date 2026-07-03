#pragma once
#include <wrl.h>
#include <d3d11.h>
#include "Engine/Core/Math/Vector3.h"
#include "Engine/Core/Color.h"
struct RenderContext;

class DebugRenderer
{
public:
	/** @brief デバッグ描画の初期化処理。*/
	static void Initialize();
	/** @brief デバッグ描画の終了処理。*/
	static void Finalize();
	/** @brief デバッグ描画の実行。*/
	static void DrawAll(RenderContext* rtx, D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	/**
	 * @brief 頂点を追加します。
	 * @param position 頂点の位置。
	 * @param color 頂点の色。
	 */
	static void AddVertex(const Vector3& position, const Color& color);

	/**
	 * @brief 線分を描画します。
	 * @param start 始点。
	 * @param end 終点。
	 * @param color 色。
	 */
	static void DrawLine(const Vector3& start, const Vector3& end, const Color& color);
	
	/**
	 * @brief グリッドを描画します。
	 * @param center グリッドの中心位置。
	 * @param size グリッドの全体サイズ。(例: 10なら10mx10mのグリッドで、中心から端までの距離が5mになります)
	 * @param divisions グリッドの分割数（例: 10なら10x10のグリッド）。
	 * @param color 色。
	 */
	static void DrawGrid(const Vector3& center, float size, int divisions, const Color& color);

private:
	// 内部で使用するリソースや状態をここに追加
	//static const uint32_t VertexCapacity = 3 * 1024; // 描画する頂点の最大数
	static const uint32_t VertexCapacity = 3 * 32768; // 描画する頂点の最大数

	struct Vertex
	{
		Vector3 position; // 頂点の位置
		Color color;      // 頂点の色
	};

	struct ConstantBufferData
	{
		DirectX::XMFLOAT4X4 viewProjection; // ビュー射影行列
	};

	static inline std::vector<Vertex> vertices; // 描画する頂点のリスト

	static inline Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer; // 頂点バッファ
	static inline Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout; // 入力レイアウト
	static inline Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader; // 頂点シェーダー
	static inline Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader; // ピクセルシェーダー
	static inline Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer; // 定数バッファ
};