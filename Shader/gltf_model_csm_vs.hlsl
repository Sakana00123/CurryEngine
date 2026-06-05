#include "gltf_model.hlsli"

struct csm_constants
{
    row_major float4x4 cascaded_matrices[4];
    float4 cascaded_plane_distances;
};
cbuffer csm_constants : register(b3)
{
    csm_constants csm_data;
}


VS_OUT_CSM main(VS_IN vin, uint instance_id : SV_INSTANCEID)
{
    if (skin > -1)
    {
        float4 blended_position = { 0, 0, 0, 1 };
        for (int bone_index = 0; bone_index < 4; ++bone_index)
        {
            blended_position += vin.weights[bone_index] * mul(vin.position, joint_matrices[vin.joints[bone_index]]);
        }
        vin.position = float4(blended_position.xyz, 1.0f);
    }
    
    VS_OUT_CSM vout;
    
    vout.instanceId = instance_id;
    vout.position = mul(float4(vin.position.xyz, 1), mul(world, csm_data.cascaded_matrices[instance_id]));
    vout.texcoord = vin.texcoord;

    return vout;
}