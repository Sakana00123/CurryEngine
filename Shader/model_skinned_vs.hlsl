#include "model.hlsli"

VS_OUT main(VS_IN_SKINNED vin)
{
    float sigma = vin.tangent.w;

    // スキニング：ジョイント行列をウェイトでブレンドしてスキン行列を構築
    if (skinIndex > -1)
    {
        row_major float4x4 skin_matrix =
            vin.weights.x * joint_matrices[vin.joints.x] +
            vin.weights.y * joint_matrices[vin.joints.y] +
            vin.weights.z * joint_matrices[vin.joints.z] +
            vin.weights.w * joint_matrices[vin.joints.w];

        vin.position = mul(float4(vin.position, 1),   skin_matrix).xyz;
        vin.normal   = normalize(mul(float4(vin.normal, 0),        skin_matrix).xyz);
        vin.tangent.xyz = normalize(mul(float4(vin.tangent.xyz, 0), skin_matrix).xyz);
    }

    VS_OUT vout;

    vout.position       = mul(float4(vin.position, 1), mul(world, viewProjection));
    vout.world_position = mul(float4(vin.position, 1), world);

    vout.world_normal   = normalize(mul(float4(vin.normal, 0), world));
    vout.world_normal.w = 0;

    vout.world_tangent   = normalize(mul(float4(vin.tangent.xyz, 0), world));
    vout.world_tangent.w = sigma;

    vout.texcoord = vin.texcoord;

    return vout;
}
