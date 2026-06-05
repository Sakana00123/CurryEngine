#include "GltfModelGBuffer.hlsli"

VS_OUT main(VS_IN vin)
{
    float sigma = vin.tangent.w; // ミップマップレベル
	
    VS_OUT vout = (VS_OUT) 0;
	
    vin.position.w = 1;
    vout.position = mul(vin.position, mul(world, viewProjection));
    vout.world_position = mul(vin.position, world);
	
    vin.normal.w = 0;
    vout.world_normal = normalize(mul(vin.normal, world));
    
    vin.tangent.w = 0;
    vout.world_tangent = normalize(mul(vin.tangent, world));
    vout.world_tangent.w = sigma; // ミップマップレベルを渡す
    
    vout.texcoord = vin.texcoord;
    
	return vout;
}