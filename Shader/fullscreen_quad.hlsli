struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
};

cbuffer CONSTANT_TEST : register(b2)
{
    float f_min;
    float f_max;
    float gaussian_sigma;
    float bloom_intensity;
    float exposure;
    float p[3];
};