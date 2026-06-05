#pragma once
#include <DirectXMath.h>
#include <wrl.h>
#include "Engine/Core/Misc.h"
#include "d3d11.h"

class GeometricPrimitive
{
public:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
	};
	struct Constants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 matrial_color;
	};

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;

public:
	GeometricPrimitive(ID3D11Device* device);
	virtual ~GeometricPrimitive() = default;

	//—§•û‘Ì¶¬
	void CreateCube(ID3D11Device* device);

	//‰~’Œ¶¬
	void CreateCylinder(ID3D11Device* device, int segmentCount = 30);

	/// <summary>
	/// ‹…¶¬
	/// </summary>
	/// <param name="device"></param>
	/// <param name="stackCount">‰¡ü‚Ì•ªŠ„”</param>
	/// <param name="sliceCount">cü‚Ì•ªŠ„”</param>
	void CreateSphere(ID3D11Device* device, int stackCount = 10, int sliceCount = 10);

	void Render(ID3D11DeviceContext* immediate_context,
		const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& material_color);
	
protected:
	void CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertex_count,
		uint32_t* indices, size_t index_count);
private:
	bool isCreated = false;
};