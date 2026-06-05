#include "pch.h"
#include "ConstantBufferPass.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Rendering/Lights/DirectionalLightComponent.h"
#include "Engine/Rendering/Lights/PointLightComponent.h"
#include "Engine/Rendering/Lights/SpotLightComponent.h"

void ConstantBufferPass::Initialize()
{
	// 定数バッファの初期化やリソースの準備を行う
	auto device = Graphics::GetDevice();
	HRESULT hr{ S_OK };

    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = sizeof(SceneConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffers[0].GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

    bufferDesc.ByteWidth = sizeof(ShadowConstants);
    hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffers[1].GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

    bufferDesc.ByteWidth = sizeof(LightConstants);
    hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffers[2].GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}


void ConstantBufferPass::Execute(RenderContext* rtx, Scene* scene)
{
	// シーン定数バッファの更新
	sceneConstants.view = rtx->view;
	sceneConstants.projection = rtx->projection;
	sceneConstants.viewProjection = rtx->viewProjection;
	sceneConstants.inverseView = rtx->inverseView;
	sceneConstants.inverseProjection = rtx->inverseProjection;
	sceneConstants.inverseViewProjection = rtx->inverseViewProjection;
	sceneConstants.cameraPosition.x = rtx->cameraPosition.x;
	sceneConstants.cameraPosition.y = rtx->cameraPosition.y;
	sceneConstants.cameraPosition.z = rtx->cameraPosition.z;
	sceneConstants.cameraPosition.w = 1;
	sceneConstants.time = rtx->totalTime;
	sceneConstants.deltaTime = rtx->deltaTime;
	sceneConstants.unscaledDeltaTime = rtx->unscaledDeltaTime;
	Graphics::GetScreenSize(sceneConstants.screenSize.x, sceneConstants.screenSize.y);
	
	// シャドウ定数バッファの更新(いまは固定値)
	//shadowConstants.min = 0.6f;
	//shadowConstants.max = 0.8f;
	//shadowConstants.gaussian_sigma = 1.0f;
	//shadowConstants.bloom_intensity = 1.0f;
	shadowConstants.shadowColor = 0.7f;
	shadowConstants.shadowDepthBias = 0.000175f;
	shadowConstants.colorizeCascadedLayer = false;
    
	// ライト定数バッファの更新
	DirectionalLight directionalLightData = scene->directionalLight ? scene->directionalLight->GetDirectionalLight() : DirectionalLight{};
	LightConstants lightConstants{};
	lightConstants.ambientColor = { 0.1f, 0.1f, 0.1f, 1.0f };
	lightConstants.directionalLightDirection = directionalLightData.direction;
	lightConstants.directionalLightColor = directionalLightData.color;
	
	// RenderContextにもライトの向きを設定しておく（シェーダで直接定数バッファから取るのではなく、RenderContextから取る場合もあるため）
	rtx->lightDirection = scene->directionalLight ? directionalLightData.direction : DirectX::XMFLOAT4{ -0.3f,-0.94f,0.36f,0.0f }; // デフォルトのライトの向きは、シーンにディレクショナルライトがない場合の値と同じにしておく
	
	for (size_t i = 0; i < scene->pointLights.size() && i < 8; ++i)
	{
		PointLight pointLightData = scene->pointLights[i]->GetPointLightData();
		lightConstants.pointLights[i] = pointLightData;
	}
	for (size_t i = 0; i < scene->spotLights.size() && i < 8; ++i)
	{
		SpotLight spotLightData = scene->spotLights[i]->GetSpotLightData();
		lightConstants.spotLights[i] = spotLightData;
	}
	
    // 定数バッファにデータを転送してシェーダにバインドする
    auto immediateContext = rtx->immediateContext;
    // シーン定数バッファの更新とバインド
    immediateContext->UpdateSubresource(constantBuffers[0].Get(), 0, nullptr, &sceneConstants, 0, 0);
    immediateContext->VSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());
    immediateContext->PSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());
    immediateContext->GSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());
    immediateContext->CSSetConstantBuffers(1, 1, constantBuffers[0].GetAddressOf());
    // シャドウ定数バッファの更新とバインド
    immediateContext->UpdateSubresource(constantBuffers[1].Get(), 0, nullptr, &shadowConstants, 0, 0);
    immediateContext->PSSetConstantBuffers(2, 1, constantBuffers[1].GetAddressOf());
    immediateContext->VSSetConstantBuffers(2, 1, constantBuffers[1].GetAddressOf());
    immediateContext->GSSetConstantBuffers(2, 1, constantBuffers[1].GetAddressOf());
    immediateContext->CSSetConstantBuffers(2, 1, constantBuffers[1].GetAddressOf());
    // ライト定数バッファの更新とバインド
    immediateContext->UpdateSubresource(constantBuffers[2].Get(), 0, nullptr, &lightConstants, 0, 0);
    immediateContext->PSSetConstantBuffers(4, 1, constantBuffers[2].GetAddressOf());
    immediateContext->VSSetConstantBuffers(4, 1, constantBuffers[2].GetAddressOf());
    immediateContext->GSSetConstantBuffers(4, 1, constantBuffers[2].GetAddressOf());
    immediateContext->CSSetConstantBuffers(4, 1, constantBuffers[2].GetAddressOf());
}