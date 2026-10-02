#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

#include "Engine/Resources/Texture.h"
#include "Engine/Utils/JsonFileHandler.h"

/** @brief Skymap を表すクラスです。 */
class Skymap
{
public:
	/**
	 * @brief Skymap を構築します。
	 */
	Skymap(ID3D11Device* device);
	/**
	 * @brief Skymap を破棄します。
	 */
	virtual ~Skymap() = default;
	/**
	 * @brief Skymap を構築します。
	 */
	Skymap(const Skymap&) = delete;
	/**
	 * @brief 演算子処理を行います。
	 */
	Skymap& operator=(const Skymap&) = delete;
	/**
	 * @brief Skymap を構築します。
	 */
	Skymap(Skymap&&) = delete;
	/**
	 * @brief 演算子処理を行います。
	 */
	Skymap& operator=(Skymap&&) noexcept = delete;

	/**
	 * @brief 描画処理を行います。
	 */
	void Draw(ID3D11DeviceContext* immediateContext);


	// プロパティ描画
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty();

	// シリアライズ
	/**
	 * @brief Serialize の処理を行います。
	 */
	json Serialize() const;

	// デシリアライズ
	/**
	 * @brief Deserialize の処理を行います。
	 */
	void Deserialize(const json& j);

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> skymap_vs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> skymap_ps;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> skybox_ps;

	std::shared_ptr<AssetTexture> texture;

	bool isTextureCube = false;
};
