#include "sprite.hlsli"
#include "Sampler.hlsli"

Texture2D colorMap : register(t0);
Texture2D maskTexture : register(t1);


float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = colorMap.Sample(samplerStates[ANISOTROPIC], pin.texcoord);
    float maskValue = maskTexture.Sample(samplerStates[ANISOTROPIC], pin.texcoord);
    color.a *= maskValue;
    
	return color;
}