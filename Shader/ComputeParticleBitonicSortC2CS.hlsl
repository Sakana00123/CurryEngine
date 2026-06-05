#include "ComputeParticle.hlsli"

#include "ComputeParticleBitonicSort.hlsli"

groupshared ParticleHeader sharedData[BitonicSortC2Thread * 2];

[numthreads(BitonicSortC2Thread, 1, 1)]
void main( uint3 dispatchThreadId : SV_DispatchThreadID )
{
    int t = dispatchThreadId.x; // thread index
    int wgBits = 2 * BitonicSortC2Thread - 1; // bit mask to get index in local memory AUX (size is 2*WG)
    
    for (int inc = increment; inc > 0; inc >>= 1)
    {
        int low = t & (inc - 1); // low order bits (below INC)
        int i = (t << 1) - low; // insert 0 at position INC
        bool reverse = ((direction & i) == 0); // asc/desc order
        ParticleHeader x0, x1;
        
        // Load
        if (inc == (int) increment)
        {
            // First iteration: load from global memory
            x0 = particleHeaderBuffer[i];
            x1 = particleHeaderBuffer[i + inc];
        }
        else
        {
            // Other iterations: load from local memory
            GroupMemoryBarrierWithGroupSync();
            x0 = sharedData[i & wgBits];
            x1 = sharedData[(i + inc) & wgBits];
        }
        
        // Sort
        {
            ParticleHeader auxa = x0;
            ParticleHeader auxb = x1;
            if (reverse ^ comparer(x0, x1))
            {
                x0 = auxb;
                x1 = auxa;
            }
        }
        
        // Store
        if (inc == 1)
        {
            // Last iteration: store to global memory
            particleHeaderBuffer[i] = x0;
            particleHeaderBuffer[i + inc] = x1;
        }
        else
        {
            // Other iterations: store to local memory
            GroupMemoryBarrierWithGroupSync();
            sharedData[i & wgBits] = x0;
            sharedData[(i + inc) & wgBits] = x1;
        }
    }
}