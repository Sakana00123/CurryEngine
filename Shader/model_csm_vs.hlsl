#include "model.hlsli"

cbuffer CSM_CONSTANTS : register(b3)
{
    row_major float4x4 cascaded_matrices[4];
    float4             cascaded_plane_distances;
};

// CSM は静的・スキニングどちらも共通 VS で処理する。
// スキニングあり頂点（VS_IN_SKINNED）を入力とし、
// skinIndex == -1 のときはスキニング計算をスキップする。
// 静的メッシュを CSM に描く場合は joints/weights が 0 になるだけなので問題ない。
VS_OUT_CSM main(VS_IN_SKINNED vin, uint instance_id : SV_INSTANCEID)
{
    if (skinIndex > -1)
    {
        float4 blended = float4(0, 0, 0, 1);
        [unroll]
        for (int b = 0; b < 4; ++b)
        {
            blended += vin.weights[b] * mul(float4(vin.position, 1), joint_matrices[vin.joints[b]]);
        }
        vin.position = blended.xyz;
    }

    VS_OUT_CSM vout;
    vout.instanceId = instance_id;
    vout.position   = mul(float4(vin.position, 1), mul(world, cascaded_matrices[instance_id]));
    vout.texcoord   = vin.texcoord;

    return vout;
}
