#pragma once
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>

class Sprite
{
public:
	Sprite(ID3D11Device* device);
	Sprite(ID3D11Device* device, const wchar_t* filename);
	~Sprite();

	void Render(ID3D11DeviceContext* immediate_context,
		float dx, float dy,
		float dw, float dh,
		DirectX::XMFLOAT4 color = { 1,1,1,1 },
		float angle = 0//degree
	) {
		Render(immediate_context, dx, dy, dw, dh, color, angle, 0, 0, (float)texture2d_desc.Width, static_cast<float>(texture2d_desc.Height));
	}

	void Render(ID3D11DeviceContext* immediate_context,
		float dx, float dy,
		float dw, float dh,
		DirectX::XMFLOAT4 color,
		float angle,
		float sx, float sy,
		float sw, float sh
	);

	void Render(ID3D11DeviceContext* immediate_context,
		float dx, float dy,
		float dw, float dh,
		DirectX::XMFLOAT2 pivot,
		DirectX::XMFLOAT4 color = { 1,1,1,1 }
	) {
		Render(immediate_context, dx, dy, dw, dh, pivot, color, 0.f, 0.f, (float)texture2d_desc.Width, static_cast<float>(texture2d_desc.Height));
	}

	void Render(ID3D11DeviceContext* immediate_context,
		float dx, float dy,
		float dw, float dh,
		DirectX::XMFLOAT2 pivot,
		DirectX::XMFLOAT4 color,
		float sx, float sy,
		float sw, float sh
	);


	void Render(ID3D11DeviceContext* immediate_context,
		float dx, float dy,
		float dw, float dh,
		DirectX::XMFLOAT4 color,
		float angle,
		float sx, float sy,
		float sw, float sh,
		float scaleX, float scaleY,
		float pivotX, float pivotY);

	void TextOut(ID3D11DeviceContext* immediate_context, std::string s,
		float x, float y, float w, float h, DirectX::XMFLOAT4 color);

	float GetTextureWidth() { return (float)texture2d_desc.Width; }
	float GetTextureHeight() { return (float)texture2d_desc.Height; }

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
};