#include "particle.hlsli"
#include "Sampler.hlsli"

Texture2D colorTemperChart : register(t0);

StructuredBuffer<Particle> particleBuffer : register(t9);


struct GSOutput
{
	float4 pos : SV_POSITION;
};

[maxvertexcount(4)]
void main(point VS_OUT input[1], inout TriangleStream<GS_OUT> output)
{
    const float2 corners[] =
    {
        float2(-1.0, -1.0),
		float2(-1.0, +1.0),
		float2(+1.0, -1.0),
		float2(+1.0, +1.0),
    };
    const float2 texcoords[] =
    {
        float2(0.0, 1.0),
		float2(0.0, 0.0),
		float2(1.0, 1.0),
		float2(1.0, 0.0),
    };
	
    Particle p = particleBuffer[input[0].vertexId];
	
#if 1
    if (p.age <= 0.0 || p.age >= p.lifespan)
    {
        return;
    }
#endif
	
    const float aspectRatio = 1280.0 / 720.0;
	
    float ageGaradient = saturate(p.age / p.lifespan);
	
#if 1
    float size = lerp(p.size.x/*spawn*/, p.size.y/*despawn*/, ageGaradient);
#else
	float size = p.maxSize;
#endif
    
    [unroll]
    for (uint vertexIndex = 0; vertexIndex < 4; ++vertexIndex)
    {
        GS_OUT element;
        
        // Transform to clip space
        element.position = mul(float4(p.position, 1), viewProjection);
        // Make corner position as billboard
        float2 corner;
#if 1
        corner.x = corners[vertexIndex].x * cos(p.angle) - corners[vertexIndex].y * sin(p.angle);
        corner.y = corners[vertexIndex].x * sin(p.angle) + corners[vertexIndex].y * cos(p.angle);
#else
        corner = corners[vertexIndex];
#endif
        element.position.xy += corner * float2(size, size * aspectRatio);
        
        element.color = p.color;
        
#if 0
		float fadeInTime = 2.0;
		float fadeOutTime = 5.0;
		float alpha = FadeIn(fadeInTime, p.age, 1) * FadeOut(fadeOutTime, p.age, p.lifespan, 1);
		element.color.a = pow(alpha, 1);
#endif
        
        //復数のテクスチャがあるとき
#if 0
        float2 texcoord = texcoords[vertexIndex] / spriteSheetGrid;
        float2 gridSize = 1.0 / spriteSheetGrid;
        uint x = p.chip % spriteSheetGrid.x;
        uint y = p.chip / spriteSheetGrid.x;
        element.texcoord = texcoord + gridSize * uint2(x, y);
#else
        element.texcoord = texcoords[vertexIndex];
#endif
        
        output.Append(element);
    }
    output.RestartStrip();

}