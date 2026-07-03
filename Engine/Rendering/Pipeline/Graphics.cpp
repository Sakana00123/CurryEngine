#include "pch.h"
#include "Graphics.h"
void AcquireHighPerformanceAdapter(IDXGIFactory6* dxgi_factory6, IDXGIAdapter3** dxgi_adapter3)
{
	HRESULT hr{ S_OK };

	Microsoft::WRL::ComPtr<IDXGIAdapter3> enumerated_adapter;
	for (UINT adapter_index = 0; DXGI_ERROR_NOT_FOUND != dxgi_factory6->EnumAdapterByGpuPreference(adapter_index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(enumerated_adapter.ReleaseAndGetAddressOf())); ++adapter_index)
	{
		DXGI_ADAPTER_DESC1 adapter_desc;
		hr = enumerated_adapter->GetDesc1(&adapter_desc);
		_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

		if (adapter_desc.VendorId == 0x1002/*AMD*/ || adapter_desc.VendorId == 0x10DE/*NVIDIA*/)
		{
			OutputDebugStringW((std::wstring(adapter_desc.Description) + L" has been selected.\n").c_str());
			OutputDebugStringA(std::string("\tVendorId:" + std::to_string(adapter_desc.VendorId) + '\n').c_str());
			OutputDebugStringA(std::string("\tDeviceId:" + std::to_string(adapter_desc.DeviceId) + '\n').c_str());
			OutputDebugStringA(std::string("\tSubSysId:" + std::to_string(adapter_desc.SubSysId) + '\n').c_str());
			OutputDebugStringA(std::string("\tRevision:" + std::to_string(adapter_desc.Revision) + '\n').c_str());
			OutputDebugStringA(std::string("\tDedicatedVideoMemory:" + std::to_string(adapter_desc.DedicatedVideoMemory) + '\n').c_str());
			OutputDebugStringA(std::string("\tDedicatedSystemMemory:" + std::to_string(adapter_desc.DedicatedSystemMemory) + '\n').c_str());
			OutputDebugStringA(std::string("\tSharedSystemMemory:" + std::to_string(adapter_desc.SharedSystemMemory) + '\n').c_str());
			OutputDebugStringA(std::string("\tAdapterLuid.HighPart:" + std::to_string(adapter_desc.AdapterLuid.HighPart) + '\n').c_str());
			OutputDebugStringA(std::string("\tAdapterLuid.LowPart:" + std::to_string(adapter_desc.AdapterLuid.LowPart) + '\n').c_str());
			OutputDebugStringA(std::string("\tFlags:" + std::to_string(adapter_desc.Flags) + '\n').c_str());
			break;
		}
	}
	*dxgi_adapter3 = enumerated_adapter.Detach();
}

void Graphics::Initialize(HWND hwnd, bool fullScreenMode)
{
	m_hwnd = hwnd;
	m_fullscreenMode = fullScreenMode;
	GetWindowRect(hwnd, &windowedRect);
	if (m_fullscreenMode)
	{
		StylizeWindow(m_fullscreenMode);
	}

	RECT client_rect;
	GetClientRect(hwnd, &client_rect);
	m_screenSize.cx = client_rect.right - client_rect.left;
	m_screenSize.cy = client_rect.bottom - client_rect.top;

	//デバイス、デバイスコンテキスト、スワップチェーン作成
	HRESULT hr{ S_OK };

	UINT create_factory_flags{};
#ifdef _DEBUG
	create_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hr = CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(dxgi_factory6.GetAddressOf()));
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
	AcquireHighPerformanceAdapter(dxgi_factory6.Get(), adapter.GetAddressOf());


	UINT create_device_flag{ 0 };
#ifdef _DEBUG
	create_device_flag |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL feature_levels{ D3D_FEATURE_LEVEL_11_1 };
	hr = D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, 0, create_device_flag,
		&feature_levels, 1, D3D11_SDK_VERSION, device.GetAddressOf(), NULL, immediate_context.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	CreateSwapChain(dxgi_factory6.Get());

	// レンダーステート生成
	renderState = std::make_unique<RenderState>(device.Get());

	// フルスクリーンクワッド生成
	fullScreenQuad = std::make_unique<FullScreenQuad>(device.Get());

	// DDSテクスチャロード
	//D3D11_TEXTURE2D_DESC texture2d_desc;
	//LoadTextureFromFile(device.Get(), L"./Assets/environments/sunset_jhbcentral_4k/sunset_jhbcentral_4k.dds",
	//	shader_resource_views[0].GetAddressOf(), &texture2d_desc);
	//LoadTextureFromFile(device.Get(), L"./Assets/environments/sunset_jhbcentral_4k/diffuse_iem.dds",
	//	shader_resource_views[1].GetAddressOf(), &texture2d_desc);
	//LoadTextureFromFile(device.Get(), L"./Assets/environments/specular_pmrem.dds",
	//	shader_resource_views[2].GetAddressOf(), &texture2d_desc);
	//LoadTextureFromFile(device.Get(), L"./Assets/environments/lut_ggx.dds",
	//	shader_resource_views[3].GetAddressOf(), &texture2d_desc);
}

