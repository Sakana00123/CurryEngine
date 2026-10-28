#include "pch.h"
#include "ParticleSystem.h"

#include "Engine/Resources/Shader.h"
#include "Engine/Core/Misc.h"

using namespace DirectX;

#include <imgui.h>

ParticleSystem::ParticleSystem(ID3D11Device* device, int particleCount) : maxParticleCount(particleCount)
{
	HRESULT hr{ S_OK };
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Particle) * particleCount);
	bufferDesc.StructureByteStride = sizeof(Particle);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	hr = device->CreateBuffer(&bufferDesc, NULL, particleBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	shaderResourceViewDesc.Buffer.ElementOffset = 0;
	shaderResourceViewDesc.Buffer.NumElements = static_cast<UINT>(particleCount);
	hr = device->CreateShaderResourceView(particleBuffer.Get(), &shaderResourceViewDesc, particleBufferSrv.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc{};
	unorderedAccessViewDesc.Format = DXGI_FORMAT_UNKNOWN;
	unorderedAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	unorderedAccessViewDesc.Buffer.FirstElement = 0;
	unorderedAccessViewDesc.Buffer.NumElements = static_cast<UINT>(particleCount);
	unorderedAccessViewDesc.Buffer.Flags = 0;
	hr = device->CreateUnorderedAccessView(particleBuffer.Get(), &unorderedAccessViewDesc, particleBufferUav.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	bufferDesc.ByteWidth = sizeof(ParticleSystemConstants);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	CreateVertexShaderFromCSO(device, "./Data/Shaders/particleVS.cso", particleVs.ReleaseAndGetAddressOf(), NULL, NULL, 0);
	CreatePixelShaderFromCSO(device, "./Data/Shaders/particlePS.cso", particlePs.ReleaseAndGetAddressOf());
	CreateGeometryShaderFromCSO(device, "./Data/Shaders/particleGS.cso", particleGs.ReleaseAndGetAddressOf());
	CreateComputeShaderFromCSO(device, "./Data/Shaders/integrateParticleCS.cso", particleCs.ReleaseAndGetAddressOf());
	CreateComputeShaderFromCSO(device, "./Data/Shaders/initializeParticleCS.cso", particleInitializerCs.ReleaseAndGetAddressOf());
}

UINT align(UINT num, UINT alignment)
{
	return (num + (alignment - 1)) & ~(alignment - 1);
}

void ParticleSystem::Integrate(ID3D11DeviceContext* immediateContext, float deltaTime)
{
	immediateContext->CSSetUnorderedAccessViews(0, 1, particleBufferUav.GetAddressOf(), NULL);

	particleSystemData.time += deltaTime;
	particleSystemData.deltaTime = deltaTime;
	particleSystemData.maxParticleCount = maxParticleCount;
	immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &particleSystemData, 0, 0);
	immediateContext->CSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());

	immediateContext->CSSetShader(particleCs.Get(), NULL, 0);

	const UINT threadGroupCountX = align(static_cast<UINT>(maxParticleCount), NUMTHREADS_X) / NUMTHREADS_X;
	immediateContext->Dispatch(threadGroupCountX, 1, 1);

	ID3D11UnorderedAccessView* nullUnorderedAccessView{};
	immediateContext->CSSetUnorderedAccessViews(0, 1, &nullUnorderedAccessView, NULL);
}

void ParticleSystem::Initialize(ID3D11DeviceContext* immediateContext, float deltaTime)
{
	immediateContext->CSSetUnorderedAccessViews(0, 1, particleBufferUav.GetAddressOf(), NULL);

	particleSystemData.time = 0;
	particleSystemData.deltaTime = deltaTime;
	particleSystemData.maxParticleCount = maxParticleCount;
	immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &particleSystemData, 0, 0);
	immediateContext->CSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());

	immediateContext->CSSetShader(particleInitializerCs.Get(), NULL, 0);

	const UINT threadGroupCountX = align(static_cast<UINT>(maxParticleCount), NUMTHREADS_X) / NUMTHREADS_X;
	immediateContext->Dispatch(threadGroupCountX, 1, 1);

	ID3D11UnorderedAccessView* nullUnorderedAccessView{};
	immediateContext->CSSetUnorderedAccessViews(0, 1, &nullUnorderedAccessView, NULL);
}

