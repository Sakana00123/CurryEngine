#include "Sampler.hlsli"
Texture2D<float4> colorMap : register(t0);
Texture2D<float4> materialOpacityTex : register(t31);

struct GS_OUTPUT_CSM
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
    float depth : DEPTH;
    uint renderTargetArrayIndex : SV_RENDERTARGETARRAYINDEX;
};

float main(GS_OUTPUT_CSM pin) : SV_DEPTH
{
    float alpha_cutoff = 0.5;
#if 1
    float opacity = materialOpacityTex.Sample(samplerStates[LINEAR], pin.texcoord).r;

    clip(opacity - alpha_cutoff);

    return pin.depth;
#else
    float alpha = colorMap.Sample(samplerStates[POINT], pin.texcoord).a;
    clip(alpha - alpha_cutoff);
    return pin.depth;
#endif
}