void Graphics::Finalize()
{
	BOOL fullscreen{};

	swap_chain->GetFullscreenState(&fullscreen, 0);
	if (fullscreen)
	{
		swap_chain->SetFullscreenState(FALSE, 0);
	}
}

void Graphics::StylizeWindow(BOOL fullscreen)
{
	m_fullscreenMode = fullscreen;
	if (fullscreen)
	{
		GetWindowRect(m_hwnd, &windowedRect);

		// Modify the window style for fullscreen (remove title bar and borders)
		DWORD fullscreenStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME);
		SetWindowLongPtrA(m_hwnd, GWL_STYLE, fullscreenStyle);

		RECT fullscreenWindowRect;

		HRESULT hr{ E_FAIL };
		if (swap_chain)
		{
			Microsoft::WRL::ComPtr<IDXGIOutput> dxgiOutput;
			hr = swap_chain->GetContainingOutput(dxgiOutput.GetAddressOf());
			if (hr == S_OK)
			{
				DXGI_OUTPUT_DESC outputDesc;
				hr = dxgiOutput->GetDesc(&outputDesc);
				if (hr == S_OK)
				{
					fullscreenWindowRect = outputDesc.DesktopCoordinates;
				}
			}
		}
		if (hr != S_OK)
		{
			DEVMODE devmode = {};
			devmode.dmSize = sizeof(DEVMODE);
			EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);

			fullscreenWindowRect = {
				devmode.dmPosition.x,
				devmode.dmPosition.y,
				devmode.dmPosition.x + static_cast<LONG>(devmode.dmPelsWidth),
				devmode.dmPosition.y + static_cast<LONG>(devmode.dmPelsHeight)
			};
		}
		SetWindowPos(
			m_hwnd,
			NULL,
			fullscreenWindowRect.left,
			fullscreenWindowRect.top,
			fullscreenWindowRect.right,
			fullscreenWindowRect.bottom,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(m_hwnd, SW_MAXIMIZE);
	}
	else
	{
		// Windowed mode settings
		DEVMODE devmode = {};
		devmode.dmSize = sizeof(DEVMODE);
		EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);

		//RECT windowedRect;
		//windowedRect.left = (devmode.dmPelsWidth - framebuffer_dimensions.cx) / 2; // Center the window
		//windowedRect.top = (devmode.dmPelsHeight - framebuffer_dimensions.cy) / 2;
		//windowedRect.right = windowedRect.left + framebuffer_dimensions.cx;
		//windowedRect.bottom = windowedRect.top + framebuffer_dimensions.cy;

		// Set window mode style (disable maximize button and resizing)
		DWORD windowedStyle = WS_OVERLAPPEDWINDOW /*^ WS_MAXIMIZEBOX ^ WS_THICKFRAME*/ | WS_VISIBLE;
		AdjustWindowRectEx(&windowedRect, windowedStyle, FALSE, 0); // Adjust window size

		SetWindowLongPtrA(m_hwnd, GWL_STYLE, windowedStyle);
		SetWindowPos(
			m_hwnd,
			HWND_NOTOPMOST,
			windowedRect.left,
			windowedRect.top,
			windowedRect.right - windowedRect.left,
			windowedRect.bottom - windowedRect.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(m_hwnd, SW_NORMAL);
	}
}