void ParticleSystem::Render(ID3D11DeviceContext* immediateContext)
{
	immediateContext->VSSetShader(particleVs.Get(), NULL, 0);
	immediateContext->PSSetShader(particlePs.Get(), NULL, 0);
	immediateContext->GSSetShader(particleGs.Get(), NULL, 0);
	immediateContext->GSSetShaderResources(9, 1, particleBufferSrv.GetAddressOf());

	immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &particleSystemData, 0, 0);
	immediateContext->VSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());
	immediateContext->PSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());
	immediateContext->GSSetConstantBuffers(9, 1, constantBuffer.GetAddressOf());

	immediateContext->IASetInputLayout(NULL);
	immediateContext->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	immediateContext->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	immediateContext->Draw(static_cast<UINT>(maxParticleCount), 0);

	ID3D11ShaderResourceView* nullShaderResourceView{};
	immediateContext->GSSetShaderResources(9, 1, &nullShaderResourceView);
	immediateContext->VSSetShader(NULL, NULL, 0);
	immediateContext->PSSetShader(NULL, NULL, 0);
	immediateContext->GSSetShader(NULL, NULL, 0);
}

void ParticleSystem::DrawProperty()
{
#ifdef USE_IMGUI
	ImGui::Checkbox("respawn", &particleSystemData.respawn);
	ImGui::SliderFloat("gravity", &particleSystemData.gravity, -1.0f, +1.0f, "%.4f");
	ImGui::SliderFloat("lifespan min", &particleSystemData.lifespan.x, +0.0f, +10.0f, "%.4f");
	ImGui::SliderFloat("lifespan max", &particleSystemData.lifespan.y, +0.0f, +10.0f, "%.4f");
	ImGui::SliderFloat("spawn_delay min", &particleSystemData.spawnDelay.x, +0.0f, +10.0f, "%.4f");
	ImGui::SliderFloat("spawn_delay max", &particleSystemData.spawnDelay.y, +0.0f, +10.0f, "%.4f");
	ImGui::SliderFloat("fade in duration", &particleSystemData.fadeDuration.x, +0.0f, 10.0f, "%.4f");
	ImGui::SliderFloat("fade out duration", &particleSystemData.fadeDuration.y, +0.0f, 10.0f, "%.4f");
	ImGui::SliderFloat("emission_position.x", &particleSystemData.emissionPosition.x, -10.0f, +10.0f);
	ImGui::SliderFloat("emission_position.y", &particleSystemData.emissionPosition.y, -10.0f, +10.0f);
	ImGui::SliderFloat("emission_position.z", &particleSystemData.emissionPosition.z, -10.0f, +10.0f);
	ImGui::SliderFloat("emission_offset min", &particleSystemData.emissionOffset.x, +0.0f, +10.0f, "%.4f");
	ImGui::SliderFloat("emission_offset max", &particleSystemData.emissionOffset.y, +0.0f, +10.0f, "%.4f");
	ImGui::SliderFloat("emission_size spawn", &particleSystemData.emissionSize.x, +0.0f, +1.0f, "%.4f");
	ImGui::SliderFloat("emission_size despawn", &particleSystemData.emissionSize.y, +0.0f, +1.0f, "%.4f");
	ImGui::SliderFloat("emission_speed min", &particleSystemData.emissionSpeed.x, +0.0f, +10.0f, "%.4f");
	ImGui::SliderFloat("emission_speed max", &particleSystemData.emissionSpeed.y, +0.0f, +10.0f, "%.4f");
	ImGui::SliderFloat("emission_angular_speed min", &particleSystemData.emissionAngularSpeed.x, +0.0f, +10.0f, "%.4f");
	ImGui::SliderFloat("emission_angular_speed max", &particleSystemData.emissionAngularSpeed.y, +0.0f, +10.0f, "%.4f");
	ImGui::SliderFloat("emission_cone_angle min", &particleSystemData.emissionConeAngle.x, +0.0f, +3.141592653f, "%.4f");
	ImGui::SliderFloat("emission_cone_angle max", &particleSystemData.emissionConeAngle.y, +0.0f, +3.141592653f, "%.4f");
	ImGui::SliderFloat("noise_scale", &particleSystemData.noiseScale, +0.0f, +1.0f, "%.4f");
#endif // USE_IMGUI
}