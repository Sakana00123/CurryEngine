#include "Decal.hlsli"

float4 main( float4 position : POSITION ) : SV_POSITION
{
    return mul(position, mul(world, viewProjection));
}