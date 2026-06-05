#include "sprite.hlsli"
#include "Sampler.hlsli"
Texture2D color_map : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = color_map.Sample(samplerStates[ANISOTROPIC], pin.texcoord);
    //color = saturate(color);
    return color * pin.color;
}