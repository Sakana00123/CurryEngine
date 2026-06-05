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

struct RenderContext
{
	// コンストラクタ
	RenderContext(ID3D11DeviceContext* context, FullScreenQuad* fullScreenQuad, std::unordered_map<std::string, void*> sharedResources);
	// デストラクタ
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
	void SetSharedResource(const std::string& key, void* resource);

	// 共有リソースの取得
	void* GetSharedResource(const std::string& key) const;

	// レンダーターゲットの設定
	void SetRenderTarget(const RenderTexture& target);

	// デフォルトのレンダーターゲットに切り替える
	void SetDefaultRenderTarget();

	// 現在のレンダーターゲットをクリア
	void ClearCurrentRenderTarget(const Color& color) const;

	// フルスクリーン描画
	void DrawFullScreenQuad(ID3D11ShaderResourceView** shaderResourceViews, uint32_t startSlot, uint32_t numViews, ID3D11PixelShader* replacedPixelShader = nullptr);
	void DrawFullScreenQuad(Material* material);

	// フルスクリーンクアッドの参照
	FullScreenQuad* fullScreenQuad;
private:
	// シェーダーリソースビューをすべて解除
	void UnbindSRVs() const;

	// 現在のデバイスコンテキスト
	ID3D11DeviceContext* m_context{ nullptr };

	// 現在のレンダーターゲット
	const RenderTexture* m_currentRenderTarget{};
	
	// 共有リソースの管理
	std::unordered_map<std::string, void*> sharedResources;


};