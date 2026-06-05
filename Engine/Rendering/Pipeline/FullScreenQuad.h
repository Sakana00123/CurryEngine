#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <cstdint>

class Material;
struct RenderContext;

class FullScreenQuad
{
public:
	FullScreenQuad(ID3D11Device* device);
	virtual ~FullScreenQuad() = default;

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> embedded_vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> embedded_pixel_shader;

public:
	void Draw(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView** shader_resource_view,
		uint32_t startSlot, uint32_t numViews, ID3D11PixelShader* replaced_pixel_shader = nullptr);


	void Render(RenderContext* rtx, Material* material);
};
