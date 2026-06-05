struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

cbuffer SHADERTOY_CONSTANTS : register(b7)
{
    float4 iResolution;
    float4 iMouse; // TOOD
    float4 iChannelResolution[4]; // TODO
    float iTime;
    float iFrame; // TODO
    float iPad0;
    float iPad1;
}


typedef float2 vec2;
typedef float3 vec3;
typedef float4 vec4;
typedef int2 ivec2;
typedef int3 ivec3;
typedef int4 ivec4;
typedef uint2 uvec2;
typedef uint3 uvec3;
typedef uint4 uvec4;
typedef float2x2 mat2;
typedef float3x3 mat3;
typedef float4x4 mat4;
#define mix lerp
#define mod fmod
#define fract frac
#define atan atan2
#define inversesqrt rsqrt
#define texture Sample
#define textureLod SampleLevel
#define texelFetch Load
#define sampler2D Texture2D

#if 0

float3 drawCircle(float2 pos, float radius, float width, float power, float4 color)
{
    //float2 mousePos = iMouse.xy - float2(0.5);
    float dist1 = length(pos);
    dist1 = fract((dist1 * 5.0) - fract(iTime));
    float dist2 = dist1 - radius;
    float intensity = pow(radius / abs(dist2), width);
    float3 col = color.rgb * intensity * power * max((0.8 - abs(dist2)), 0.0);
    return col;
}

float3 hsv2rgb(float h, float s, float v)
{
    float4 t = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = 1/*abs(frac(float3(h) + t.xyz) * 6.0 - float3(t.w))*/;
    return /*v * mix(float3(t.x), clamp(p - float3(t.x), 0.0, 1.0), s)*/1;
}

//void mainImage(out float4 fragColor, in float2 fragCoord)
//{
//    // // -1.0 ~ 1.0
//    float2 pos = (fragCoord.xy * 2.0 - iResolution.xy) / min(iResolution.x, iResolution.y);
    
//    float h = mix(0.5, 0.65, length(pos));
//    float4 color = float4(hsv2rgb(h, 1.0, 1.0), 1.0);
//    float radius = 0.5;
//    float width = 0.8;
//    float power = 0.1;
//    float3 finalColor = drawCircle(pos, radius, width, power, color);

//    pos = abs(pos);
//    // vec3 finalColor = vec3(pos.x, 0.0, pos.y);

//    fragColor = float4(finalColor, 1.0);
//}

void mainImage(out float4 fragColor, in float2 fragCoord)
{
    float3 c;
    float l, z = 3. * iTime;
    for (int i = 0; i < 3; i++)
    {
        vec2 uv, p = fragCoord.xy / iResolution.xy;
        uv = p;
        p -= .5;
        p.x *= iResolution.x / iResolution.y;
        z += 2.1;
        l = length(p);
        float h = 3. * log(l);
        
        //digital
		//c[i]=((sin(h-z)+1.)/2.>.9)?1.:0.;
        
        //analog
        c[i] = (sin(h - z) + 1.) / 2.;
    }
    fragColor = float4(c, iTime);
}
#else
#define Rot(a) float2x2(cos(a),-sin(a),sin(a),cos(a))
#define antialiasing(n) n/min(iResolution.y,iResolution.x)
#define S(d,b) smoothstep(antialiasing(1.0),b,d)
#define T (iTime)
#define matRotateZ(rad) float3x3(cos(rad),-sin(rad),0,sin(rad),cos(rad),0,0,0,1)
#define DEBUG 0

// https://iquilezles.org/articles/distfunctions2d
float sdBox(float2 p, float2 b)
{
    vec2 d = abs(p) - b;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

float sdCappedCylinder(float3 p, float h, float r)
{
    float2 d = abs(float2(length(p.xz), p.y)) - float2(h, r);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

float3 Transform(float3 p, float angle)
{
    p.xz = mul(Rot(angle), p.xz);
    p.xy = mul(Rot(angle*.7), p.xy);
    return p;
}

float Confetti(float3 ro, float3 rd, float3 pos, float angle, int type)
{
    float t = dot(pos - ro, rd);
    float3 p = ro + rd * t;
    float y = length(pos - p);
    float2 bsize = float2(0.2, 0.25);
    float d = 1.0;
    if (type == 0 && y < 1.0)
    {
        float x = sqrt(1.0 - y);
        
        // front
        float3 pF = ro + rd * (t - x) - pos;
        pF = Transform(pF, angle);
        float2 uvF = float2(atan(pF.x, pF.z), pF.y);
        float f = sdBox(uvF, bsize);
        
        // back
        float3 pB = ro + rd * (t + x) - pos;
        pB = Transform(pB, angle);
        float2 uvB = float2(atan(pB.x, pB.z), pB.y);
        float b = sdBox(uvB, bsize);
        d = min(f, b);
    }
    
    float3 rotatedPos = mul(matRotateZ(radians(90.0)), pos - p);
    y = sdCappedCylinder(rotatedPos, 0.3, 0.001);
    //y = sdCappedCylinder((pos - p) * matRotateZ(radians(90.0)), 0.3, 0.001);
    bsize = float2(1.0, 0.02);
    if (type == 1 && y < 0.07)
    {
        p = pos - p;
		
        float2 uv = p.xy;
        uv = mul(Rot(radians(30.0)), uv);
        uv.y -= T * 0.2;
        
        uv.y = mod(uv.y, 0.1) - 0.05;
        d = sdBox(uv, bsize);
    }
    
    return d;
}

void mainImage(out float4 fragColor, in float2 fragCoord)
{
    
    float2 uv = (fragCoord - .5 * iResolution.xy) / iResolution.y;
    
    float2 M = iMouse.xy / iResolution.xy - .5;

    float3 bg = 0.9 * max(
    mix(
        float3(1.2, 1.2, 1.2) + (0.1 - length(uv.xy) / 3.0),
        float3(1.0, 1.0, 1.0),
        0.1
    ),
    float3(0.0, 0.0, 0.0)
);
    float4 col = float4(bg, 0.0);
    float3 ro = float3(0.0, 0.0, -3.0);
    float3 rd = normalize(float3(uv, 1));
	    
#if DEBUG
        float confetti = Confetti(ro,rd,vec3(0.0),iTime,1);
        col = mix(col,vec4(vec3(1.0,0.0,0.0),1.0),S(confetti,0.0));
#else
    for (float i = 0.; i < 1.0; i += 1.0 / 60.0)
    {
        float x = mix(-8.0, 8.0, fract(i)) + M.x;
        float y = mix(5., -5., fract((sin(i * 564.3) * 4570.3) - T * .3)) + M.y;
        float z = mix(5.0, 0.0, i);
        float a = T + i * 563.34;
        float ratio = clamp(fract((sin(i * 1564.3) * 9570.3)), 0.0, 1.0);
        int type = (i < ratio) ? 0 : 1;

        float3 ccol = 0.5 + 0.5 * cos(T + uv.xyx + float3(i, 2, 4));
        float confetti = Confetti(ro, rd, float3(x, y, z), a, type);
        col = mix(col, float4(ccol, 1.0), S(confetti, 0.0));
    }
#endif
        
    fragColor = col.rgba;
}
#endif
float4 main(VS_OUT pin) : SV_TARGET
{
    float4 fragColor = 0;
    float2 fragCoord = pin.position.xy;

    mainImage(fragColor, fragCoord);

    return fragColor;
}