#pragma once
#include "RenderPass.h"
#include "Engine/Rendering/Pipeline/LightData.h"

class ConstantBufferPass : public RenderPass
{
public:
	// ConstantBufferPassの初期化処理
	void Initialize() override;

	// ConstantBufferPassの実装
	void Execute(RenderContext* rtx, Scene* scene) override;

private:
    struct SceneConstants
    {
        DirectX::XMFLOAT4X4 view{};
        DirectX::XMFLOAT4X4 projection{};
        DirectX::XMFLOAT4X4 viewProjection{};//ビュー・プロジェクション変換行列
        DirectX::XMFLOAT4X4 inverseView{};
        DirectX::XMFLOAT4X4 inverseProjection{};
        DirectX::XMFLOAT4X4 inverseViewProjection{};
        DirectX::XMFLOAT4 cameraPosition{};

        float time = 0.0f;
		float deltaTime = 0.0f;
		float unscaledDeltaTime = 0.0f;
        float pad{};
		Vector2 screenSize{ 0.0f, 0.0f };
		Vector2 pad2{ 0.0f, 0.0f };
    };
	SceneConstants sceneConstants;
	
    struct ShadowConstants
    {
        //CascadedShadowMaps
        float shadowColor = 0.7f;
        //float shadowDepthBias = 0.00175f;
        float shadowDepthBias = 0.000175f;
        bool colorizeCascadedLayer = false;
        int pad{};
    };
    ShadowConstants shadowConstants{};

	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffers[3];//0:SceneConstants, 1:ConstantTest, 2:LightConstants
};