#include "Decal.hlsli"
#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define LINEAR_BORDER_BLACK 3
#define LINEAR_BORDER_WHITE 4

Texture2D decalTexture : register(t0);
Texture2D<float> sceneDepthTexture : register(t16);

SamplerState samplerStates[5] : register(s0);
// ピクセルからworldpositionを求める
float4 GetWorldPosFromPixelPos(float2 texcoord, float2 dimension, float depth)
{
    float4 ndc = float4(2.0 * texcoord.x - 1.0, 1.0 - 2.0 * texcoord.y, depth, 1.0);

    float4 position = mul(ndc, inverseViewProjection);
    position /= position.w;

    //position = mul(position, decalInverseProjection);
    //position /= position.w;

    return position;
}

// 
float3 GetNormalizeVectorAB(float3 worldPosA, float3 worldPosB)
{
    float3 AB = normalize(worldPosB - worldPosA);
    return AB;
}



float4 main(float4 svPosition : SV_POSITION) : SV_TARGET
{
    float2 dimension;
    sceneDepthTexture.GetDimensions(dimension.x, dimension.y);

    float2 texcoord = svPosition.xy / dimension;
    float depth = sceneDepthTexture.Sample(samplerStates[LINEAR_BORDER_WHITE], texcoord);

    float4 ndc = float4(2.0 * texcoord.x - 1.0, 1.0 - 2.0 * texcoord.y, depth, 1.0);

    // これworldPosition
    float4 position = mul(ndc, inverseViewProjection);
    position /= position.w;

    // その場所
    position = mul(position, decalInverseProjection);
    position /= position.w;
    float2 decalTexcoord = float2(position.x * +0.5f + 0.5f, position.y * -0.5f + 0.5f);
 
    //return float4(decalTexcoord.xy, 0, 1);
#if 0

    float4 color = decalTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], decalTexcoord);
    clip(color.a - 0.5);
    color.a *= alpha;
    return float4(color.rgb, alpha);

#else
    // 右
    float2 rightTexcoord = float2(svPosition.x + 1, svPosition.y);
    float4 rightWorldPos = GetWorldPosFromPixelPos(rightTexcoord, dimension, depth);

    // 下
    float2 downTexcoord = float2(svPosition.x, svPosition.y + 1);
    float4 downWorldPos = GetWorldPosFromPixelPos(downTexcoord, dimension, depth);

    float3 vectorA = GetNormalizeVectorAB(position.xyz, rightWorldPos.xyz);
    float3 vectorB = GetNormalizeVectorAB(position.xyz, downWorldPos.xyz);
    float3 worldNormal = normalize(cross(vectorA, vectorB));
    float3 up = float3(1e-6f, 1, 0);
    float3 tangent = normalize(cross(worldNormal, up));

    float3 bitangent = normalize(cross(worldNormal, tangent));

    float3x3 TBN = float3x3(tangent, bitangent, worldNormal);

    float4 sampled = decalTexture.Sample(samplerStates[0], decalTexcoord);
    float3 normalFactor = (sampled.rgb * 2.0) - 1.0;

#if 1

#if 0
    // 法線を徐々に(0,0,1)に寄せる
    float3 fadedNormal = normalize(lerp(normalFactor, float3(0, 0, 1), fade));

    float dotF = saturate(dot(fadedNormal, float3(0, 0, 1)));
#else
    //float dotF = saturate(dot(normalFactor, float3(0, 0, 1)));

    float baseDot = saturate(dot(normalFactor, float3(0, 0, 1)));

    float dotF = lerp(baseDot, 1.0, fade);

    dotF = max(dotF, 0.15);


#endif
#else
    // 接空間からワールド空間
    float3 normalWS = normalize(mul(normalFactor, TBN));
    //float3 lightColor = float3(1.0f, 1.0f, 1.0f);

    float dotF = saturate(dot(normalWS, -lightDirection.xyz));
#endif
    //if (dotF > 0.4)
    //    dotF = 1.0;

    //float3 litColor = lightColor * dotF;
    //float4 normal = decalTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], decalTexcoord);

    //clip(sampled.a - 0.5);

    //float3 result = float3(1, 1, 1) * litColor;

    return float4(dotF, dotF, dotF, 1.0);
#endif
}