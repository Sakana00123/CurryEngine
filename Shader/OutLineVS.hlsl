#include "gltf_model.hlsli"
#include "OutLine.hlsli"
VS_OUT main(VS_IN vin)
{
    float sigma = vin.tangent.w;

    if (skin > -1)
    {
        row_major float4x4 skin_matrix =
            vin.weights.x * joint_matrices[vin.joints.x] +
            vin.weights.y * joint_matrices[vin.joints.y] +
            vin.weights.z * joint_matrices[vin.joints.z] +
            vin.weights.w * joint_matrices[vin.joints.w];
        vin.position = mul(float4(vin.position.xyz, 1), skin_matrix);
        vin.normal = normalize(mul(float4(vin.normal.xyz, 0), skin_matrix));
        vin.tangent = normalize(mul(float4(vin.tangent.xyz, 0), skin_matrix));
    }

    VS_OUT vout;

    //vin.position += vin.normal * 2.0f;

    vin.position += vin.normal * strength;

    vin.position.w = 1;

    vout.position = mul(vin.position, mul(world, viewProjection));
    vin.normal.w = 0;
    vout.world_normal = normalize(mul(vin.normal, world));
    vin.tangent.w = 0;
    vout.world_tangent = normalize(mul(vin.tangent, world));
    vout.world_tangent.w = sigma;

    vout.world_position = mul(vin.position, world);

    vout.texcoord = vin.texcoord;


    return vout;
}