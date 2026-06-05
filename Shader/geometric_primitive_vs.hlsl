#include "geometric_primitive.hlsli"
#include "Lights.hlsli"
#include "ShaderFunctions.hlsli"

VS_OUT main( float3 position : POSITION, float3 normal : NORMAL)
{
    VS_OUT vout;
    vout.position = mul(float4(position, 1.0), mul(world, viewProjection));
    float3 worldPosition = mul(float4(position, 1.0), world).xyz;
    float3 worldNormal = normalize(mul(float4(normal, 0.0), world).xyz);
    
    float4 N = normalize(mul(float4(normal, 0.0), world));
    float4 L = normalize(directionalLightDirection);
    float3 C = directionalLightColor.rgb;
    float3 K = materialColor.rgb;
    
    const float3 V = normalize(cameraPositon.xyz - worldPosition.xyz); // 視線ベクトル
    
    //vout.color.rgb = materialColor.rgb * max(0, LightColor * dot(L, N));
    //vout.color.rgb = CalcLambert(N.xyz, L.xyz, C, K);
    
    // DirectionalLightの適用
    vout.color.rgb = ClacHalfLambert(N.xyz, L.xyz, C, K);
    
    // PointLightの適用
    float3 pointDiffuse = float3(0, 0, 0);
    float3 pointSpecular = float3(0, 0, 0);
    CalcPointLights(worldPosition, worldNormal, V, pointDiffuse, pointSpecular);
    vout.color.rgb += pointDiffuse * K;
    
    // SpotLightの適用
    float3 spotDiffuse = float3(0, 0, 0);
    float3 spotSpecular = float3(0, 0, 0);
    CalcSpotLights(worldPosition, spotDiffuse, spotSpecular);
    vout.color.rgb += spotDiffuse * K;
    
    // アルファ値は定数
    vout.color.a = materialColor.a;
	return vout;
}