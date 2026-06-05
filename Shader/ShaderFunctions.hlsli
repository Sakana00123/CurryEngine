#ifndef __SHADING_FUNCTIONS_HLSLI__
#define __SHADING_FUNCTIONS_HLSLI__

//	ガンマ係数
static const float GammaFactor = 2.2f;

//	円周率
static const float PI = 3.141592654f;

//--------------------------------------------
//	ランバート拡散反射計算関数
//--------------------------------------------
// N:法線(正規化済み)
// L:入射ベクトル(正規化済み)
// C:入射光(色・強さ)
// K:反射率
float3 CalcLambert(float3 N, float3 L, float3 C, float3 K)
{
    float power = saturate(dot(N, -L));
    return C * power * K;
}

//--------------------------------------------
// フォン鏡面反射計算関数
//--------------------------------------------
// N:法線(正規化済み)
// L:入射ベクトル(正規化済み)
// E:視線ベクトル(正規化済み)
// C:入射光(色・強さ)
// K:反射率
// Power:鏡面反射の強さ
float3 CalcPhongSpecular(float3 N, float3 L, float3 E, float3 C, float3 K, float Power = 128)
{
    float3 R = reflect(L, N);
    float power = max(dot(-E, R), 0);
    power = pow(power, Power);
    return C * power * K;
}

//--------------------------------------------
// ハーフランバート計算関数
//--------------------------------------------
// N:法線(正規化済み)
// L:入射ベクトル(正規化済み)
// C:入射光(色・強さ)
// K:反射率
float3 ClacHalfLambert(float3 N, float3 L, float3 C, float3 K)
{
    float D = saturate(dot(N, -L) * 0.5f + 0.5f);
    return C * D * K;
}

//--------------------------------------------
// リムライト計算関数
//--------------------------------------------
// N:法線(正規化済み)
// E:視線ベクトル(正規化済み)
// L:入射ベクトル(正規化済み)
// C:入射光(色・強さ)
// RimPower:リムライトの強さ
float3 CalcRimLight(float3 N, float3 E, float3 L, float3 C, float RimPower = 3.0f)
{
    float rim = 1.0f - saturate(dot(N, -E));
    return C * pow(rim, RimPower) * saturate(dot(L, -E));
}

//--------------------------------------------
// ランプシェーディング
//--------------------------------------------
// tex:ランプシェーディング用テクスチャ
// samp:ランプシェーディング用サンプラステート
// N:法線(正規化済み)
// L:入射ベクトル(正規化済み)
// C:入射光(色・強さ)
// K:反射率
float3 CalcRampShading(Texture2D tex, SamplerState samp, float3 N, float3 L, float3 C, float3 K)
{
    float D = saturate(dot(N, -L) * 0.5f + 0.5f);
    float Ramp = tex.Sample(samp, float2(D, 0.5f)).r;
    return C * Ramp * K.rgb;
}

//--------------------------------------------
// 球面環境マッピング
//--------------------------------------------
// tex:環境マップ用テクスチャ
// samp:環境マップ用サンプラステート
// color:元の色
// N:法線(正規化済み)
// E:視線ベクトル(正規化済み)
// value:環境マップの影響度(0.0～1.0)
float3 CalcSphereEnvironment(Texture2D tex, SamplerState samp, in float3 color, float3 N, float3 E, float value)
{
    float3 R = reflect(E, N);
    float2 texcoord = R.xy * 0.5f + 0.5f;
    return lerp(color.rgb, tex.Sample(samp, texcoord).rgb, value);
}

//--------------------------------------------
// ヘミスフィアライト計算
//--------------------------------------------
// normal:法線(正規化済み)
// up:上方向ベクトル(正規化済み)
// skyColor:空の色
// groundColor:地面の色
// hemisphereWeight:ライトの強さ(x:ライトの強さ、y:未使用、z:未使用、w:未使用)
float3 CalcHemiSphereLight(float3 normal, float3 up, float3 skyColor, float3 groundColor, float4 hemisphereWeight)
{
    float factor = dot(normal, up) * 0.5f + 0.5f;
    return lerp(groundColor, skyColor, factor) * hemisphereWeight.x;
}

//--------------------------------------------
// フォグ計算
//--------------------------------------------
// color: 現在のピクセル色
// fogColor: フォグ色
// fogRange: フォグの開始距離と終了距離(x:開始距離、y:終了距離)
// eyeLength: 視点からピクセルまでの距離
float4 CalcFog(in float4 color, float4 fogColor, float2 fogRange, float eyeLength)
{
    float fogAlpha = saturate((eyeLength - fogRange.x) / (fogRange.y - fogRange.x));
    return lerp(color, fogColor, fogAlpha);
}

//--------------------------------------------
//	スカイボックスサンプリング
//--------------------------------------------
// tex:スカイボックス用テクスチャ
// samp:スカイボックス用サンプラステート
// direction:サンプリングする方向ベクトル(正規化済み)
float4 SampleSkybox(Texture2D tex, SamplerState samp, float3 direction)
{
    static const float PI = 3.14159265f;

    float latitude = (1.0f / (2.0f * PI)) * atan2(direction.z, direction.x) + 0.5f;
    float longitude = (1.0f / PI) * atan2(direction.y, length(direction.xz)) + 0.5f;
    return tex.SampleLevel(samp, float2(1.0f - saturate(latitude), 1.0f - saturate(longitude)), 0);
}

#endif	//	__SHADING_FUNCTIONS_HLSLI__
