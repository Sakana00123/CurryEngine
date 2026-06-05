#include "fullscreen_quad.hlsli"
#include "Sampler.hlsli"
Texture2D texture_maps[4] : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    uint mip_level = 0, width, height, number_of_levels;
    texture_maps[1].GetDimensions(mip_level, width, height, number_of_levels);
    
    float4 color = texture_maps[0].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
    float alpha = color.a;
    
    float3 blur_color = 0;
    float gaussian_kernel_total = 0;
    
    const int gaussian_half_kernel_size = 3;
    //const float gaussian_sigma = 1.0;
    
    [unroll]
    for (int x = -gaussian_half_kernel_size; x <= +gaussian_half_kernel_size; x += 1)
    {
        [unroll]
        for (int y = -gaussian_half_kernel_size; y <= +gaussian_half_kernel_size; y += 1)
        {
            float gaussian_kernel = exp(-(x * x + y * y) / (2.0 * gaussian_sigma * gaussian_sigma)) /
                (2 * 3.14159265358979 * gaussian_sigma * gaussian_sigma);
            blur_color += texture_maps[1].Sample(samplerStates[LINEAR], pin.texcoord +
                float2(x * 1.0 / width, y * 1.0 / height)).rgb * gaussian_kernel;
            gaussian_kernel_total += gaussian_kernel;
        }
    }
    blur_color /= gaussian_kernel_total;
    //const float bloom_intensity = 1.0;
    color.rgb += blur_color * bloom_intensity;
    
#if 1
    // Tone mapping : HDR -> SDR
    //const float exposure = 1.2;
    color.rgb = 1 - exp(-color.rgb * exposure);
#endif
    
#if 1
    // Gamma process
    const float GAMMA = 2.2;
    color.rgb = pow(color.rgb, 1.0 / GAMMA);
#endif
    
    
    return float4(color.rgb, alpha);
}