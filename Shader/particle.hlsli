
cbuffer PARTICLE_SYSTEM_CONSTANTS : register(b9)
{
    float4 emissionPosition;
    float2 emissionOffset; // x: minimum radius, y: maximum radius
    float2 emissionSize; // x: spawn, y: despawn
    float2 emissionConeAngle; // x: minimum radian, y: maximum radian
    float2 emissionSpeed; // x: minimum speed, y: maximum speed
    float2 emissionAngularSpeed; // x: minimum angular speed, y: maximum angular speed
    float2 lifespan; // x: minimum second, y: maximum second
    float2 spawnDelay; // x: minimum second, y: maximum second
    float2 fadeDuration; // x: fade in, y: fade out
    
    float time;
    float deltaTime;
    float noiseScale;
    float gravity;
    
    uint2 spriteSheetGrid;
    uint maxParticleCount;
    bool respawn;
}

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 viewProjection;
    float4 lightDirection;
    float4 cameraPosition;
    
    row_major float4x4 view;
    row_major float4x4 projection;
}

struct VS_OUT
{
    uint vertexId : VERTEXID;
};

struct GS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

struct Particle
{
    int state;
    
    float4 color;
    
    float3 position; // World space position
    float mass; // Mass of particle
    
    float angle;
    float angularSpeed;
    float3 velocity; // World space velocity
    
    float lifespan; // Lifespan of the particle.
    //float distance_to_eye; // The distance from the particle to the eye
    float age; // The current age counting down from lifespan to zero
    
    float2 size;
    
    uint chip;
};

#define PI 3.14159265358979
float rand(float n)
{
	// Return value is greater than or equal to 0 and less than 1.
    return frac(sin(n) * 43758.5453123);
}

#define NUMTHREADS_X 16

void Spawn(in uint id, inout Particle p)
{
    float f0 = rand((float) id / maxParticleCount * 2.0 * PI);
    float f1 = rand(f0 * PI);
    float f2 = rand(f1 * PI);
    float f3 = rand(f2 * PI);
    
    float3 offset = 0;
#if 1
    float radius = lerp(emissionOffset.x, emissionOffset.y, f0);
    offset.x = radius * cos(f1 * 2.0 * PI);
    offset.z = radius * sin(f1 * 2.0 * PI);
#endif
    p.position = emissionPosition.xyz + offset;
    
    float phi = 2.0 * PI * f0;
    float theta = lerp(emissionConeAngle.x, emissionConeAngle.y, f1);
    
    float sinTheta = sin(theta);
    float cosTheta = cos(theta);
    float sinPhi = sin(phi);
    float cosPhi = cos(phi);
    
    p.velocity.x = sinTheta * cosPhi;
    p.velocity.y = cosTheta;
    p.velocity.z = sinTheta * sinPhi;
    p.velocity *= lerp(emissionSpeed.x, emissionSpeed.y, f2);
    
    p.color = 1.0;
    
    p.mass = 1.0;
    
#if 1
    p.angle = PI * f0;
    p.angularSpeed = lerp(emissionAngularSpeed.x, emissionAngularSpeed.y, f3);
#else
    p.angle = 0.0;
    p.angularSpeed = 0.0;
#endif
    
    p.lifespan = lerp(lifespan.x, lifespan.y, f0);
    p.age = -lerp(spawnDelay.x, spawnDelay.y, f0);
    p.state = 0;
    p.size.x = emissionSize.x * f1;
    p.size.y = emissionSize.y * f2;
    
    int count = spriteSheetGrid.x * spriteSheetGrid.y;
    p.chip = f3 * count;
}

float FadeIn(float duration, float age, float exponent)
{
    //return clamp(age / duration, 0.0, 1.0);
    return pow(smoothstep(0.0, 1.0, age / duration), exponent);
}

float FadeOut(float duration, float age, float lifespan, float exponent)
{
    //return clamp((lifespan - age) / duration, 0.0, 1.0);
    return pow(smoothstep(0.0, 1.0, (lifespan - age) / duration), exponent);
}