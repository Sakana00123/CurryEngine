
#include "gltf_model.hlsli"

VS_OUT main(BATCH_VS_IN vin)
{
    VS_OUT vout;

    vin.position.w = 1;
    vout.position = mul(vin.position, mul(world, viewProjection));
    vout.world_position = mul(vin.position, world);

    vin.normal.w = 0;
    vout.world_normal = normalize(mul(vin.normal, world));
    //vout.world_normal = normalize(mul(vin.normal, inverse_transpose_world));

    float sigma = vin.tangent.w;
    vin.tangent.w = 0;
    vout.world_tangent = normalize(mul(vin.tangent, world));
    //vout.world_tangent = normalize(mul(vin.tangent, inverse_transpose_world));
    vout.world_tangent.w = sigma;

    vout.texcoord = vin.texcoord;

    return vout;
}
