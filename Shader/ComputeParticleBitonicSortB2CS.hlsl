#include "ComputeParticle.hlsli"

#include "ComputeParticleBitonicSort.hlsli"

[numthreads(BitonicSortB2Thread, 1, 1)]
void main( uint3 dispatchThreadId : SV_DispatchThreadID )
{
    uint t = dispatchThreadId.x;
    uint low = t & (increment - 1);
    uint i = (t << 1) - low;
    bool reverse = ((direction & i) == 0);
    
    ParticleHeader x0 = particleHeaderBuffer[i];
    ParticleHeader x1 = particleHeaderBuffer[i + increment];
    ParticleHeader auxa = x0;
    ParticleHeader auxb = x1;
    if (reverse ^ comparer(x0, x1))
    {
        x0 = auxb;
        x1 = auxa;
    }
    
    particleHeaderBuffer[i] = x0;
    particleHeaderBuffer[i + increment] = x1;
}