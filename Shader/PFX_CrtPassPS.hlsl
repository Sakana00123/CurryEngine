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

// --- 既存のヘルパー関数群 (省略なし) ---
float scanline(float2 uv, float lines, float speed)
{
    return sin(uv.y * lines - (-time) * speed);
}
float2 pixelate(float2 uv, float dotCount)
{
    return floor(uv * dotCount) / dotCount;
}
float2 crt_coords(float2 uv, float bend)
{
    if (bend <= 0.0)
        return uv; // 0除算回避
    uv -= 0.5;
    uv *= 2.0;
    uv.x *= 1.0 + pow(abs(uv.y) / bend, 2.0);
    uv.y *= 1.0 + pow(abs(uv.x) / bend, 2.0);
    uv *= 0.5;
    return uv + 0.5;
}
float random2(float2 uv)
{
    return frac(sin(dot(uv, float2(15.5151, 42.2561))) * 12341.14122 * sin(time * 0.03));
}
float noise(float2 uv)
{
    float2 i = floor(uv);
    float2 f = frac(uv);
    float a = random2(i);
    float b = random2(i + float2(1, 0));
    float c = random2(i + float2(0, 1));
    float d = random2(i + float2(1, 1));
    float2 u = smoothstep(0.0, 1.0, f);
    return lerp(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}
float vignette(float2 uv, float size, float smoothness, float edgeRounding)
{
    uv -= 0.5;
    uv *= size;
    float amount = sqrt(pow(abs(uv.x), edgeRounding) + pow(abs(uv.y), edgeRounding));
    amount = 1.0 - amount;
    return smoothstep(0.0, smoothness, amount);
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
    float pixelSize = 512;
    // 1. ピクセル化とCRT座標計算
    float2 pix_uv = pixelate(uv, pixelSize);
    //float2 crt_uv = crt_coords(pix_uv, 4.0); // bendを少し入れるとCRT感が出ます

    // 2. シーンカラーのサンプリング（色収差あり）
    float4 col;
    col.r = textureMap.Sample(samplerStates[LINEAR_BORDER_BLACK], pix_uv + float2(0.0, 0.001)).r;
    col.g = textureMap.Sample(samplerStates[LINEAR_BORDER_BLACK], pix_uv).g;
    col.b = textureMap.Sample(samplerStates[LINEAR_BORDER_BLACK], pix_uv + float2(0.0, -0.001)).b;
    col.a = 1.0;

//   // 3. 深度アウトラインの適用
//   float outlineThickness = 1.0 / pixelSize;
//   //float outlineThreshold = 0.01;
//   float outlineThreshold = 0.003;
//
/// 深度アウトライン
//   float depthEdge = CalculateDepthEdge(pix_uv, outlineThickness, outlineThreshold, depthMap, samplerStates[POINT]);
//
/// 色アウトライン
//   float colorEdge = CalculateColorEdge(pix_uv, outlineThickness, textureMap, samplerStates[POINT]);
//
/// アウトラインカラー（黒 + 好きな色）
//   float3 outlineColorDepth = float3(0.0, 0.0, 0.0); // 深度アウトライン
//   float3 outlineColorColor = float3(0.0, 0.0, 0.0); // 色アウトライン（例：緑）
//
/// 合成（強さ調整可能）
//   float depthFactor = depthEdge;
//   float colorFactor = colorEdge*0.5; // 色アウトラインの強さ
//
//   float3 combinedOutline =
//   outlineColorDepth * depthFactor;
//   +
//   outlineColorColor * colorFactor;
//
/// 元の色にアウトラインをブレンド
//   col.rgb = lerp(col.rgb, combinedOutline, saturate(depthFactor + colorFactor));

    //ポスタライズ
    float levels = 20;
    float3 c = col.rgb * levels;
    col.rgb = floor(c + 0.5) / levels;

    //return float4(c, col.a);

    // 4. スキャンラインの合成
    float scan1 = scanline(uv, 150.0, -10.0);
    float scan2 = scanline(uv, 20.0, -3.0);
    col = lerp(col, float4(scan1 + scan2, scan1 + scan2, scan1 + scan2, 1.0), 0.025);

    // 5. ビネット
   // col *= vignette(uv, 1.9, 0.6, 10.0);

    // 6. ノイズ
    float n = noise(uv * 75.0);
    col = lerp(col, float4(n, n, n, 1.0), 0.01);


    return col;
}