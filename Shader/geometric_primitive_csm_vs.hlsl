#include "geometric_primitive.hlsli"

struct csm_constants
{
    row_major float4x4 cascaded_matrices[4];
    float4 cascaded_plane_distances;
};

cbuffer csm_constants : register(b3)
{
    csm_constants csm_data;
}


VS_OUT_CSM main(VS_IN vin, uint instanceId : SV_InstanceID)
{
    VS_OUT_CSM vout;
    
    vout.instanceId = instanceId;
    vout.position = mul(float4(vin.position.xyz, 1), mul(world, csm_data.cascaded_matrices[instanceId]));
    vout.texcoord = vin.texcoord;
    
    return vout;
}
