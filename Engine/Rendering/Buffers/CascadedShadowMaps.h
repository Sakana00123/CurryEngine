#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

#include <vector>
#include <functional>
#include "Engine/Resources/Texture.h"

// https://learnopengl.com/Guest-Articles/2021/CSM
// https://learn.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps
// https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-10-parallel-split-shadow-maps-programmable-gpus
class CascadedShadowMaps
{
public:
	CascadedShadowMaps(ID3D11Device* device, UINT width, UINT height, UINT cascadeCount = 4);
	virtual ~CascadedShadowMaps() = default;
	CascadedShadowMaps(const CascadedShadowMaps&) = delete;
	CascadedShadowMaps& operator=(const CascadedShadowMaps&) = delete;
	CascadedShadowMaps(CascadedShadowMaps&&) noexcept = delete;
	CascadedShadowMaps& operator=(CascadedShadowMaps&&) noexcept = delete;

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	D3D11_VIEWPORT viewport;

	std::vector<DirectX::XMFLOAT4X4> cascadedMatrices;
	std::vector<float> cascadedPlaneDistances;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	std::shared_ptr<RawTexture2D> depthTexture;

	struct Constants
	{
		DirectX::XMFLOAT4X4 cascadedMatrices[4];
		float cascadedPlaneDistances[4];
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

public:

	// Resize the depth map and related views
	void Resize(ID3D11Device* device, UINT width, UINT height);


	void Activate(ID3D11DeviceContext* immediateContext,
		const DirectX::XMFLOAT4X4& cameraView,
		const DirectX::XMFLOAT4X4& cameraProjection,
		const DirectX::XMFLOAT4& lightDirection,
		float criticalDepthValue/*If this value is 0, the camera's far panel distance is used.*/,
		UINT slot);

	//void Deactivate(ID3D11DeviceContext* immediateContext);
	void Clear(ID3D11DeviceContext* immediateContext)
	{
		immediateContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1, 0);
	}
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& DepthMap()
	{
		return shaderResourceView;
	}

	/** @brief 深度テクスチャを取得します。*/
	RawTexture2D* GetDepthTexture();

public:
	const UINT cascadeCount;
	float splitSchemeWeight = 0.7f; // logarithmic_split_scheme * _split_scheme_weight + uniform_split_scheme * (1 - _split_scheme_weight)
	// https://learn.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps
	// Fit to scene vs.fit to cascade
	// - Fit to Scene
	//	All of the frusta can be created with the same near plane.This forces the cascades to overlap.
	// - Fit to Cascade
	//	Alternatively, frusta can be created with the actual partition interval being used as near and far planes.This causes a tighter fit, but degenerates to fit to scene in the case of dueling frusta.
	// Fit to cascade wastes less resolution.The problem with fit to cascade is that the orthographic projection grows and shrinks based on the orientation of the view frustum.
	// The fit to scene technique pads the orthographic projection by the max size of the view frustum removing the artifacts that appear when the view - camera moves.
		// Common Techniques to Improve Shadow Depth Maps addresses the artifacts that appear when the light moves in the section "Moving the light in texel sized increments."
	bool fitToCascade = true;
	// Before creating the actual projection matrix we are going to increase the size of the space covered by the nearand far plane of the light frustum.
	// We do this by "pulling back" the near plane, and "pushing away" the far plane.In the code we achieve this by dividing or multiplying by zMult.
	// This is because we want to include geometry which is behind or in front of our frustum in camera space. Think about it : not only geometry which 
	// is in the frustum can cast shadows on a surface in the frustum!
	float zMult = 10.0f;

private:
	D3D11_VIEWPORT cachedViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	UINT viewportCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cachedRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cachedDepthStencilView;
};