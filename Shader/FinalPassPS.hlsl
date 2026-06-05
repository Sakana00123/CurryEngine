#include "fullscreen_quad.hlsli"
#include "Sampler.hlsli"

Texture2D textureMap : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    return textureMap.Sample(samplerStates[LINEAR], pin.texcoord);
}