#include "model.hlsli"
#include "bidirectional_reflectance_distribution_function.hlsli"
#include "Lights.hlsli"

// ============================================================
// マテリアルパラメータ（cbuffer）
// ============================================================
// 旧実装では StructuredBuffer<MaterialConstants> を materialIndex で引いていたが、
// AssetModel では Material クラスが直接 GPU へ SetValue() するため、
// 1 ドローコールにつき 1 マテリアルが cbuffer に積まれる形に変更。
// C++ 側 Material::SetValue() で書き込む変数名と一致させること。
// ============================================================

cbuffer MATERIAL_CONSTANT_BUFFER : register(b3)
{
    float4 baseColorFactor;      // rgba
    float  metallicFactor;
    float  roughnessFactor;
    float2 _matPad0;

    float3 emissiveFactor;
    float  _matPad1;

    float  normalScale;
    float  occlusionStrength;
    int    alphaMode;            // 0:OPAQUE  1:MASK  2:BLEND
    float  alphaCutoff;
};

// ============================================================
// テクスチャスロット（固定スロット、Material::SetTexture() の名前で紐づく）
// ============================================================

// t0 は旧実装の StructuredBuffer が使っていたが廃止。
// Material::Apply() がスロット 1〜5 にバインドする想定。
Texture2D<float4> baseColorTexture         : register(t1);
Texture2D<float4> metallicRoughnessTexture : register(t2);
Texture2D<float4> normalTexture            : register(t3);
Texture2D<float4> emissiveTexture          : register(t4);
Texture2D<float4> occlusionTexture         : register(t5);

// テクスチャが存在するかどうかのフラグ（Material 側で SetValue する）
cbuffer MATERIAL_TEXTURE_FLAGS : register(b2)
{
    bool has_baseColorTexture;
    bool has_metallicRoughnessTexture;
    bool has_normalTexture;
    bool has_emissiveTexture;
    bool has_occlusionTexture;
    bool3 _flagPad;
};

// ============================================================
// ピクセルシェーダー本体
// ============================================================

