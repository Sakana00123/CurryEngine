#include "static_mesh.hlsli"

VS_OUT main( float4 position : POSITION, float4 normal : NORMAL, float2 texcoord : TEXCOORD )
{
    VS_OUT vout;
    vout.position = mul(position, mul(world, viewProjection));
    
    vout.world_position = mul(position, world);
    normal.w = 0;
    vout.world_normal = normalize(mul(normal, world));
    
    vout.color = material_color;
    
    vout.texcoord = texcoord;
    
	return vout;
}