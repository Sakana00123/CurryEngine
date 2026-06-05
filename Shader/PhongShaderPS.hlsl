#include "gltf_model.hlsli"
#include "bidirectional_reflectance_distribution_function.hlsli"

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

float4 main(VS_OUT pin) : SV_TARGET
{
    const float GAMMA = 2.2;
    
    const MaterialConstants m = materials[material];
    
    float4 baseColorFactor = m.pbrMetallicRoughness.baseColorFactor;
    const int baseColorTexture = m.pbrMetallicRoughness.baseColorTexture.index;
    if (baseColorTexture > -1)
    {
        float4 sampled = materialTextures[BASECOLOR_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        baseColorFactor *= sampled;
    }
    
    float3 emissiveFactor = m.emissiveFactor;
    const int emissiveTexture = m.emissiveTexture.index;
    if (emissiveTexture > -1)
    {
        float4 sampled = materialTextures[EMISSIVE_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        emissiveFactor *= sampled.rgb;
    }
    
    float roughnessFactor = m.pbrMetallicRoughness.roughnessFactor;
    float metallicFactor = m.pbrMetallicRoughness.metallicFactor;
    const int metallicRoughnessTexture = m.pbrMetallicRoughness.metalicRoughnessTexture.index;
    if (metallicRoughnessTexture > -1)
    {
        float4 sampled = materialTextures[METALLIC_ROUGHNESS_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        roughnessFactor *= sampled.g;
        metallicFactor *= sampled.b;
    }
    
    float occlusionFactor = 1.0;
    const int occlusionTexture = m.occlusionTexture.index;
    if (occlusionTexture > -1)
    {
        float4 sampled = materialTextures[OCCLUSION_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        occlusionFactor *= sampled.r;
    }
    const float occlusionStrength = m.occlusionTexture.strength;
    
    const float f0 = lerp(0.04, baseColorFactor.rgb, metallicFactor);
    const float f90 = 1.0;
    const float alphaRoughness = roughnessFactor * roughnessFactor;
    const float3 c_diff = lerp(baseColorFactor.rgb, 0.0, metallicFactor);
    
    const float3 P = pin.world_position.xyz;
    const float3 V = normalize(cameraPositon.xyz - pin.world_position.xyz);
    
    float3 N = normalize(pin.world_normal.xyz);
    float3 T = has_tangent ? normalize(pin.world_tangent.xyz) : float3(1, 0, 0);
    float sigma = has_tangent ? pin.world_tangent.w : 1.0;
    T = normalize(T - N * dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);
    
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
    
    // Loop for shading process for each light
    float3 L = normalize(-directionalLightDirection.xyz);
    float3 Li = directionalLightColor.rgb; // Radiance of the light
    const float NoL = max(0.0, dot(N, L));
    const float NoV = max(0.0, dot(N, V));
    if (NoL > 0.0 || NoV > 0.0)
    {
        const float3 R = reflect(-L, N);
        const float3 H = normalize(V + L);
        
        const float NoH = max(0.0, dot(N, H));
        const float HoV = max(0.0, dot(H, V));
        
        diffuse += Li * NoL * brdf_lambertian(f0, f90, c_diff, HoV);
        specular += Li * NoL * brdf_specular_ggx(f0, f90, alphaRoughness, HoV, NoL, NoV, NoH);
    }
    
    //UNIT39
#if 1
    diffuse += ibl_radiance_lambertian(N, V, roughnessFactor, c_diff, f0);
    //specular += ibl_radiance_ggx(N, V, roughnessFactor, f0);
#endif
    
    float3 emissive = emissiveFactor;
    diffuse = lerp(diffuse, diffuse * occlusionFactor, occlusionStrength);
    specular = lerp(specular, specular * occlusionFactor, occlusionStrength);
    
    
    float3 E = normalize(cameraPositon.xyz - pin.world_position.xyz);
    
    float3 R = reflect(-L, N);
    
    specular = dot(R, E);
    specular = (pow(specular, 128));
    
    float3 Lo = specular + diffuse + emissive;

    return float4(Lo, baseColorFactor.a);
    
    return float4(1.0f, 0.0f, 1.0f, 1.0f);
}