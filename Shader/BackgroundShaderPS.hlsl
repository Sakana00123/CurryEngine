#include "fullscreen_quad.hlsli"
#include "Sampler.hlsli"
#include "constants.hlsli"

Texture2D texture_map : register(t0);

cbuffer BACKGROUND_CONSTANT_BUFFER : register(b7)
{
    float2 uvOffset; // UVオフセット
};

static const float3 bgColor = float3(0.98, 0.78, 0.55);
static const float3 rectColor = float3(0.98, 0.55, 0.25);
static const float3 darkColor = float3(0.98, 0.55, 0.25);


static const float noiseIntensity = 2.8;
static const float noiseDefinition = 0.6;
static const float2 glowPos = float2(-2.0, 0.0);

static const float total = 60.0;
static const float minSize = 0.03;
static const float maxSize = 0.08 - minSize; // 0.05
static const float yDistribution = 0.5;
float random(float2 co)
{
    // fract(sin(dot(...)) * N)  ← identical to GLSL
    return frac(sin(dot(co, float2(12.9898, 78.233))) * 43758.5453);
}

float noise(float2 p)
{
    p *= noiseIntensity;
    float2 i = floor(p);
    float2 f = frac(p);
    float2 u = f * f * (3.0 - 2.0 * f); // smoothstep

    return lerp(
               lerp(random(i + float2(0.0, 0.0)),
                    random(i + float2(1.0, 0.0)), u.x),
               lerp(random(i + float2(0.0, 1.0)),
                    random(i + float2(1.0, 1.0)), u.x),
               u.y);
}

float fbm(float2 uv)
{
    uv *= 5.0;
    // rotation matrix
    float2x2 m = float2x2(1.6, 1.2, -1.2, 1.6);

    float f = 0.5000 * noise(uv);
    uv = mul(m, uv);
    f += 0.2500 * noise(uv);
    uv = mul(m, uv);
    f += 0.1250 * noise(uv);
    uv = mul(m, uv);
    f += 0.0625 * noise(uv);

    f = 0.5 + 0.5 * f;
    return f;
}

// ============================================================
//  Background
// ============================================================
float3 bg(float2 uv)
{
    float velocity = time / 1.6;
    float intensity = sin(uv.x * 3.0 + velocity * 2.0) * 1.1 + 1.5;

    uv.y -= 2.0;
    float2 bp = uv + glowPos;
    uv *= noiseDefinition;

    // ripple
    float rb = fbm(float2(uv.x * 0.5 - velocity * 0.03, uv.y)) * 0.1;
    uv += rb;

    // coloring
    float rz = fbm(uv * 0.9 + float2(velocity * 0.35, 0.0));
    rz *= dot(bp * intensity, bp) + 1.2;

    float3 col = bgColor / (0.1 - rz);
    return sqrt(abs(col));
}

// ============================================================
//  Rectangle SDF (soft-edged)
// ============================================================
float rectangle(float2 uv, float2 pos, float width, float height, float blur)
{
    // GLSL: pos = (vec2(width,height)+.01)/2. - abs(uv-pos)
    float2 d = (float2(width, height) + 0.01) * 0.5 - abs(uv - pos);
    d = smoothstep(0.0, blur, d);
    return d.x * d.y;
}

// ============================================================
//  2-D rotation matrix
// ============================================================
float2x2 rotate2d(float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return float2x2(c, -s, s, c);
}
float4 main(VS_OUT pin) : SV_TARGET
{
    float2 uv = pin.texcoord;
    uv = uv * 2.0 - 1.0; // [0,1] → [-1,1]

    // Background
    float3 color = bg(uv) * (2.0 - abs(uv.y * 2.0));

    // Floating rectangles
    float velX = -time / 8.0;
    float velY = time / 10.0;

    [loop]
    for (float i = 0.0; i < total; i += 1.0)
    {
        float index = i / total;
        float rnd = random(float2(index, 0.0)); // vec2(index) → float2(index,0)

        float3 pos;
        pos.x = frac(velX * rnd + index) * 4.0 - 2.0;
        pos.y = sin(index * rnd * 1000.0 + velY) * yDistribution;
        pos.z = maxSize * rnd + minSize; // z stores size

        // Rotate UV around rectangle pivot
        float2 uvRot = uv - pos.xy + pos.z * 0.5;
        uvRot = mul(rotate2d(i + time * 0.5), uvRot);
        uvRot += pos.xy + pos.z * 0.5;

        float rect = rectangle(uvRot, pos.xy, pos.z, pos.z,
                               (maxSize + minSize - pos.z) * 0.5);

        color += rectColor * rect * (pos.z / maxSize);
    }

    float lum = dot(color, float3(0.299, 0.587, 0.114));
    float darkMask = 1.0 - saturate(lum * 1.5);
    color = lerp(color, darkColor, darkMask * 0.85);
    
    return float4(color, 1.0);
}
//float4 main(VS_OUT pin) : SV_TARGET
//{
//    float2 uv = pin.texcoord;
//    uv += uvOffset; // UVオフセットを適用
//    float4 color = texture_map.Sample(samplerStates[LINEAR_WRAP], uv);
//    
//    return color;
//}