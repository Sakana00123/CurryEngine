#include "fullscreen_quad.hlsli"
#include "Sampler.hlsli"
#include "Constants.hlsli"

Texture2D textureMap : register(t0);
Texture2D<float> depthMap : register(t1); // 深度マップはfloatとして明示

// --- 深度エッジ検出関数 ---
float CalculateDepthEdge(float2 uv, float thickness, float threshold, Texture2D<float> depthTex, SamplerState smp)
{
    float d = depthTex.SampleLevel(smp, uv, 0).r;

    // 背景（無限遠）ならエッジ判定を行わない
    if (d >= 1.0f)
        return 0.0f;

    float dUp = depthTex.SampleLevel(smp, uv + float2(0, thickness), 0).r;
    float dDown = depthTex.SampleLevel(smp, uv + float2(0, -thickness), 0).r;
    float dLeft = depthTex.SampleLevel(smp, uv + float2(-thickness, 0), 0).r;
    float dRight = depthTex.SampleLevel(smp, uv + float2(thickness, 0), 0).r;

    float edge = abs(d - dUp) + abs(d - dDown) + abs(d - dLeft) + abs(d - dRight);

    return step(threshold, edge);
}

float CalculateColorEdge(float2 uv, float thickness, Texture2D colorMap, SamplerState samp)
{
    float3 c = colorMap.Sample(samp, uv).rgb;
    float3 cx = colorMap.Sample(samp, uv + float2(thickness, 0)).rgb;
    float3 cy = colorMap.Sample(samp, uv + float2(0, thickness)).rgb;

    float dx = length(cx - c);
    float dy = length(cy - c);

    return saturate((dx + dy) * 4.0); // 強さ調整
}

// --- メイン処理 ---
float4 main(VS_OUT pin) : SV_TARGET
{
    float2 uv = pin.texcoord;
    float pixelSize = 300;
    float4 col = textureMap.Sample(samplerStates[LINEAR_BORDER_BLACK], uv);
    return col;
    // 3. 深度アウトラインの適用
    float outlineThickness = 1.0 / pixelSize;
    //float outlineThreshold = 0.01;
    float outlineThreshold = 0.003;

// 深度アウトライン
    float depthEdge = CalculateDepthEdge(uv, outlineThickness, outlineThreshold, depthMap, samplerStates[POINT]);

// 色アウトライン
    float colorEdge = CalculateColorEdge(uv, outlineThickness, textureMap, samplerStates[POINT]);

// アウトラインカラー（黒 + 好きな色）
    float3 outlineColorDepth = float3(0.0, 0.0, 0.0); // 深度アウトライン
    float3 outlineColorColor = float3(0.0, 0.0, 0.0); // 色アウトライン（例：緑）

// 合成（強さ調整可能）
    float depthFactor = depthEdge * 0;
    float colorFactor = colorEdge * 0; // 色アウトラインの強さ

    float3 combinedOutline =
    outlineColorDepth * depthFactor;
    +
    outlineColorColor * colorFactor;

// 元の色にアウトラインをブレンド
    col.rgb = lerp(col.rgb, combinedOutline, saturate(depthFactor + colorFactor));

   
    return col;
}