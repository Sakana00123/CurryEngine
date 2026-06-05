#include "GltfModelGBuffer.hlsli"

// https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#reference-textureinfo
struct TextureInfo
{
    int index; // required
    int texcoord; // The set index of texture's TEXCOORD attribute used for texture coordinate mapping.
};

// https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#reference-material-normaltextureinfo
struct NormalTextureInfo
{
    int index; // required
    int texcoord; // The set index of texture's TEXCOORD attribute used for texture coordinate mapping.
    float scale; // scaledNormal = normalize((<sampled normal texture value> * 2.0 - 1.0) * vec3(<normal scale>, <normal scale>, 1.0))
};

// https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#reference-material-occlusiontextureinfo
struct OcclusionTextureInfo
{
    int index; // required
    int texcoord; // The set index of texture's TEXCOORD attribute used for texture coordinate mapping.
    float strength; // A scalar parameter controlling the amount of occlusion applied. A value of `0.0` means no occlusion. A value of `1.0` means full occlusion. This value affects the final occlusion value as: `1.0 + strength * (<sampled occlusion texture value> - 1.0)`.
};

// https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#reference-material-pbrmetallicroughness
struct PbrMetallicRoughness
{
    float4 baseColorFactor; // len = 4; default [1,1,1,1] The material's base color factor.
    TextureInfo baseColorTexture; // The base color texture.
    float metallicFactor; // default 1.0; The metalness of the material.
    float roughnessFactor; // default 1.0; The roughness of the material.
    TextureInfo metallicRoughnessTexture; // The metallic-roughness texture.
};

struct MaterialConstants
{
    float3 emissiveFactor; // len = 3; default [0,0,0] The material's emissive color factor.
    int alphaMode; // "OPAQUE" : 0, "MASK" : 1, "BLEND" : 2
    float alphaCutoff; // default 0.5; The cutoff threshold when in `MASK` mode.
    int doubleSided; // default false; Specifies whether the material is double sided.
    
    PbrMetallicRoughness pbrMetallicRoughness;
    
    NormalTextureInfo normalTexture; // The normal map texture.
    OcclusionTextureInfo occlusionTexture; // The occlusion map texture.
    TextureInfo emissiveTexture; // The emissive map texture.
};
StructuredBuffer<MaterialConstants> materials : register(t0);

#define BASECOLOR_TEXTURE 0
#define METALLICROUGHNESS_TEXTURE 1
#define NORMAL_TEXTURE 2
#define EMISSIVE_TEXTURE 3
#define OCCLUSION_TEXTURE 4
Texture2D<float4> materialTextures[5] : register(t1);

#include "Sampler.hlsli"

PSGBufferOut main(VS_OUT pin, bool isFrontFace : SV_IsFrontFace)
{
    MaterialConstants m = materials[material];
    
    // ベースカラー
    float4 baseColor = m.pbrMetallicRoughness.baseColorFactor;
    if (m.pbrMetallicRoughness.baseColorTexture.index >= 0)
    {
        baseColor *= materialTextures[BASECOLOR_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
    }
    
    // 自己発光色
    float3 emissive = m.emissiveFactor;
    if (m.emissiveTexture.index > -1)
    {
        emissive *= materialTextures[EMISSIVE_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord).rgb;
    }
    
    // 法線取得
    float3 N = normalize(pin.world_normal.xyz);
    float3 T = has_tangent ? normalize(pin.world_tangent.xyz) : float3(1, 0, 0);
    float sigma = has_tangent ? pin.world_tangent.w : 1.0; // ミップマップレベル
    T = normalize(T - N * dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);
    // 裏面描画の場合は法線を反転
    if (!isFrontFace)
    {
        T = -T;
        B = -B;
        N = -N;
    }
    
    // 法線マップ
    if (m.normalTexture.index > -1)
    {
        float4 sampled = materialTextures[NORMAL_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        float3 normalFactor = sampled.xyz;
        normalFactor = (normalFactor * 2.0f) - 1.0f;
        normalFactor = normalize(normalFactor * float3(m.normalTexture.scale, m.normalTexture.scale, 1.0f));
        N = normalize((normalFactor.x * T) + (normalFactor.y * B) + (normalFactor.z * N));
    }
    
    // GBufferDataに出力情報をまとめる
    GBufferData data = (GBufferData) 0;
    data.baseColor = baseColor.rgb;
    data.wNormal = N * (isFrontFace ? 1.0f : -1.0f); // 裏面描画の場合は法線を反転
    data.wPosition = pin.world_position.xyz; // data.depthはEncode時はいらない
    data.emissiveColor = emissive;
    data.shadingModel = shaderModelShading;
    
    // シェーディング方法
    return EncodeGBuffer(data, viewProjection);
}