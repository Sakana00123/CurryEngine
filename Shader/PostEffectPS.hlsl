#include "fullscreen_quad.hlsli"
#include "Sampler.hlsli"
#include "Constants.hlsli"

SamplerComparisonState comparisonSamplerState : register(s5);

Texture2D colorMap : register(t0);
Texture2D depthMap : register(t1);
Texture2D<float4> bloomMap : register(t2);
Texture2DArray cascadedShadowMaps : register(t3);


float3 reinhard_tone_mapping(float3 color)
{
    float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
    float tone_mapped_luma = luma / (1. + luma);
    color *= tone_mapped_luma / luma;
    return color;
}

cbuffer PARAMETRIC_CONSTANT_BUFFER : register(b2)
{
    float extractionThreshold;
    float guassianSigma;
    float bloomIntensity;
    float _exposure;
    
    float shadowColor;
    float shadowDepthBias;
    bool colorizeCascadedLayer;
}

cbuffer CASCADED_CONSTANTS : register(b3)
{
    row_major float4x4 cascadedMatrices[4];
    float4 cascadedPlaneDistances;
}

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 sampledColor = colorMap.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord);
    float3 color = sampledColor.rgb;
    float alpha = sampledColor.a;

    //return float4(color, alpha);

    float4 bloom = bloomMap.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord);
    
    float depthNdc = depthMap.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord).x;
    
    float4 positionNdc;
    // texture space to ndc
    positionNdc.x = pin.texcoord.x * +2 - 1;
    positionNdc.y = pin.texcoord.y * -2 + 1;
    positionNdc.z = depthNdc;
    positionNdc.w = 1;
    
    // ndc to view space
    float4 positionViewSpace = mul(positionNdc, inverseProjection);
    positionViewSpace = positionViewSpace / positionViewSpace.w;
    
    float4 positionWorldSpace = mul(positionNdc, inverseViewProjection);
    positionWorldSpace = positionWorldSpace / positionWorldSpace.w;
    
    // Apply cascaded shadow mapping
	// Find a layer of cascaded view frustum volume 
    float depthViewSpace = positionViewSpace.z;
    int cascadeIndex = -1;
    for (uint layer = 0; layer < 4; ++layer)
    {
        float distance = cascadedPlaneDistances[layer];
        if (distance > depthViewSpace)
        {
            cascadeIndex = layer;
            break;
        }
    }
    float shadowFactor = 1.0;
    if (cascadeIndex > -1)
    {
        // world space to light view clip space, and to ndc
        float4 positionLightSpace = mul(positionWorldSpace, cascadedMatrices[cascadeIndex]);
        positionLightSpace /= positionLightSpace.w;
        // ndc to texture space
        positionLightSpace.x = positionLightSpace.x * +0.5 + 0.5;
        positionLightSpace.y = positionLightSpace.y * -0.5 + 0.5;
        
        shadowFactor = cascadedShadowMaps.SampleCmpLevelZero(comparisonSamplerState, float3(positionLightSpace.xy, cascadeIndex), positionLightSpace.z - shadowDepthBias).x;

        float3 layerColor = 1;
        if (colorizeCascadedLayer)
        {
            const float3 layerColors[4] =
            {
                { 1, 0, 0 },
                { 0, 1, 0 },
                { 0, 0, 1 },
                { 1, 1, 0 },
            };
            layerColor = layerColors[cascadeIndex];
        }
        
        color *= lerp(shadowColor, 1.0, shadowFactor) * layerColor;
    }

    float3 fragment_color = color.rgb + bloom.rgb;
    return float4(color, alpha);

	// Tone map
    //fragment_color = reinhard_tone_mapping(fragment_color);

	//// Gamma correction
 //   const float INV_GAMMA = 1.0 / 2.2;
 //   fragment_color = pow(fragment_color, INV_GAMMA);

	// Tone mapping : HDR -> SDR
    color = reinhard_tone_mapping(color);
#if 0
    const float exposure = 1.2;
    color = 1 - exp(-color * exposure);
#endif


#if 1
	// Gamma process
    const float GAMMA = 2.2;
    color = pow(color, 1.0 / GAMMA);
#endif
	
    return float4(color, alpha);
    return float4(fragment_color, alpha);
}