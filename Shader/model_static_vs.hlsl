#include "model.hlsli"

VS_OUT main(VS_IN_STATIC vin)
{
    VS_OUT vout;

    vout.position       = mul(float4(vin.position, 1), mul(world, viewProjection));
    vout.world_position = mul(float4(vin.position, 1), world);

    vout.world_normal   = normalize(mul(float4(vin.normal, 0), world));
    vout.world_normal.w = 0;

    float sigma         = vin.tangent.w;
    vout.world_tangent  = normalize(mul(float4(vin.tangent.xyz, 0), world));
    vout.world_tangent.w = sigma;

    vout.texcoord = vin.texcoord;

    return vout;
}
