#include "particle.hlsli"

RWStructuredBuffer<Particle> particleBuffer : register(u0);

[numthreads(NUMTHREADS_X, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint id = DTid.x;
    
    Particle p = particleBuffer[id];
    
    Spawn(id, p);
    
    particleBuffer[id] = p;
    
}