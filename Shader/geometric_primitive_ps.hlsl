#include "geometric_primitive.hlsli"
#include "Sampler.hlsli"

Texture2D texture0 : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 texColor = texture0.Sample(samplerStates[LINEAR], pin.texcoord);
	
    float4 finalColor = pin.color * texColor;
    
    return finalColor;
}