float4 main(VS_OUT pin, bool isFrontFace : SV_IsFrontFace) : SV_TARGET
{
    const float GAMMA = 2.2;

    // -----------------------------------------------------------------
    // ベースカラー
    // -----------------------------------------------------------------
    float4 baseColor = baseColorFactor;
    if (has_baseColorTexture)
    {
        float4 sampled = baseColorTexture.Sample(samplerStates[ANISOTROPIC], pin.texcoord);
        sampled.rgb    = pow(abs(sampled.rgb), GAMMA); // リニア空間へ変換
        baseColor     *= sampled;
    }

    if (alphaMode == 0 /*OPAQUE*/)
        baseColor.a = 1.0;

    // アルファテスト（MASK モード）
    if (alphaMode == 1 /*MASK*/)
        clip(baseColor.a - alphaCutoff);

    float3 ambientColor = baseColor.rgb * 0.3;

    // -----------------------------------------------------------------
    // エミッシブ
    // -----------------------------------------------------------------
    float3 emissive = 0;
    if (has_emissiveTexture)
    {
        float4 sampled = emissiveTexture.Sample(samplerStates[ANISOTROPIC], pin.texcoord);
        sampled.rgb    = pow(abs(sampled.rgb), GAMMA);
        emissive       = sampled.rgb * emissiveFactor;
    }

    // -----------------------------------------------------------------
    // メタリック・ラフネス
    // -----------------------------------------------------------------
    float metallic  = metallicFactor;
    float roughness = roughnessFactor;
    if (has_metallicRoughnessTexture)
    {
        // glTF 仕様: G チャンネル = ラフネス、B チャンネル = メタリック
        float4 sampled = metallicRoughnessTexture.Sample(samplerStates[LINEAR], pin.texcoord);
        roughness     *= sampled.g;
        metallic      *= sampled.b;
    }

    // -----------------------------------------------------------------
    // オクルージョン
    // -----------------------------------------------------------------
    float occlusion = 1.0;
    if (has_occlusionTexture)
    {
        float4 sampled = occlusionTexture.Sample(samplerStates[LINEAR], pin.texcoord);
        occlusion      = sampled.r;
    }

    // -----------------------------------------------------------------
    // PBR パラメータの準備
    // -----------------------------------------------------------------
    const float3 f0             = lerp(0.04, baseColor.rgb, metallic);
    const float3 f90            = 1.0;
    const float  alphaRoughness = roughness * roughness;
    const float3 cDiff          = lerp(baseColor.rgb, 0.0, metallic);

    const float3 P = pin.world_position.xyz;
    const float3 V = normalize(cameraPositon.xyz - P);

    // -----------------------------------------------------------------
    // 法線（ノーマルマップ適用）
    // -----------------------------------------------------------------
    float3 N = normalize(pin.world_normal.xyz);
    float  sigma = has_tangent ? pin.world_tangent.w : 1.0;
    float3 T     = has_tangent ? normalize(pin.world_tangent.xyz) : float3(1, 0, 0);
    T = normalize(T - N * dot(N, T)); // グラム・シュミット直交化
    float3 B = normalize(cross(N, T) * sigma);

    // 背面ポリゴンは接空間ベクトルを反転
    if (!isFrontFace)
    {
        T = -T;
        B = -B;
        N = -N;
    }

    if (has_normalTexture)
    {
        float3 nSampled = normalTexture.Sample(samplerStates[LINEAR], pin.texcoord).xyz;
        nSampled        = nSampled * 2.0 - 1.0;
        nSampled        = normalize(nSampled * float3(normalScale, normalScale, 1.0));
        N               = normalize(nSampled.x * T + nSampled.y * B + nSampled.z * N);
    }

    // -----------------------------------------------------------------
    // ディレクショナルライト
    // -----------------------------------------------------------------
    float3 diffuse  = 0;
    float3 specular = 0;
    {
        float3 L         = normalize(-directionalLightDirection.xyz);
        float  lightPower = max(directionalLightDirection.w * 0.5f, 2.0f);
        float3 Li        = directionalLightColor.rgb * lightPower;

        float NoL = max(0.0, dot(N, L));
        float NoV = max(0.0, dot(N, V));

        if (NoL > 0.0 || NoV > 0.0)
        {
            float3 H   = normalize(V + L);
            float  NoH = max(0.0, dot(N, H));
            float  HoV = max(0.0, dot(H, V));

            diffuse  += Li * NoL * brdf_lambertian(f0, f90, cDiff, NoL);
            specular += Li * NoL * brdf_specular_ggx(f0, f90, alphaRoughness, HoV, NoL, NoV, NoH);
        }
    }

    // -----------------------------------------------------------------
    // ポイントライト・スポットライト（旧実装から継承）
    // -----------------------------------------------------------------
    float3 pointDiffuse = 0, pointSpecular = 0;
    CalcPointLights(P, N, V, pointDiffuse, pointSpecular);

    float3 spotDiffuse = 0, spotSpecular = 0;
    for (int i = 0; i < 8; i++)
    {
        if (!spotLights[i].enable) continue;

        float3 LP  = spotLights[i].position.xyz - P;
        float  len = length(LP);
        if (len >= spotLights[i].range) continue;

        float att = saturate(1.0f - (len / spotLights[i].range));
        att       = att * att;
        LP       /= len;

        float3 spotDir      = normalize(spotLights[i].direction.xyz);
        float  angle        = dot(spotDir, -LP);
        float  area         = spotLights[i].innerCone - spotLights[i].outerCone;
        float  spotAtt      = saturate((angle - spotLights[i].outerCone) / area);
        float  attenuation  = att * spotAtt;

        if (attenuation > 0.0f)
        {
            float pNoV = max(0.0, dot(N, V));
            float pNoL = max(0.0, dot(N, LP));
            if (pNoL > 0.0 || pNoV > 0.0)
            {
                float3 H   = normalize(V + LP);
                float3 pLi = spotLights[i].color.rgb * spotLights[i].color.w;
                float  NoH = max(0.0, dot(N, H));
                float  HoV = max(0.0, dot(H, V));
                spotDiffuse  += pLi * pNoL * attenuation;
                spotSpecular += pLi * pNoL * attenuation;
            }
        }
    }

    // -----------------------------------------------------------------
    // 最終合成
    // -----------------------------------------------------------------
    float3 totalDiffuse  = diffuse  + pointDiffuse  + spotDiffuse;
    float3 totalSpecular = specular + pointSpecular + spotSpecular;

    // オクルージョン適用
    totalDiffuse  = lerp(totalDiffuse,  totalDiffuse  * occlusion, occlusionStrength);
    totalSpecular = lerp(totalSpecular, totalSpecular * occlusion, occlusionStrength);

    float3 finalColor = totalDiffuse + totalSpecular + emissive + ambientColor;

    // ガンマ補正
    finalColor = pow(saturate(finalColor), 1.0 / GAMMA);
    return float4(finalColor, baseColor.a);
}
