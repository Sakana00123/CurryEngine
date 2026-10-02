#pragma once
#include <string>
#include <unordered_map>

#include "Engine/Core/Color.h"
#include "Engine/Core/Math/Vector3.h"
#include "Engine/Rendering/Buffers/RenderTexture.h"
class Material;
struct ID3D11DeviceContext;
struct DirectX::XMFLOAT3;
class RenderState;
class FullScreenQuad;

/** @brief RenderContext を表す構造体です。 */
struct RenderContext
{
	// コンストラクタ
	/**
	 * @brief RenderContext を構築します。
	 */
	RenderContext(ID3D11DeviceContext* context, FullScreenQuad* fullScreenQuad, std::unordered_map<std::string, void*> sharedResources);
	// デストラクタ
	/**
	 * @brief RenderContext を破棄します。
	 */
	~RenderContext() = default;

	// 描画に必要なコンテキスト情報をここに追加
	ID3D11DeviceContext* immediateContext;
	RenderState* renderState;
	Vector3 cameraPosition;
	DirectX::XMFLOAT4 lightDirection;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4X4 viewProjection;
	DirectX::XMFLOAT4X4 inverseView;
	DirectX::XMFLOAT4X4 inverseProjection;
	DirectX::XMFLOAT4X4 inverseViewProjection;


	float deltaTime{ 0.0f }; // 前フレームからの経過時間（秒）
	float unscaledDeltaTime{ 0.0f }; // 前フレームからの経過時間（秒、スケーリングなし）
	float totalTime{ 0.0f }; // アプリケーション開始からの総経過時間（秒、スケーリングなし）

	bool acceptRendering{ true }; // 描画を許可するかどうかのフラグ。カメラがないなど描画できない状況でfalseになる

	// 共有リソースの設定
	/**
	 * @brief SharedResource を設定します。
	 */
	void SetSharedResource(const std::string& key, void* resource);

	// 共有リソースの取得
	/**
	 * @brief GetSharedResource に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	void* GetSharedResource(const std::string& key) const;

	// レンダーターゲットの設定
	/**
	 * @brief RenderTarget を設定します。
	 */
	void SetRenderTarget(const RenderTexture& target);

	// デフォルトのレンダーターゲットに切り替える
	/**
	 * @brief DefaultRenderTarget を設定します。
	 */
	void SetDefaultRenderTarget();

	// 現在のレンダーターゲットをクリア
	/**
	 * @brief 保持している内容を消去します。
	 */
	void ClearCurrentRenderTarget(const Color& color) const;

	// フルスクリーン描画
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawFullScreenQuad(ID3D11ShaderResourceView** shaderResourceViews, uint32_t startSlot, uint32_t numViews, ID3D11PixelShader* replacedPixelShader = nullptr);
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawFullScreenQuad(Material* material);

	// フルスクリーンクアッドの参照
	FullScreenQuad* fullScreenQuad;
private:
	// シェーダーリソースビューをすべて解除
	/**
	 * @brief UnbindSRVs の処理を行います。
	 */
	void UnbindSRVs() const;

	// 現在のデバイスコンテキスト
	ID3D11DeviceContext* m_context{ nullptr };

	// 現在のレンダーターゲット
	const RenderTexture* m_currentRenderTarget{};
	
	// 共有リソースの管理
	std::unordered_map<std::string, void*> sharedResources;


};