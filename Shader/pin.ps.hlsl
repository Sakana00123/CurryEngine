#include "gltf_model.hlsli"
#include "bidirectional_reflectance_distribution_function.hlsli"
#include "Lights.hlsli"

struct TextureInfo
{
    int index;
    int texcoord;
};
struct NormalTextureInfo
{
    int index;
    int texcoord;
    float scale;
};
struct OcclusionTextureInfo
{
    int index;
    int texcoord;
    float strength;
};
struct PBRMetallicRoughness
{
    float4 baseColorFactor;
    TextureInfo baseColorTexture;
    float metallicFactor;
    float roughnessFactor;
    TextureInfo metalicRoughnessTexture;
};
struct MaterialConstants
{
    float3 emissiveFactor;
    int alphaMode; //  "OPAQUE" : 0, "MASK" : 1, "BLEND" : 2
    float alphaCutoff;
    bool doubleSided;

    PBRMetallicRoughness pbrMetallicRoughness;

    NormalTextureInfo normalTexture;
    OcclusionTextureInfo occlusionTexture;
    TextureInfo emissiveTexture;
};
StructuredBuffer<MaterialConstants> materials : register(t0);

#define BASECOLOR_TEXTURE 0
#define METALLIC_ROUGHNESS_TEXTURE 1
#define NORMAL_TEXTURE 2
#define EMISSIVE_TEXTURE 3
#define OCCLUSION_TEXTURE 4
Texture2D<float4> materialTextures[5] : register(t1);

cbuffer PIN_CBUFFER : register(b6)
{
    float3 pin_color;    
};

