#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

#include <vector>
#include "Engine/Resources/Texture.h"

#define NUMTHREADS_X 16

/** @brief Particle を表す構造体です。 */
struct Particle
{
	int state = 0;

	DirectX::XMFLOAT4 color = { 1,1,1,1 };

	DirectX::XMFLOAT3 position = { 0,0,0 };
	float mass = 1.0f;

	float angle = 0.0f;
	float angularSpeed = 0.0f;
	DirectX::XMFLOAT3 velocity = { 0,0,0 };

	float lifespan = 1.0f;
	float age = 0.0f;

	DirectX::XMFLOAT2 size{ 0 }; // x: spawn, y: despawn

	int chip = 0;
};

/** @brief ParticleSystem を表す構造体です。 */
struct ParticleSystem
{
	const int maxParticleCount;

	/** @brief ParticleSystemConstants を表す構造体です。 */
	struct ParticleSystemConstants
	{
		DirectX::XMFLOAT4 emissionPosition{};
		DirectX::XMFLOAT2 emissionOffset{ 0.0f,0.0f };// x: minimum radius, y: maximum radius
		DirectX::XMFLOAT2 emissionSize{ 1.f,0.7f }; // x: spawn, y: despawn
		DirectX::XMFLOAT2 emissionConeAngle{ 0.0f, 0.f };// x: minimum radian, y: maximum radian
		DirectX::XMFLOAT2 emissionSpeed{ 10.f, 10.0f };// x: minimum speed, y: maximum speed
		DirectX::XMFLOAT2 emissionAngularSpeed{ 0.f, 1.f };// x: minimum angular speed, y: maximum angular speed
		DirectX::XMFLOAT2 lifespan{ 1.0f, 2.5f };// x: minimum second, y: maximum second
		DirectX::XMFLOAT2 spawnDelay{ 0.0f, 0.0f }; // x: minimum second, y: maximum second
		DirectX::XMFLOAT2 fadeDuration{ 1.0f, 2.5f }; // x: fade in, y: fade out

		float time = 0.0f;
		float deltaTime = 0.0f;
		float noiseScale = 0.001f;
		float gravity = 1.f;

		DirectX::XMUINT2 spriteSheetGrid = { 1,1 };

		int maxParticleCount = 0;
		bool respawn = false;
		bool pads[3];
	};
	ParticleSystemConstants particleSystemData;

	Microsoft::WRL::ComPtr<ID3D11Buffer> particleBuffer;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleBufferUav;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleBufferSrv;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> particleVs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> particlePs;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> particleGs;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> particleCs;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> particleInitializerCs;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	/**
	 * @brief ParticleSystem を構築します。
	 */
	ParticleSystem(ID3D11Device* device, int particleCount);
	/**
	 * @brief ParticleSystem を構築します。
	 */
	ParticleSystem(const ParticleSystem&) = delete;
	/**
	 * @brief 演算子処理を行います。
	 */
	ParticleSystem& operator=(const ParticleSystem&) noexcept = delete;
	/**
	 * @brief ParticleSystem を構築します。
	 */
	ParticleSystem(ParticleSystem&&) noexcept = delete;
	/**
	 * @brief 演算子処理を行います。
	 */
	ParticleSystem& operator=(ParticleSystem&&) noexcept = delete;
	/**
	 * @brief ParticleSystem を破棄します。
	 */
	virtual ~ParticleSystem() = default;

	/**
	 * @brief Integrate の処理を行います。
	 */
	void Integrate(ID3D11DeviceContext* immediateContext, float deltaTime);
	/**
	 * @brief 初期化します。
	 */
	void Initialize(ID3D11DeviceContext* immediateContext, float deltaTime);
	/**
	 * @brief 描画処理を行います。
	 */
	void Render(ID3D11DeviceContext* immediateContext);

	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty();
};