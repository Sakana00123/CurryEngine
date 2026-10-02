#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <cstdint>

/** @brief FrameBuffer を表すクラスです。 */
class FrameBuffer
{
public:
    /**
     * @brief FrameBuffer を構築します。
     */
    FrameBuffer(ID3D11Device* device, uint32_t width, uint32_t height, bool withDepthStencil = true);
    /**
     * @brief FrameBuffer を破棄します。
     */
    virtual ~FrameBuffer() = default;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_stencil_view;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_views[2];
    D3D11_VIEWPORT viewport;

    void Clear(ID3D11DeviceContext* immediate_context,
        float r = 0, float g = 0, float b = 0, float a = 1, float depth = 1);
    /**
     * @brief Activate の処理を行います。
     */
    void Activate(ID3D11DeviceContext* immediate_context);
    /**
     * @brief Deactivate の処理を行います。
     */
    void Deactivate(ID3D11DeviceContext* immediate_context);

private:
    UINT viewportCount{ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE };
    D3D11_VIEWPORT cachedViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cached_render_target_view;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cached_depth_stencil_view;
};