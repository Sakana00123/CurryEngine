#include "pch.h"
#define NOMINMAX

#include "CascadedShadowMaps.h"

#include <array>

#include "Engine/Core/Misc.h"

//Calculate the 8 vertices of the frustum based on the provided view and projection matrices.
std::array<DirectX::XMFLOAT4, 8> ExtractFrustumCorners(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
	//Define the NDC space corners
	std::array<DirectX::XMFLOAT4, 8> frustumCorners =
	{
		DirectX::XMFLOAT4{-1.0f, -1.0f, -1.0f, 1.0f},
		DirectX::XMFLOAT4{-1.0f, -1.0f,  1.0f, 1.0f},
		DirectX::XMFLOAT4{-1.0f,  1.0f, -1.0f, 1.0f},
		DirectX::XMFLOAT4{-1.0f,  1.0f,  1.0f, 1.0f},
		DirectX::XMFLOAT4{ 1.0f, -1.0f, -1.0f, 1.0f},
		DirectX::XMFLOAT4{ 1.0f, -1.0f,  1.0f, 1.0f},
		DirectX::XMFLOAT4{ 1.0f,  1.0f, -1.0f, 1.0f},
		DirectX::XMFLOAT4{ 1.0f,  1.0f,  1.0f, 1.0f}
	};
	const DirectX::XMMATRIX inverseViewProjection = DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&view) * DirectX::XMLoadFloat4x4(&projection));
	for (std::array<DirectX::XMFLOAT4, 8>::reference frustumCorner : frustumCorners)
	{
		DirectX::XMStoreFloat4(&frustumCorner, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat4(&frustumCorner), inverseViewProjection));
	}
	return frustumCorners;
}

