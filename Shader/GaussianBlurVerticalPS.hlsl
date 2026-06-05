//BLOOM
#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define LINEAR_BORDER_BLACK 3
#define LINEAR_BORDER_WHITE 4
SamplerState sampler_states[5] : register(s0);

Texture2D hdr_color_buffer_texture : register(t0);

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
    uint mip_level = 0, width, height, number_of_levels;
    hdr_color_buffer_texture.GetDimensions(mip_level, width, height, number_of_levels);
    const float aspect_ratio = width / height;

#if 1
    const float offset[3] = { 0.0, 1.3846153846, 3.2307692308 };
    const float weight[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };

    float4 sampled_color = hdr_color_buffer_texture.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord) * weight[0];
    for (int i = 1; i < 3; i++)
    {
        sampled_color += hdr_color_buffer_texture.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + float2(0.0, offset[i] / height)) * weight[i];
        sampled_color += hdr_color_buffer_texture.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord - float2(0.0, offset[i] / height)) * weight[i];
    }
#else
	const float offset[2] = { 0.53805, 2.06278 };
	const float weight[2] = { 0.44908, 0.05092 };
	float4 sampled_color = 0;
	for (int i = 0; i < 2; i++)
	{
		sampled_color += hdr_color_buffer_texture.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + float2(0.0, offset[i]) / height) * weight[i];
		sampled_color += hdr_color_buffer_texture.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord - float2(0.0, offset[i]) / height) * weight[i];
	}
#endif
    return sampled_color;
}
