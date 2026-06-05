#pragma once
//#include "Engine/Core/Component.h"
#include "Renderer.h"
#include <DirectXMath.h>
#include <wrl.h>
#include "Engine/Core/Misc.h"
#include "d3d11.h"

#include "Engine/Core/Color.h"

/**
 * @file
 * @brief 基本的な図形を描画するコンポーネント。
 * @details キューブ、円柱、球のいずれかを描画します。マテリアルを使用しない場合は
 *          色を指定できます。
 */

/*
 * @brief 基本的な図形を描画するコンポーネント。
 */
class PrimitiveRenderer : public Renderer
{
	C_REFLECT(PrimitiveRenderer)
public:
	/** @brief 頂点フォーマット。*/
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
	};
#ifndef USE_MATERIAL
	struct Constants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 matrialColor;
	};
#endif // !USE_MATERIAL

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

#ifndef USE_MATERIAL
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
#endif
public:
	/** @brief 図形の種類。*/
	enum class Shape { Cube, Cylinder, Sphere };
	
	/** @brief コンストラクタ。*/
	PrimitiveRenderer();
	virtual ~PrimitiveRenderer() = default;

	/*
	 * @brief 図形の種類を設定
	 */
	void SetShape(const Shape& shape);

	/*
	 * @brief キューブ生成
	 */
	void CreateCube(ID3D11Device* device);

	/*
	 * @brief 円柱生成
	 */
	void CreateCylinder(ID3D11Device* device, int segmentCount = 30);

	/*
	 * @brief 球生成
	 */
	void CreateSphere(ID3D11Device* device, int stackCount = 10, int sliceCount = 10);

	/**
	 * @brief 描画処理。
	 * @param rtx 描画コンテキスト。
	 */
	void Render(RenderContext* rtx) override;

	/**
	 * @brief デバッグ GUI の描画（インスペクタなど）。
	 */
	void DrawProperty() override;

	/**
	 * @brief AABBを計算して返す
	 * @return AABB (ワールド座標系)
	 */
	Math::BoundingBox CalculateAABB() const override;
	
	// シリアライズ
	json Serialize() const override;

	// デシリアライズ
	void Deserialize(const json& j) override;

protected:
	/*
	 * @brief 頂点バッファ、インデックスバッファの生成
	 * @param device D3D11 デバイス
	 * @param vertices 頂点配列
	 * @param vertexCount 頂点数
	 * @param indices インデックス配列
	 * @param indexCount インデックス数
	 * @details 頂点バッファ、インデックスバッファを生成します。
	 */
	void CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertexCount,
		uint32_t* indices, size_t indexCount);

public:
	Shape shape = Shape::Cube; //!< 図形の種類

	Color color;
};