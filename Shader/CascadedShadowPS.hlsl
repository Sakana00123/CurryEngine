#include "fullscreen_quad.hlsli"
#include "Sampler.hlsli"
#include "Constants.hlsli"

SamplerComparisonState comparisonSamplerState : register(s11);

Texture2D colorMap : register(t0);
Texture2D depthMap : register(t1);
Texture2DArray cascadedShadowMaps : register(t2);

cbuffer PARAMETRIC_CONSTANT_BUFFER : register(b2)
{
    //float extractionThreshold;
    //float guassianSigma;
    //float bloomIntensity;
    //float _exposure;
    
    float shadowColor;
    float shadowDepthBias;
    bool colorizeCascadedLayer;
    int pad1;
}

cbuffer CASCADED_CONSTANTS : register(b3)
{
    row_major float4x4 cascadedMatrices[4];
    float4 cascadedPlaneDistances;
}
float ComputeShadowPCF(float3 shadowCoord, int cascadeIndex)
{
    // shadowCoord.xy : shadow map UV
    // shadowCoord.z  : receiver depth in light space
    float shadow = 0.0;
    float2 shadowMapSize;
    uint NOL;
    depthMap.GetDimensions(cascadeIndex, shadowMapSize.x, shadowMapSize.y, NOL);
    float2 shadowMapTexelSize = 1.0 / 8192;
    
    // 3x3 カーネル
    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            float2 offset = float2(x, y) * shadowMapTexelSize;
            float2 uv = shadowCoord.xy + offset;

            shadow += cascadedShadowMaps.SampleCmpLevelZero(
                comparisonSamplerState,
                float3(uv, cascadeIndex),
                shadowCoord.z
            ).x;
        }
    }

    shadow /= 9.0; // 3x3 = 9 サンプル
    return shadow;
}

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 sampledColor = colorMap.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord);
    float3 color = sampledColor.rgb;
    float alpha = sampledColor.a;
    
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
    if (cascadeIndex > -1)
    {
        float4 positionLightSpace = mul(positionWorldSpace, cascadedMatrices[cascadeIndex]);
        positionLightSpace /= positionLightSpace.w;

        positionLightSpace.x = positionLightSpace.x * 0.5 + 0.5;
        positionLightSpace.y = positionLightSpace.y * -0.5 + 0.5;

        float3 shadowCoord;
        shadowCoord.xy = positionLightSpace.xy;
        shadowCoord.z = positionLightSpace.z - shadowDepthBias;

        // ★ PCF 版
        float3 shadowFactor = ComputeShadowPCF(shadowCoord, cascadeIndex);

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

        float3 shadow = lerp(shadowColor, color, shadowFactor) * layerColor;
        color = lerp(color, shadow, 0.5);
    }
    
#if 0
	// Tone mapping : HDR -> SDR
    const float exposure = 1.2;
    color = 1 - exp(-color * exposure);
#endif


#if 0
	// Gamma process
    const float GAMMA = 2.2;
    color = pow(color, 1.0 / GAMMA);
#endif
	
    return float4(color, alpha);
}