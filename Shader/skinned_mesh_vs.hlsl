#include "skinned_mesh.hlsli"

VS_OUT main(VS_IN vin)
{
    vin.normal.w = 0;
    float sigma = vin.tangent.w;
    vin.tangent.w = 0;
    
    float4 blendedPosition = { 0, 0, 0, 1 };
    float4 blendedNormal = { 0, 0, 0, 0 };
    float4 blendedTangent = { 0, 0, 0, 0 };
    for (int boneIndex = 0; boneIndex < 4; ++boneIndex)
    {
        blendedPosition += vin.bone_weights[boneIndex]
            * mul(vin.position, bone_transforms[vin.bone_indices[boneIndex]]);
        blendedNormal += vin.bone_weights[boneIndex]
            * mul(vin.normal, bone_transforms[vin.bone_indices[boneIndex]]);
        blendedTangent += vin.bone_weights[boneIndex] *
            mul(vin.tangent, bone_transforms[vin.bone_indices[boneIndex]]);
    }
    vin.position = float4(blendedPosition.xyz, 1.0f);
    vin.normal = float4(blendedNormal.xyz, 0.0f);
    vin.tangent = float4(blendedTangent.xyz, 0.0f);
    
    VS_OUT vout;
    vout.position = mul(vin.position, mul(world, viewProjection));
    
    vout.world_position = mul(vin.position, world);
    vout.world_normal = normalize(mul(vin.normal, world));
    vout.world_tangent = normalize(mul(vin.tangent, world));
    vout.world_tangent.w = sigma;
    
    vout.texcoord = vin.texcoord;
    #if 1
    vout.color = material_color;
    #else
    vout.color = 0;
    const float4 boneColors[4] =
    {
        { 1, 0, 0, 1 },
        { 0, 1, 0, 1 },
        { 0, 0, 1, 1 },
        { 1, 1, 1, 1 },
    };
    for (int boneIndex = 0; boneIndex < 4; ++boneIndex)
    {
        vout.color += boneColors[vin.bone_indices[boneIndex] % 4]
            * vin.bone_weights[boneIndex];
    }
#endif
    return vout;
}