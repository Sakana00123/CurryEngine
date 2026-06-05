#include "skymap.hlsli"
#include "Sampler.hlsli"
Texture2D skymap : register(t0); // latitude-longitude mapped texture

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 R = mul(float4((pin.texcoord.x * 2.0) - 1.0, 1.0 - (pin.texcoord.y * 2.0), 1, 1), inverseViewProjection);
    R /= R.w;
    float3 v = normalize(R.xyz);
	
    // Blinn/Newell Latitude Mapping
    const float PI = 3.14159265358979;
    float2 samplePoint;

    samplePoint.x = (atan2(v.z, v.x) + PI) / (PI * 2.0);
    samplePoint.y = 1.0 - ((asin(v.y) + PI * 0.5) / PI);
    
    const float lod = 0;
    return skymap.SampleLevel(samplerStates[LINEAR], samplePoint, lod);
}