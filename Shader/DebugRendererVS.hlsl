#include "DebugRenderer.hlsli"

VS_OUT main(VS_IN vin)
{
    VS_OUT vout;
    vout.position = mul(vin.position, viewProj);
    vout.color = vin.color;

    return vout;
}