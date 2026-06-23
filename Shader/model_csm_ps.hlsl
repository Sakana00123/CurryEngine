#include "Sampler.hlsli"

// GS_OUTPUT_CSM は model.hlsli で定義しているが、
// CSM PS は VS/GS と別コンパイル単位になるため独立宣言する
struct GS_OUTPUT_CSM
{
    float4 position               : SV_POSITION;
    float2 texcoord               : TEXCOORD;
    float  depth                  : DEPTH;
    uint   renderTargetArrayIndex : SV_RENDERTARGETARRAYINDEX;
};

// アルファ抜きのためにオパシティテクスチャを参照
Texture2D<float4> materialOpacityTex : register(t31);

float main(GS_OUTPUT_CSM pin) : SV_DEPTH
{
    const float alpha_cutoff = 0.5;
    float opacity = materialOpacityTex.Sample(samplerStates[LINEAR], pin.texcoord).r;
    clip(opacity - alpha_cutoff);
    return pin.depth;
}
