#pragma once
#include <DirectXMath.h>

//ポイントライト
struct PointLight
{
    UINT enable{ false };
    DirectX::XMFLOAT3 position{ 0,0,0 };
    DirectX::XMFLOAT4 color{ 1,1,1,1 };
    float range{ 10.0f };
    float dummy[3]{};
};
//スポットライト
struct SpotLight
{
    UINT enable{ false };
    DirectX::XMFLOAT3 position{ 0,0,0 };
    DirectX::XMFLOAT4 direction{ 0,0,1,0 };
    DirectX::XMFLOAT4 color{ 1,1,1,1 };
    float range{ 0 };
    float innerCorn{ 0.99f };
    float outerCorn{ 0.9f };
    float dummy;
};
// ディレクショナルライト
struct DirectionalLight
{
	UINT enable{ true };
    DirectX::XMFLOAT4 direction{ -0.3f,-0.94f,0.36f,0.0f };
    DirectX::XMFLOAT4 color{ 1,1,1,1 };
	float dummy[3]{};
};

// ライト定数バッファ用構造体
struct LightConstants
{
    DirectX::XMFLOAT4 ambientColor;
    DirectX::XMFLOAT4 directionalLightDirection;
    DirectX::XMFLOAT4 directionalLightColor;
    PointLight pointLights[8];
    SpotLight spotLights[8];
};