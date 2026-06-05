#include "skinned_mesh.hlsli"
#include "Lights.hlsli"

#include "Sampler.hlsli"
Texture2D textureMaps[4] : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = textureMaps[0].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
    float alpha = color.a;
#if 1
    // Inverse gammma process
    const float GAMMA = 2.2;
    color.rgb = pow(color.rgb, GAMMA);
#endif
    float3 N = normalize(pin.world_normal.xyz);
    float3 T = normalize(pin.world_tangent.xyz);
    float sigma = pin.world_tangent.w;
    T = normalize(T - N * dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);
    
    float4 normal = textureMaps[1].Sample(samplerStates[LINEAR], pin.texcoord);
    normal = (normal * 2.0) - 1.0;
    N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));
    
    float3 L = normalize(-directionalLightDirection);
    float3 diffuse = color.rgb * max(0, dot(N, L));
    diffuse *= directionalLightColor.rgb;
    float3 V = normalize(cameraPositon.xyz - pin.world_position.xyz);
    float3 specular = pow(max(0, dot(N, normalize(V + N))), 128);
    return float4(diffuse + specular, alpha) * pin.color;
}