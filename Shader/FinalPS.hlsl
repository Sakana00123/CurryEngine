#include "fullscreen_quad.hlsli"
#include "Sampler.hlsli"

cbuffer TUTORIAL_CONSTANTS : register(b6)
{
    float2 transferTexcoord;
    float radius;
    float aspect;
    
    float4 color;
    
    uint isTutorialScene;
    float brightness;
    float contrast;
    float hue;

    float saturation;
}

Texture2D texture_map : register(t2);
float3 brightness_contrast(float3 fragment_color, float brightness, float contrast)
{
	//Brightness - Contrast Effect
	//The brightness - contrast effect allows you to modify the brightness and contrast of the rendered image.
	//Brightness: The brighness of the image.Ranges from - 1 to 1 (-1 is solid black, 0 no change, 1 solid white).
	//Contrast : The contrast of the image.Ranges from - 1 to 1 (-1 is solid gray, 0 no change, 1 maximum contrast).
    fragment_color += brightness;
    if (contrast > 0.0)
    {
        fragment_color = (fragment_color - 0.5) / (1.0 - contrast) + 0.5;
    }
    else if (contrast < 0.0)
    {
        fragment_color = (fragment_color - 0.5) * (1.0 + contrast) + 0.5;
    }
    return fragment_color;
}

float3 hue_saturation(float3 fragment_color, float hue, float saturation)
{
	//Hue - Saturation Effect
	//The hue - saturation effect allows you to modify the hue and saturation of the rendered image.
	//Hue: The hue of the image.Ranges from - 1 to 1 (-1 is 180 degrees in the negative direction, 0 no change, 1 is 180 degrees in the postitive direction).
	//Saturation : The saturation of the image.Ranges from - 1 to 1 (-1 is solid gray, 0 no change, 1 maximum saturation).
    float angle = hue * 3.14159265;
    float s = sin(angle), c = cos(angle);
    float3 weights = (float3(2.0 * c, -sqrt(3.0) * s - c, sqrt(3.0) * s - c) + 1.0) / 3.0;
    fragment_color = float3(dot(fragment_color, weights.xyz), dot(fragment_color, weights.zxy), dot(fragment_color, weights.yzx));
    float average = (fragment_color.r + fragment_color.g + fragment_color.b) / 3.0;
    if (saturation > 0.0)
    {
        fragment_color += (average - fragment_color) * (1.0 - 1.0 / (1.001 - saturation));
    }
    else
    {
        fragment_color += (average - fragment_color) * (-saturation);
    }
    return fragment_color;
}

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = texture_map.Sample(samplerStates[LINEAR], pin.texcoord);
    float alpha = color.a;
    float3 fragment_color = color.rgb;
    fragment_color = hue_saturation(fragment_color, hue, saturation);
    fragment_color = brightness_contrast(fragment_color, brightness, contrast);
    
    return float4(fragment_color, alpha);
}