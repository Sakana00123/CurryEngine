#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <cstdint>

class Material;
struct RenderContext;

/** @brief FullScreenQuad を表すクラスです。 */
class FullScreenQuad
{
public:
	/**
	 * @brief FullScreenQuad を構築します。
	 */
	FullScreenQuad(ID3D11Device* device);
	/**
	 * @brief FullScreenQuad を破棄します。
	 */
	virtual ~FullScreenQuad() = default;

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> embedded_vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> embedded_pixel_shader;

public:
	void Draw(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView** shader_resource_view,
		uint32_t startSlot, uint32_t numViews, ID3D11PixelShader* replaced_pixel_shader = nullptr);


	/**
	 * @brief 描画処理を行います。
	 */
	void Render(RenderContext* rtx, Material* material);
};
