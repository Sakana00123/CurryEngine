#include "pch.h"
#include "FullScreenQuad.h"
#include "Engine/Core/Misc.h"
#include "Engine/Resources/Shader.h"
#include "Engine/Rendering/Material.h"
#include "Engine/Core/EnginePaths.h"
#include "RenderContext.h"

FullScreenQuad::FullScreenQuad(ID3D11Device* device)
{
	std::string dir = EnginePaths::ShadersDataDir;
	CreateVertexShaderFromCSO(device, (dir + "fullscreen_quad_vs.cso").c_str(), embedded_vertex_shader.ReleaseAndGetAddressOf(),
		nullptr, nullptr, 0);
	CreatePixelShaderFromCSO(device, (dir + "fullscreen_quad_ps.cso").c_str(), embedded_pixel_shader.ReleaseAndGetAddressOf());
}

void FullScreenQuad::Draw(ID3D11DeviceContext* immediate_context,
	ID3D11ShaderResourceView** shader_resource_view, uint32_t startSlot, uint32_t numViews,
	ID3D11PixelShader* replaced_pixel_shader)
{
	immediate_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	immediate_context->IASetInputLayout(nullptr);

	immediate_context->VSSetShader(embedded_vertex_shader.Get(), 0, 0);
	replaced_pixel_shader ? immediate_context->PSSetShader(replaced_pixel_shader, 0, 0) :
		immediate_context->PSSetShader(embedded_pixel_shader.Get(), 0, 0);

	immediate_context->PSSetShaderResources(startSlot, numViews, shader_resource_view);

	immediate_context->Draw(4, 0);
}

void FullScreenQuad::Render(RenderContext* rtx, Material* material)
{
	auto immediateContext = rtx->immediateContext;

	immediateContext->PSSetShader(embedded_pixel_shader.Get(), nullptr, 0);

	// マテリアルの適用(シェーダーの設定など)
	Shader::SetNullShader(immediateContext); // まずはシェーダーをすべてアンバインドしてからマテリアルを適用する
	material->Apply(rtx);

	// フルスクリーンクアッドの描画に必要な設定を上書きする
	immediateContext->IASetInputLayout(nullptr);
	immediateContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 頂点シェーダーはフルスクリーンクアッド用のものを使用する
	immediateContext->VSSetShader(embedded_vertex_shader.Get(), nullptr, 0);

	// フルスクリーンクアッドの描画
	immediateContext->Draw(4, 0);
}