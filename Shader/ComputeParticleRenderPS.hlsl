#include "ComputeParticle.hlsli"
#include "Sampler.hlsli"

Texture2D colorMap : register(t0);

float4 main(PS_IN pin) : SV_TARGET
{
    return colorMap.Sample(samplerStates[ANISOTROPIC], pin.texcoord) * pin.color;
}