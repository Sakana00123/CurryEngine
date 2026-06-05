#ifndef __GBUFFER_HLSLI__
#define __GBUFFER_HLSLI__

static const int shaderModelUnlit = 0; // ライティング無し
static const int shaderModelShading = 1; // シェーディング

static const int shadingModelMax = 2; // 最大数

// ピクセルシェーダーへの出力用構造体
struct PSGBufferOut
{
    float4 baseColor : SV_TARGET0;
    float4 emissiveColor : SV_TARGET1;
    float4 normalDepth : SV_TARGET2;
};

// GBuffer用データ構造体
struct GBufferData
{
    float3 baseColor; // ベースカラー
    float3 emissiveColor; // 自己発光色
    float3 wNormal; // ワールド法線
    float3 wPosition; // ワールド座標
    float depth; // 深度
    int shadingModel; // シェーディング方法を決めるId
};

// GBuffer用データ構造体の初期化
PSGBufferOut EncodeGBuffer(in GBufferData data, matrix viewProjectionMatrix)
{
    PSGBufferOut ret = (PSGBufferOut) 0;
    ret.baseColor.rgb = data.baseColor;
    ret.baseColor.a = 1.0f;
    
    ret.emissiveColor.rgb = data.emissiveColor;
    ret.emissiveColor.a = 1.0f;
    
    ret.normalDepth.rgb = data.wNormal;
    float4 position = mul(float4(data.wPosition, 1.0f), viewProjectionMatrix);
    ret.normalDepth.a = position.z / position.w; // クリップ空間のZ値を正規化
    
    ret.baseColor.a = ((float) data.shadingModel) / ((float) shadingModelMax); // シェーディング方法をアルファに格納
    return ret;
};

#endif // __GBUFFER_HLSLI__