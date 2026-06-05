#pragma once
#include <DirectXMath.h>
#include <wrl.h>
#include "Engine/Core/Misc.h"
#include "d3d11.h"
#include <string>
#include <vector>

class StaticMesh
{
public:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texcoord;
	};
	struct Constants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 matrial_color;
	};
	struct SubSet
	{
		std::wstring usemtl;
		uint32_t index_start{ 0 };//start position of index buffer
		uint32_t index_count{ 0 };//number of vertices (indices)
	};
	std::vector<SubSet> subsets;

	struct Material
	{
		std::wstring name;
		DirectX::XMFLOAT4 Ka{ 0.2f, 0.2f, 0.2f, 1.0f };
		DirectX::XMFLOAT4 Kd{ 0.8f, 0.8f, 0.8f, 1.0f };
		DirectX::XMFLOAT4 Ks{ 1.0f,1.0f,1.0f,1.0f };
		std::wstring texture_filenames[2];
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_views[2];
	};
	std::vector<Material> materials;

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;		


public:
	StaticMesh(ID3D11Device* device, const wchar_t* obj_filename);
	virtual ~StaticMesh() = default;

	void Render(ID3D11DeviceContext* immediate_context,
		const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& material_color);

protected:
	void CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertex_count,
		uint32_t* indices, size_t index_count);

};