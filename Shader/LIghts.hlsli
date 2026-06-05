#ifndef LIGHTS_HLSLI
#define LIGHTS_HLSLI

//点光源
struct PointLight
{
    uint enable;
    float3 position;
    float4 color;
    float range;
    float3 dummy;
};
//スポットライト
struct SpotLight
{
    uint enable;
    float3 position;
    float4 direction;
    float4 color;
    float range;
    float innerCone;
    float outerCone;
    float dummy;
};

cbuffer LIGHT_CONSTANT_BUFFER : register(b4)
{
    float4 ambientColor;
    float4 directionalLightDirection;
    float4 directionalLightColor;
    PointLight pointLights[8];
    SpotLight spotLights[8];
}

// -----------------------------------------------------------------
// 点光源の計算関数
// -----------------------------------------------------------------
void CalcPointLights(
    float3 worldPosition, // ピクセルのワールド座標
    float3 N, // 正規化された法線ベクトル
    float3 V, // 正規化された視線（カメラ）ベクトル
    //float3 f0, // 垂直反射率（非金属は0.04、金属はベースカラー）
    //float f90, // グラージング角の反射率（通常は 1.0）
    //float3 cDiff, // ディフューズカラー（金属の場合は 0 になる）
    //float alphaRoughness, // ラフネスの2乗値（GGX微小面分布等の計算で使用）
    out float3 outDiffuse, // 計算されたディフューズ（拡散反射）光の出力
    out float3 outSpecular) // 計算されたスペキュラ（鏡面反射）光の出力
{
    outDiffuse = 0;
    outSpecular = 0;
    int pointLightCount = 8;
    
    for (int i = 0; i < pointLightCount; i++)
    {
        if (pointLights[i].enable)
        {
            float3 LP = pointLights[i].position.xyz - worldPosition; // ライトへのベクトル
            float len = length(LP);
            
            float attenuation = 1.0 / (len * len); // 距離の2乗に反比例する減衰
            
            LP /= len; // 正規化
            const float pNoV = max(0.0, dot(N, V));
            
            float pNoL = max(0.0, dot(N, LP));
            
            if (pNoL > 0.0 || pNoV > 0.0)
            {
                const float3 H = normalize(V + LP);
                float3 pLi = float3(pointLights[i].color.xyz) * pointLights[i].color.w;
                
                const float NoH = max(0.0, dot(N, H));
                const float HoV = max(0.0, dot(H, V));
                
                outDiffuse += pLi * pNoL * /*brdf_lambertian(f0, f90, cDiff, HoV) **/ attenuation;
                outSpecular += pLi * pNoL * /*brdf_specular_ggx(f0, f90, alphaRoughness, HoV, pNoL, pNoV, NoH) **/ attenuation;
            }
        }
    }
}

// TODO: 遮蔽物を考慮するようにライトの計算を改良する必要がある
// スポットライトの計算 (position: ワールド空間の位置, diffuse: 拡散反射の色, specular: 鏡面反射の色)
void CalcSpotLights(float3 position, in out float3 diffuse, in out float3 specular)
{
    for (int i = 0; i < 8; i++)
    {
        if (spotLights[i].enable)
        {
            float3 LP = position.xyz - spotLights[i].position.xyz;
            float len = length(LP);
            if (len >= spotLights[i].range)
                continue;
            float attenuateLength = saturate(1.0f - len / spotLights[i].range);
            float attenuation = attenuateLength * attenuateLength;
            LP /= len;
            float3 spotDirection = normalize(spotLights[i].direction.xyz);
            float angle = dot(spotDirection, LP);
            float area = spotLights[i].innerCone - spotLights[i].outerCone;
            attenuation *= saturate(1.0f - (spotLights[i].innerCone - angle) / area);
            
            diffuse += spotLights[i].color.rgb * attenuation;
            specular += spotLights[i].color.rgb * attenuation;
        }
    }
}

#endif // LIGHTS_HLSLI