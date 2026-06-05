#include "fullscreen_quad.hlsli"
#include "Sampler.hlsli"

Texture2D texture_map : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
#if 0
    float4 color = texture_map.Sample(sampler_states[LINEAR], pin.texcoord);
    return float4((color.rgb + 1.0)*0.5,color.a)
#else
    return texture_map.Sample(samplerStates[LINEAR], pin.texcoord);
#endif
}