float4 main(VS_OUT pin, bool isFrontFace : SV_IsFrontFace) : SV_TARGET
{
    const float GAMMA = 2.2;
    const MaterialConstants m = materials[material];

    // -----------------------------------------------------------------
    // ベースカラーの計算
    // -----------------------------------------------------------------
    float4 basecolorFactor = m.pbrMetallicRoughness.baseColorFactor;
    const int basecolorTexture = m.pbrMetallicRoughness.baseColorTexture.index;

    if (basecolorTexture > -1)
    {
        float4 sampled = materialTextures[BASECOLOR_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
        // テクスチャの色をリニア空間に変換
        sampled.rgb = pow(sampled.rgb, GAMMA);
        sampled.rgb = pin_color;
        basecolorFactor *= sampled;
    }

    if (m.alphaMode == 0 /*OPAQUE*/)
    {
        basecolorFactor.a = 1.0;
    }
    float3 ambientColor = basecolorFactor.rgb * 0.3; // 環境光の仮の値（必要に応じて環境マップなどを使用してください）
    // -----------------------------------------------------------------
    // エミッシブ（自己発光）の計算
    // -----------------------------------------------------------------
    float3 emmisiveFactor = m.emissiveFactor;
    const int emissiveTexture = m.emissiveTexture.index;
    if (emissiveTexture > -1)
    {
        float4 sampled = materialTextures[EMISSIVE_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        emmisiveFactor *= sampled.rgb;
    }

    // -----------------------------------------------------------------
    // メタリック＆ラフネスの計算
    // -----------------------------------------------------------------
    float roughnessFactor = m.pbrMetallicRoughness.roughnessFactor;
    float metallicFactor = m.pbrMetallicRoughness.metallicFactor;
    const int metallicRoughnessTexture = m.pbrMetallicRoughness.metalicRoughnessTexture.index;
    
    if (metallicRoughnessTexture > -1)
    {
        float4 sampled = materialTextures[METALLIC_ROUGHNESS_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        // glTFの標準仕様では、Gチャンネルがラフネス、Bチャンネルがメタリック
        roughnessFactor *= sampled.g;
        metallicFactor *= sampled.b;
    }

    // -----------------------------------------------------------------
    // オクルージョン（環境光遮蔽）の計算
    // -----------------------------------------------------------------
    float occlusionFactor = 1.0;
    const int occlusionTexture = m.occlusionTexture.index;
    if (occlusionTexture > -1)
    {
        float4 sampled = materialTextures[OCCLUSION_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        occlusionFactor *= sampled.r;
    }
    const float occlusionStrength = m.occlusionTexture.strength;

    // -----------------------------------------------------------------
    // PBR用パラメータの準備
    // -----------------------------------------------------------------
    const float3 f0 = lerp(0.04, basecolorFactor.rgb, metallicFactor); // 垂直反射率(非金属は0.04、金属はベースカラー)
    const float3 f90 = 1.0;
    const float alphaRoughness = roughnessFactor * roughnessFactor;
    const float3 cDiff = lerp(basecolorFactor.rgb, 0.0, metallicFactor); // ディフューズカラー（金属は0になる）

    const float3 P = pin.world_position.xyz;
    const float3 V = normalize(cameraPositon.xyz - pin.world_position.xyz); // 視線ベクトル

    // -----------------------------------------------------------------
    // 法線の計算（ノーマルマップ適用）
    // -----------------------------------------------------------------
    float3 N = normalize(pin.world_normal.xyz);
    float3 T = has_tangent ? normalize(pin.world_tangent.xyz) : float3(1, 0, 0);
    float sigma = has_tangent ? pin.world_tangent.w : 1.0;
    T = normalize(T - N * dot(N, T)); // グラム・シュミットの直交化
    float3 B = normalize(cross(N, T) * sigma); // 従法線

    // 背面ポリゴンの場合は接ベクトル空間を反転
    if (isFrontFace == false)
    {
        T = -T;
        B = -B;
        N = -N;
    }
    
    const int normalTexture = m.normalTexture.index;
    if (normalTexture > -1)
    {
        float4 sampled = materialTextures[NORMAL_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        float3 normalFactor = sampled.xyz;
        normalFactor = (normalFactor * 2.0) - 1.0;
        normalFactor = normalize(normalFactor * float3(m.normalTexture.scale, m.normalTexture.scale, 1.0));
        N = normalize((normalFactor.x * T) + (normalFactor.y * B) + (normalFactor.z * N));
    }
    
    float3 diffuse = 0;
    float3 specular = 0;

    // -----------------------------------------------------------------
    // 平行光源（Directional Light）の処理
    // -----------------------------------------------------------------
    {
        float3 L = normalize(-directionalLightDirection.xyz);
        float lightPower = max(directionalLightDirection.w * 0.5f, 2.0f);
        float3 Li = directionalLightColor.rgb * lightPower;
        
        float NoL = max(0.0, dot(N, L));
        const float NoV = max(0.0, dot(N, V));
        
        if (NoL > 0.0 || NoV > 0.0)
        {
            const float3 R = reflect(-L, N);
            const float3 H = normalize(V + L);

            const float NoH = max(0.0, dot(N, H));
            const float HoV = max(0.0, dot(H, V));

            diffuse += Li * NoL * brdf_lambertian(f0, f90, cDiff, NoL);
            specular += Li * NoL * brdf_specular_ggx(f0, f90, alphaRoughness, HoV, NoL, NoV, NoH);
        }
    }
    
    // -----------------------------------------------------------------
    // 点光源（Point Light）の処理
    // -----------------------------------------------------------------
    float3 pointDiffuse = 0;
    float3 pointSpecular = 0;
    CalcPointLights(P, N, V, /*f0, f90, cDiff, alphaRoughness,*/pointDiffuse, pointSpecular);
    
    
        // -----------------------------------------------------------------
    // スポットライト（Spot Light）の処理
    // -----------------------------------------------------------------
    float3 spotDiffuse = 0;
    float3 spotSpecular = 0;
    
    for (int i = 0; i < 8; i++)
    {
        if (spotLights[i].enable)
        {
            float3 LP = spotLights[i].position.xyz - P;
            float len = length(LP);
            
            // 影響範囲外ならスキップ
            if (len >= spotLights[i].range)
            {
                continue;
            }
            
            // 距離による減衰（Falloff）
            float attenuateLength = saturate(1.0f - (len / spotLights[i].range));
            float distanceAttenuation = attenuateLength * attenuateLength;
            
            LP /= len; // ピクセルからライトへのベクトルを正規化
            
            // 角度による減衰（Spot Cone）
            float3 spotDirection = normalize(spotLights[i].direction.xyz);
            // ライトの向きと、ピクセルへの向かうベクトルの内積（-LP）
            float angle = dot(spotDirection, -LP);
            
            float area = spotLights[i].innerCone - spotLights[i].outerCone;
            // 外側コーンと内側コーンの間の滑らかな減衰
            float spotAttenuation = saturate((angle - spotLights[i].outerCone) / area);
            
            // 距離減衰 × 角度減衰
            float attenuation = distanceAttenuation * spotAttenuation;
            
            if (attenuation > 0.0f)
            {
                float pNoV = max(0.0, dot(N, V));
                float pNoL = max(0.0, dot(N, LP));
                
                if (pNoL > 0.0 || pNoV > 0.0)
                {
                    const float3 H = normalize(V + LP);
                    float3 pLi = float3(spotLights[i].color.xyz) * spotLights[i].color.w;
                    
                    const float NoH = max(0.0, dot(N, H));
                    const float HoV = max(0.0, dot(H, V));
                    
                    // ※ PBR計算（PointLightで引数を消している場合は適宜合わせてください）
                    spotDiffuse += pLi * pNoL * /*brdf_lambertian(f0, f90, cDiff, HoV) * */attenuation;
                    spotSpecular += pLi * pNoL * /*brdf_specular_ggx(f0, f90, alphaRoughness, HoV, pNoL, pNoV, NoH) * */attenuation;
                }
            }
        }
    }
    
    // ライト結果を合成
    float3 totalDiffuse = diffuse + pointDiffuse + spotDiffuse;
    float3 totalSpecular = specular + pointSpecular + spotSpecular;
    
    float3 emmisive = emmisiveFactor;
    
    // オクルージョンマップの適用
    diffuse = lerp(totalDiffuse, totalDiffuse * occlusionFactor, occlusionStrength);
    specular = lerp(totalSpecular, totalSpecular * occlusionFactor, occlusionStrength);

    // -----------------------------------------------------------------
    // 最終カラーの合成
    // -----------------------------------------------------------------
    float3 finalColor = diffuse + specular + emmisive + ambientColor;

    // アルファテスト
    float alpha_cutoff = 0.5; // 必要に応じて m.alphaCutoff を使用
    if (basecolorFactor.a < alpha_cutoff)
    {
        discard;
    }

    // ガンマ補正をかけて出力
    finalColor = pow(saturate(finalColor), 1.0 / GAMMA);
    return float4(finalColor, basecolorFactor.a);
}