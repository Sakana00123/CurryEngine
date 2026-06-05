#include "static_mesh.hlsli"
#include "Sampler.hlsli"
Texture2D color_map : register(t0);
Texture2D normal_map : register(t1);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = color_map.Sample(samplerStates[ANISOTROPIC], pin.texcoord);
    float alpha = color.a;
    float3 N = normalize(pin.world_normal.xyz);
    
    //phong
#if 1
    float3 T = float3(1.0001, 0, 0);
    float3 B = normalize(cross(N, T));
    T = normalize(cross(B, N));
    
    float4 normal = normal_map.Sample(samplerStates[LINEAR], pin.texcoord);
    normal = (normal * 2.0) - 1.0;
    normal.w = 0;
    N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));
#endif
    
    float3 L = normalize(-directionalLightDirection.xyz);
    float3 diffuse = color.rgb * max(0, dot(N, L));
    diffuse *= directionalLightColor.rgb;
    
    float3 V = normalize(cameraPositon.xyz - pin.world_normal.xyz);
    float3 specular = pow(max(0, dot(N, normalize(V + L))), 128);
    
    return float4(diffuse + specular, alpha) * pin.color;
}