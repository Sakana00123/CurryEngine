#include "skymap.hlsli"
#include "Sampler.hlsli"
TextureCube skybox : register(t0); // latitude-longitude mapped texture

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 R = mul(float4((pin.texcoord.x * 2.0) - 1.0, 1.0 - (pin.texcoord.y * 2.0), 1, 1), inverseViewProjection);
    R /= R.w;
    
    const float lod = 0;
    return skybox.SampleLevel(samplerStates[LINEAR], R.xyz, lod);
}