CascadedShadowMaps::CascadedShadowMaps(ID3D11Device* device, UINT width, UINT height, UINT cascadeCount) : cascadeCount(cascadeCount), cascadedMatrices(cascadeCount), cascadedPlaneDistances(cascadeCount + 1)
{
	HRESULT hr = S_OK;

	// Create the depth stencil buffer and view
	D3D11_TEXTURE2D_DESC texture2dDesc{};
	texture2dDesc.Width = width;
	texture2dDesc.Height = height;
	texture2dDesc.MipLevels = 1;
	texture2dDesc.ArraySize = cascadeCount;
	texture2dDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texture2dDesc.SampleDesc.Count = 1;
	texture2dDesc.SampleDesc.Quality = 0;
	texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
	texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texture2dDesc.CPUAccessFlags = 0;
	texture2dDesc.MiscFlags = 0;
	hr = device->CreateTexture2D(&texture2dDesc, 0, depthStencilBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	depthStencilViewDesc.Texture2DArray.FirstArraySlice = 0;
	depthStencilViewDesc.Texture2DArray.ArraySize = static_cast<UINT>(cascadeCount);
	depthStencilViewDesc.Texture2DArray.MipSlice = 0;
	depthStencilViewDesc.Flags = 0;
	hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	// Create the shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	shaderResourceViewDesc.Texture2DArray.ArraySize = static_cast<UINT>(cascadeCount);
	shaderResourceViewDesc.Texture2DArray.MipLevels = 1;
	shaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
	shaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
	hr = device->CreateShaderResourceView(depthStencilBuffer.Get(), &shaderResourceViewDesc, shaderResourceView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	// RawTexture2D ÇçÏê¨
	depthTexture = std::make_shared<RawTexture2D>(shaderResourceView.Get(), texture2dDesc);

	// Create the viewport
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = (sizeof(Constants) + 0x0f) & ~0x0f;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.CPUAccessFlags = 0;
	hr = device->CreateBuffer(&bufferDesc, NULL, constantBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void CascadedShadowMaps::Resize(ID3D11Device* device, UINT width, UINT height)
{
	HRESULT hr = S_OK;
	// Resize the depth stencil buffer and view
	D3D11_TEXTURE2D_DESC texture2dDesc{};
	texture2dDesc.Width = width;
	texture2dDesc.Height = height;
	texture2dDesc.MipLevels = 1;
	texture2dDesc.ArraySize = cascadeCount;
	texture2dDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texture2dDesc.SampleDesc.Count = 1;
	texture2dDesc.SampleDesc.Quality = 0;
	texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
	texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texture2dDesc.CPUAccessFlags = 0;
	texture2dDesc.MiscFlags = 0;
	hr = device->CreateTexture2D(&texture2dDesc, 0, depthStencilBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	depthStencilViewDesc.Texture2DArray.FirstArraySlice = 0;
	depthStencilViewDesc.Texture2DArray.ArraySize = static_cast<UINT>(cascadeCount);
	depthStencilViewDesc.Texture2DArray.MipSlice = 0;
	depthStencilViewDesc.Flags = 0;
	hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	// Create the shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	shaderResourceViewDesc.Texture2DArray.ArraySize = static_cast<UINT>(cascadeCount);
	shaderResourceViewDesc.Texture2DArray.MipLevels = 1;
	shaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
	shaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
	hr = device->CreateShaderResourceView(depthStencilBuffer.Get(), &shaderResourceViewDesc, shaderResourceView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	// RawTexture2D ÇçÏê¨
	depthTexture = std::make_shared<RawTexture2D>(shaderResourceView.Get(), texture2dDesc);

	// Update the viewport
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
}

void CascadedShadowMaps::Activate(ID3D11DeviceContext* immediateContext,
	const DirectX::XMFLOAT4X4& cameraView,
	const DirectX::XMFLOAT4X4& cameraProjection,
	const DirectX::XMFLOAT4& lightDirection,
	float criticalDepthValue,
	UINT slot)
{
	//immediateContext->RSGetViewports(&viewportCount, cachedViewports);
	//immediateContext->OMGetRenderTargets(1, cachedRenderTargetView.ReleaseAndGetAddressOf(), cachedDepthStencilView.ReleaseAndGetAddressOf());

	// near / far value from perspective projection matrix
	float m33 = cameraProjection._33;
	float m43 = cameraProjection._43;
	float zn = -m43 / m33;
	float zf = (m33 * zn) / (m33 - 1);
	zf = criticalDepthValue > 0 ? (std::min)(zf, criticalDepthValue) : zf;

	// calculates split plane distances in view space
	for (size_t cascadeIndex = 0; cascadeIndex < cascadeCount; ++cascadeIndex)
	{
		float idc = cascadeIndex / static_cast<float>(cascadeCount);
		float logarithmicSplitScheme = zn * pow(zf / zn, idc);
		float uniformSplitScheme = zn + (zf - zn) * idc;
		cascadedPlaneDistances.at(cascadeIndex) = logarithmicSplitScheme * splitSchemeWeight + uniformSplitScheme * (1 - splitSchemeWeight);
	}
	// make sure border values are accurate
	cascadedPlaneDistances.at(0) = zn;
	cascadedPlaneDistances.at(cascadeCount) = zf;

	for (size_t cascadeIndex = 0; cascadeIndex < cascadeCount; ++cascadeIndex)
	{
		float nearPlane = fitToCascade ? cascadedPlaneDistances.at(cascadeIndex) : zn;
		float farPlane = cascadedPlaneDistances.at(cascadeIndex + 1);

		DirectX::XMFLOAT4X4 cascadedProjection = cameraProjection;
		cascadedProjection._33 = farPlane / (farPlane - nearPlane);
		cascadedProjection._43 = -nearPlane * farPlane / (farPlane - nearPlane);

		std::array<DirectX::XMFLOAT4, 8> corners = ExtractFrustumCorners(cameraView, cascadedProjection);

		DirectX::XMFLOAT4 center = { 0,0,0,1 };
		for (DirectX::XMFLOAT4 corner : corners)
		{
			center.x += corner.x;
			center.y += corner.y;
			center.z += corner.z;
		}
		center.x /= corners.size();
		center.y /= corners.size();
		center.z /= corners.size();

		// åıÇÃï˚å¸Ç™ê^è„Ç‚ê^â∫Ç…ãﬂÇ¢èÍçáÇÕUpVectorÇêÿÇËë÷Ç¶ÇÈ
		DirectX::XMVECTOR upVector = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		if (std::abs(lightDirection.x) < 0.001f && std::abs(lightDirection.z) < 0.001f)
		{
			upVector = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		}

		DirectX::XMMATRIX V;
		V = DirectX::XMMatrixLookAtLH(
			{ center.x - lightDirection.x, center.y - lightDirection.y, center.z - lightDirection.z, 1.0f },
			{ center.x, center.y, center.z, 1.0f },
			upVector);

		float minX = (std::numeric_limits<float>::max)();
		float maxX = (std::numeric_limits<float>::lowest)();
		float minY = (std::numeric_limits<float>::max)();
		float maxY = (std::numeric_limits<float>::lowest)();
		float minZ = (std::numeric_limits<float>::max)();
		float maxZ = (std::numeric_limits<float>::lowest)();
		for (DirectX::XMFLOAT4 corner : corners)
		{
			DirectX::XMStoreFloat4(&corner, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat4(&corner), V));
			minX = (std::min)(minX, corner.x);
			maxX = (std::max)(maxX, corner.x);
			minY = (std::min)(minY, corner.y);
			maxY = (std::max)(maxY, corner.y);
			minZ = (std::min)(minZ, corner.z);
			maxZ = (std::max)(maxZ, corner.z);
		}

#if 1
		zMult = (std::max<float>)(1.0f, zMult);
		if (minZ < 0)
		{
			minZ *= zMult;
		}
		else
		{
			minZ /= zMult;
		}
		if (maxZ < 0)
		{
			maxZ /= zMult;
		}
		else
		{
			maxZ *= zMult;
		}
#endif // 1

		DirectX::XMMATRIX P = DirectX::XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);
		DirectX::XMStoreFloat4x4(&cascadedMatrices.at(cascadeIndex), V * P);
	}

	Constants data{};
	data.cascadedMatrices[0] = cascadedMatrices.at(0);
	data.cascadedMatrices[1] = cascadedMatrices.at(1);
	data.cascadedMatrices[2] = cascadedMatrices.at(2);
	data.cascadedMatrices[3] = cascadedMatrices.at(3);

	data.cascadedPlaneDistances[0] = cascadedPlaneDistances.at(1);
	data.cascadedPlaneDistances[1] = cascadedPlaneDistances.at(2);
	data.cascadedPlaneDistances[2] = cascadedPlaneDistances.at(3);
	data.cascadedPlaneDistances[3] = cascadedPlaneDistances.at(4);

	immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
	immediateContext->VSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf());
	immediateContext->PSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> nullRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> nullDepthStencilView;
	immediateContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1, 0);
	immediateContext->OMSetRenderTargets(1, nullRenderTargetView.GetAddressOf(), depthStencilView.Get());
	immediateContext->RSSetViewports(1, &viewport);


}

RawTexture2D* CascadedShadowMaps::GetDepthTexture()
{
	return depthTexture.get();
}


//void CascadedShadowMaps::Deactivate(ID3D11DeviceContext* immediateContext)
//{
//	immediateContext->RSSetViewports(viewportCount, cachedViewports);
//	immediateContext->OMSetRenderTargets(1, cachedRenderTargetView.GetAddressOf(), cachedDepthStencilView.Get());
//
//	cachedRenderTargetView.Reset();
//	cachedDepthStencilView.Reset();
//}