void Graphics::OnSizeChanged(UINT64 width, UINT64 height)
{
	HRESULT hr{ S_OK };
	if (width == 0 || height == 0) return;

	if (width != m_screenSize.cx || height != m_screenSize.cy)
	{
		// フレームバッファ解放
		//frameBuffers[0].reset();
		//frameBuffers[1].reset();

		// スクリーンサイズ更新
		m_screenSize.cx = static_cast<LONG>(width);
		m_screenSize.cy = static_cast<LONG>(height);

		// コンテキストの状態クリア
		immediate_context->Flush();
		immediate_context->ClearState();

		// スワップチェーン再作成
		CreateSwapChain(dxgi_factory6.Get());

		// フレームバッファ再作成
		//frameBuffers[0] = std::make_unique<FrameBuffer>(device.Get(), static_cast<uint32_t>(width), static_cast<uint32_t>(height));
		//frameBuffers[1] = std::make_unique<FrameBuffer>(device.Get(), static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	}
}

void Graphics::CreateSwapChain(IDXGIFactory6* dxgi_factory6)
{
	HRESULT hr{ S_OK };

	if (swap_chain)
	{
		// スワップチェーンが既に存在する場合、リサイズ処理を行う

		// バックバッファと深度ステンシルビューのバインド解除
		ID3D11RenderTargetView* null_render_target_views[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
		immediate_context->OMSetRenderTargets(_countof(null_render_target_views), null_render_target_views, nullptr);

		ID3D11ShaderResourceView* null_shader_resource_views[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
		immediate_context->PSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);
		immediate_context->VSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);

		// ビューの解放
		m_buckBufferRTV.Reset();
		m_defaultDSV.Reset();
#if 1
		immediate_context->Flush();
		immediate_context->ClearState();
#endif

		// スワップチェーンの情報取得
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swap_chain->GetDesc(&swapChainDesc);
		
		// バッファのリサイズ
		hr = swap_chain->ResizeBuffers(swapChainDesc.BufferCount, m_screenSize.cx, m_screenSize.cy, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);
		_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

		// GetBufferおよびCreateRenderTargetViewで使用するため、バックバッファを取得
		Microsoft::WRL::ComPtr<ID3D11Texture2D> render_target_buffer;
		hr = swap_chain->GetBuffer(0, IID_PPV_ARGS(render_target_buffer.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
		D3D11_TEXTURE2D_DESC texture2d_desc;
		render_target_buffer->GetDesc(&texture2d_desc);
		// バックバッファ用レンダーターゲットビューの再作成
		hr = device->CreateRenderTargetView(render_target_buffer.Get(), NULL, m_buckBufferRTV.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
	}
	else
	{
		BOOL allow_tearing = FALSE;
		if (SUCCEEDED(hr))
		{
			hr = dxgi_factory6->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(allow_tearing));
		}
		tearingSupported = SUCCEEDED(hr) && allow_tearing;

		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1{};
		swap_chain_desc1.Width = m_screenSize.cx;
		swap_chain_desc1.Height = m_screenSize.cy;
		swap_chain_desc1.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swap_chain_desc1.Stereo = FALSE;
		swap_chain_desc1.SampleDesc.Count = 1;
		swap_chain_desc1.SampleDesc.Quality = 0;
		swap_chain_desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_desc1.BufferCount = 2;
		swap_chain_desc1.Scaling = DXGI_SCALING_STRETCH;
		swap_chain_desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swap_chain_desc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swap_chain_desc1.Flags = tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		hr = dxgi_factory6->CreateSwapChainForHwnd(device.Get(), m_hwnd, &swap_chain_desc1, NULL, NULL, swap_chain.ReleaseAndGetAddressOf());
#if 0
		swap_chain_desc1.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
#endif
		_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

		hr = dxgi_factory6->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);
		_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

		Microsoft::WRL::ComPtr<ID3D11Texture2D> render_target_buffer;
		hr = swap_chain->GetBuffer(0, IID_PPV_ARGS(render_target_buffer.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
		hr = device->CreateRenderTargetView(render_target_buffer.Get(), NULL, m_buckBufferRTV.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
	}

	// 深度ステンシルビューの再作成
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_stencil_buffer{};
	D3D11_TEXTURE2D_DESC texture2d_desc{};
	texture2d_desc.Width = m_screenSize.cx;
	texture2d_desc.Height = m_screenSize.cy;
	texture2d_desc.MipLevels = 1;
	texture2d_desc.ArraySize = 1;
	texture2d_desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texture2d_desc.SampleDesc.Count = 1;
	texture2d_desc.SampleDesc.Quality = 0;
	texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
	texture2d_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texture2d_desc.CPUAccessFlags = 0;
	texture2d_desc.MiscFlags = 0;
	hr = device->CreateTexture2D(&texture2d_desc, NULL, depth_stencil_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
	depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Texture2D.MipSlice = 0;
	hr = device->CreateDepthStencilView(depth_stencil_buffer.Get(), &depth_stencil_view_desc, m_defaultDSV.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	D3D11_VIEWPORT viewport = GetDefaultViewport();
	immediate_context->RSSetViewports(1, &viewport);
}

D3D11_VIEWPORT Graphics::GetDefaultViewport()
{
	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(m_screenSize.cx);
	viewport.Height = static_cast<float>(m_screenSize.cy);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	return viewport;
}

void Graphics::Clear(float r, float g, float b, float a)
{
	ID3D11RenderTargetView* null_render_target_views[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
	immediate_context->OMSetRenderTargets(_countof(null_render_target_views), null_render_target_views, 0);
	ID3D11ShaderResourceView* null_shader_resource_views[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	immediate_context->VSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);
	immediate_context->PSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);

	FLOAT color[]{ r, g, b, a };
	immediate_context->ClearRenderTargetView(m_buckBufferRTV.Get(), color);
	immediate_context->ClearDepthStencilView(m_defaultDSV.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	immediate_context->OMSetRenderTargets(1, m_buckBufferRTV.GetAddressOf(), m_defaultDSV.Get());
}

void Graphics::Present(bool vsync)
{
	//sync_intervalに1をセットすると描画間隔が固定フレームレートで動作するようになる
	UINT sync_interval{ vsync ? 1U : 0U };
	UINT flags = (tearingSupported && !m_fullscreenMode && !vsync) ? DXGI_PRESENT_ALLOW_TEARING : 0;
	HRESULT hr = swap_chain->Present(sync_interval, flags);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void Graphics::BindDDSTexture()
{
	//immediate_context->PSSetShaderResources(32, 1, shader_resource_views[0].GetAddressOf());
	//immediate_context->PSSetShaderResources(33, 1, shader_resource_views[1].GetAddressOf());
	//immediate_context->PSSetShaderResources(34, 1, shader_resource_views[2].GetAddressOf());
	//immediate_context->PSSetShaderResources(35, 1, shader_resource_views[3].GetAddressOf());
}