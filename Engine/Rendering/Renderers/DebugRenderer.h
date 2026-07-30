#pragma once
#include <wrl.h>
#include <d3d11.h>
#include "Engine/Core/Math/Vector3.h"
#include "Engine/Core/Math/Quaternion.h"
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
	 * @brief 立方体を描画します。
	 * @param center 立方体の中心位置。
	 * @param rotation 立方体の回転。
	 * @param size 立方体のサイズ（幅、高さ、奥行き）。
	 * @param color 色。
	 */
	static void DrawBox(const Vector3& center, const Quaternion& rotation, const Vector3& size, const Color& color);

	/**
	 * @brief 球を描画します。
	 * @param center 球の中心位置。
	 * @param radius 球の半径。
	 * @param color 色。
	 * @param segments 球の分割数（デフォルトは16）。分割数が多いほど滑らかになります。
	 */
	static void DrawSphere(const Vector3& center, float radius, const Color& color, int segments = 16);

	/**
	 * @brief 半球を描画します。
	 * @param center 半球の中心位置。
	 * @param direction 半球の向き（法線ベクトル）。
	 * @param radius 半球の半径。
	 * @param color 色。
	 * @param drawBottom 底面を描画するかどうか（デフォルトはtrue）。
	 * @param segments 半球の分割数（デフォルトは16）。分割数が多いほど滑らかになります。
	 */
	static void DrawHemisphere(const Vector3& center, const Vector3& direction, float radius, const Color& color, bool drawBottom = true, int segments = 16);

	/**
	 * @brief 円柱を描画します。
	 * @param start 円柱の始点。
	 * @param end 円柱の終点。
	 * @param radius 円柱の半径。
	 * @param color 色。
	 * @param drawTopBottom 上下の面を描画するかどうか（デフォルトはtrue）。
	 * @param segments 円柱の分割数（デフォルトは16）。分割数が多いほど滑らかになります。
	 */
	static void DrawCylinder(const Vector3& start, const Vector3& end, float radius, const Color& color, bool drawTopBottom = true, int segments = 16);

	/**
	 * @brief カプセルを描画します。
	 * @param start カプセルの始点。
	 * @param end カプセルの終点。
	 * @param radius カプセルの半径。
	 * @param color 色。
	 * @param segments カプセルの分割数（デフォルトは16）。分割数が多いほど滑らかになります。
	 */
	static void DrawCapsule(const Vector3& start, const Vector3& end, float radius, const Color& color, int segments = 16);

	/**
	 * @brief 平面を描画します。
	 * @param center 平面の中心位置。
	 * @param normal 平面の法線ベクトル。
	 * @param size 平面のサイズ（幅、高さ）。
	 * @param color 色。
	 */
	static void DrawPlane(const Vector3& center, const Vector3& normal, float size, const Color& color);

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
