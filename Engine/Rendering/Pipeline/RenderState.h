#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <vector>

/**
 * @file
 * @brief 各種 D3D11 レンダーステート（サンプラー/深度/ブレンド/ラスタライザ）のプリセット管理。
 * @details 生成済みステートの取得やバインド関数を提供します。
 */

// サンプラーステート
/**
 * @brief サンプラーステートのプリセット種別。
 */
enum class SamplerState
{
	PointWrap,
	PointBorder,
	PointClamp,
	LinearWrap,
	LinearBorder,
	LinearClamp,
	AnisotropicWrap,
	AnisotropicBorder,
	AnisotropicClamp,

	LinearBorderBack,
	LinearBorderWhite,

	Comparison,

	EnumCount,
};

/**
 * @brief サンプラーバインドのグルーピングフラグ。
 * @details Wrap/Border/Clamp ごとにプリセットをまとめてバインドする用途。
 */
enum class SamplerFlags { Wrap, Border, Clamp, EnumCount };

// デプスステート
/**
 * @brief 深度ステンシルステートのプリセット種別。
 */
enum class DepthStencilState
{
	TestAndWrite,
	WriteOnly,
	TestOnly,
	NoTestNoWrite,

	EnumCount
};

// ブレンドステート
/**
 * @brief ブレンドステートのプリセット種別。
 */
enum class BlendState
{
	Opaque,
	Transparency,
	Additive,
	Subtraction,
	Multiply,

	EnumCount
};

// ラスタライザステート
/**
 * @brief ラスタライザステートのプリセット種別。
 */
enum class RasterizerState
{
	SolidCullNone,
	SolidCullFront,
	SolidCullBack,
	WireCullNone,
	WireCullBack,
	UseScissorRects,

	EnumCount
};

/**
 * @brief レンダーステートプリセットをまとめて保持・適用するクラス。
 */
class RenderState
{
public:
	/**
	 * @brief ステートオブジェクト群を作成します。
	 * @param device D3D11 デバイス。
	 */
	RenderState(ID3D11Device* device);
	~RenderState() = default;

	/**
	 * @brief サンプラーステートをまとめてバインドします（プリセット）。
	 * @param immediateContext デバイスコンテキスト。
	 * @param flag Wrap/Border/Clamp のいずれか。
	 */
	void BindSamplerStates(ID3D11DeviceContext* immediateContext) const {
		std::vector<ID3D11SamplerState*> samplers;
		for (size_t i = 0; i < static_cast<size_t>(SamplerState::EnumCount); ++i) {
			samplers.push_back(samplerStates[i].Get());
		}
		immediateContext->PSSetSamplers(0, static_cast<UINT>(samplers.size()), samplers.data());
		immediateContext->GSSetSamplers(0, static_cast<UINT>(samplers.size()), samplers.data());
		immediateContext->CSSetSamplers(0, static_cast<UINT>(samplers.size()), samplers.data());
	}
	/**
	 * @brief サンプラーステートを取得します。
	 * @param state 取得するプリセット。
	 * @return `ID3D11SamplerState*` のアドレス。
	 */
	ID3D11SamplerState** GetSamplerState(SamplerState state) {
		size_t index = static_cast<size_t>(state);
		return samplerStates[index].GetAddressOf();
	}
	/**
	 * @brief 深度ステンシルステートをバインドします。
	 * @param immediateContext デバイスコンテキスト。
	 * @param state バインドするプリセット。
	 * @param stencilRef ステンシル参照値。
	 */
	void BindDepthStencilState(ID3D11DeviceContext* immediateContext, DepthStencilState state, UINT stencilRef = 0) {
		immediateContext->OMSetDepthStencilState(depthStencilStates[static_cast<size_t>(state)].Get(), stencilRef);
	}
	/**
	 * @brief ブレンドステートをバインドします。
	 * @param immediateContext デバイスコンテキスト。
	 * @param state バインドするプリセット。
	 */
	void BindBlendState(ID3D11DeviceContext* immediateContext, BlendState state) {
		immediateContext->OMSetBlendState(blendStates[static_cast<size_t>(state)].Get(), NULL, 0xFFFFFFFF);
	}
	/**
	 * @brief ラスタライザステートをバインドします。
	 * @param immediateContext デバイスコンテキスト。
	 * @param state バインドするプリセット。
	 */
	void BindRasterizerState(ID3D11DeviceContext* immediateContext, RasterizerState state) {
		immediateContext->RSSetState(rasterizerStates[static_cast<size_t>(state)].Get());
	}

public:
	/** @brief サンプラーステート配列。*/
	Microsoft::WRL::ComPtr<ID3D11SamplerState>		samplerStates[static_cast<int>(SamplerState::EnumCount)];
	/** @brief 深度ステンシルステート配列。*/
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStates[static_cast<int>(DepthStencilState::EnumCount)];
	/** @brief ブレンドステート配列。*/
	Microsoft::WRL::ComPtr<ID3D11BlendState>		blendStates[static_cast<int>(BlendState::EnumCount)];
	/** @brief ラスタライザステート配列。*/
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>	rasterizerStates[static_cast<int>(RasterizerState::EnumCount)];
};