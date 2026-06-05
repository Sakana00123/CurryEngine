#pragma once
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

class SpriteBatch
{
public:
	SpriteBatch(ID3D11Device* device, const wchar_t* filename, size_t max_sprites);
	~SpriteBatch();

	void Begin(ID3D11DeviceContext* immediate_context);

	void End(ID3D11DeviceContext* immediate_context);


	void Render(ID3D11DeviceContext* immediate_context,
		float dx, float dy,
		float dw, float dh,
		DirectX::XMFLOAT4 color,
		float angle//degree
	) {
		Render(immediate_context, dx, dy, dw, dh, color, angle, 0, 0, static_cast<float>(texture2d_desc.Width), static_cast<float>(texture2d_desc.Height));
	}

	void Render(ID3D11DeviceContext* immediate_context,
		float dx, float dy,
		float dw, float dh,
		DirectX::XMFLOAT4 color,
		float angle,
		float sx, float sy,
		float sw, float sh
	);

	float GetTextureWidth() const { return static_cast<float>(texture2d_desc.Width); }
	float GetTextureHeight() const { return static_cast<float>(texture2d_desc.Height); }

	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 texcoord;
	};

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;
	D3D11_TEXTURE2D_DESC texture2d_desc;
	const size_t max_vertices;
	std::vector<Vertex> vertices;
};