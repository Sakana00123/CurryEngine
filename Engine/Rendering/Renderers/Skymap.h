#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

#include "Engine/Resources/Texture.h"
#include "Engine/Utils/JsonFileHandler.h"

class Skymap
{
public:
	Skymap(ID3D11Device* device);
	virtual ~Skymap() = default;
	Skymap(const Skymap&) = delete;
	Skymap& operator=(const Skymap&) = delete;
	Skymap(Skymap&&) = delete;
	Skymap& operator=(Skymap&&) noexcept = delete;

	void Draw(ID3D11DeviceContext* immediateContext);


	// プロパティ描画
	void DrawProperty();

	// シリアライズ
	json Serialize() const;

	// デシリアライズ
	void Deserialize(const json& j);

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> skymap_vs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> skymap_ps;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> skybox_ps;

	std::shared_ptr<AssetTexture> texture;

	bool isTextureCube = false;
};
