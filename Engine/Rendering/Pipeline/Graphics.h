#pragma once
#include <d3d11_1.h>
#include <wrl.h>
#include <dxgi1_6.h>

#include <tracy/TracyD3D11.hpp>

#include <memory>
#include <sstream>
#include "Engine/Core/Misc.h"

#include "Engine/Rendering/Buffers/FrameBuffer.h"
#include "Engine/Resources/Shader.h"
#include "Engine/Resources/Texture.h"

#include "FullScreenQuad.h"
#include "RenderState.h"
#include "RenderContext.h"

class Graphics
{
public:
	static void Initialize(HWND hwnd, bool fullScreenMode);
	static void Finalize();

	static ID3D11Device* GetDevice() { return device.Get(); }

	static ID3D11DeviceContext* GetDeviceContext() { return immediate_context.Get(); }

	static RenderState* GetRenderState() { return renderState.get(); }

	// デフォルトのレンダーターゲットを取得
	static ID3D11RenderTargetView* GetDefaultRenderTargetView() { return m_buckBufferRTV.Get(); }
	// デフォルトの深度ステンシルビューを取得
	static ID3D11DepthStencilView* GetDefaultDepthStencilView() { return m_defaultDSV.Get(); }
	// デフォルトのビューポートを取得
	static D3D11_VIEWPORT GetDefaultViewport();

	static void GetScreenSize(float& x, float& y) { x = static_cast<float>(m_screenSize.cx), y = static_cast<float>(m_screenSize.cy); }

	static void Clear(float r, float g, float b, float a);

	static void Present(bool vsync = false);

	static void BindDDSTexture();

	static void StylizeWindow(BOOL fullscreen);
	static void OnSizeChanged(UINT64 width, UINT64 height);

	static void CreateSwapChain(IDXGIFactory6* dxgi_factory6);

	static HWND GetHwnd() { return m_hwnd; }

	static BOOL GetFullScreenMode() { return m_fullscreenMode; }

	static IDXGIAdapter3* GetAdapter() { return adapter.Get(); }


	//スクリーン範囲設定（スクリーン座標）
	static void SetScreenRect(float left, float top, float right, float bottom) { min = { left, top }, max = { right, bottom }; }
	//スクリーン範囲取得（スクリーン座標）
	static void GetScreenRect(float& left, float& top, float& right, float& bottom) { left = min.x, top = min.y, right = max.x, bottom = max.y; }
	
	static void SetView(DirectX::XMFLOAT4X4 view) { m_View = view; }

	static DirectX::XMFLOAT4X4 GetView() { return m_View; }

	static void SetProjection(DirectX::XMFLOAT4X4 projection) { m_Projection = projection; }

	static DirectX::XMFLOAT4X4 GetProjection() { return m_Projection; }


	// 共有リソースの管理
	static void SetSharedResource(const std::string& key, void* resource) { sharedResources[key] = resource; }

	// 共有リソースの取得
	static void* GetSharedResource(const std::string& key) { return sharedResources[key]; }

	// 共有リソースの参照を取得
	static std::unordered_map<std::string, void*> GetSharedResources() { return sharedResources; }

	// 共有リソースのリセット
	static void ResetSharedResources() { sharedResources.clear(); }

#ifdef TRACY_ENABLE
	// TracyのD3D11コンテキストを取得
	static tracy::D3D11Ctx* GetTracyD3D11Ctx() { return m_tracyD3D11Ctx; }
#endif // TRACY_ENABLE

private:
	static inline HWND m_hwnd;

	static inline SIZE m_screenSize;
	static inline Microsoft::WRL::ComPtr<IDXGIAdapter3> adapter;
	
	static inline BOOL m_fullscreenMode{ FALSE };
	static inline BOOL tearingSupported{ FALSE };
	static inline RECT windowedRect;

	static inline Microsoft::WRL::ComPtr<IDXGIFactory6> dxgi_factory6;
	static inline Microsoft::WRL::ComPtr<ID3D11Device> device;
	static inline Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediate_context;

	static inline Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain;
	static inline Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_buckBufferRTV;
	static inline Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_defaultDSV;

	static inline std::unique_ptr<RenderState> renderState;

	static inline DirectX::XMFLOAT2	min{ 0,0 };
	static inline DirectX::XMFLOAT2	max{ 1280,720 };

	static inline DirectX::XMFLOAT4X4 m_View;
	static inline DirectX::XMFLOAT4X4 m_Projection;

	static inline std::unordered_map<std::string, void*> sharedResources;

#ifdef TRACY_ENABLE
	static inline tracy::D3D11Ctx* m_tracyD3D11Ctx{ nullptr };
#endif // TRACY_ENABLE

public:
	//フルスクリーンクアッド
	static inline std::unique_ptr<FullScreenQuad> fullScreenQuad;
	//DDSテクスチャ
	static inline Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_views[8];
};
