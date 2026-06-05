#include "particle.hlsli"
#include "Sampler.hlsli"
Texture2D colorMap : register(t0);

float4 main(GS_OUT pin) : SV_TARGET
{
    float4 color = pin.color;
    //return color;
   
#if 0
    color *= dot(colorMap.Sample(samplerStates[LINEAR], pin.texcoord).rgb, float3(0.299, 0.587, 0.114));
#else
    color = colorMap.Sample(samplerStates[LINEAR], pin.texcoord);
#endif
    
#if 0
	float2 p = float2((pin.texcoord.x - 0.5) * 2, (0.5 - pin.texcoord.y) * 2);
	float3 n = normalize(float3(p.x, p.y, -sqrt(1 - (p.x * p.x + p.y * p.y)))); // view space
	float3 l = normalize(mul(-lightDirection, view)).xyz; // to view space
#if 0
	color.rgb *= max(0.0, 0.5 * dot(n, l) + 0.5);
#else
	color.rgb *= max(0.0, dot(n, l));
#endif
	
#if 0
	color.a *= smoothstep(0.5, 0.8, 1.0 - length(pin.texcoord - 0.5));
#endif
#endif

    return color;
}