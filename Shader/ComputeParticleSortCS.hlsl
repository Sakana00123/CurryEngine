#include "ComputeParticle.hlsli"

RWStructuredBuffer<ParticleData> particleDataBuffer : register(u0);
RWByteAddressBuffer indirectDataBuffer : register(u2);

RWStructuredBuffer<ParticleHeader> particleHeaderBuffer : register(u3);

[numthreads(1, 1, 1)]
void main()
{
    //パーティクル総数を取得
    uint currentNumParticle = indirectDataBuffer.Load(IndirectArgumentsNumCurrentParticle);
    if (currentNumParticle == 0)
        return;
    
    //簡易的にソートを行う
    //クイックソートなどの再帰関数を使うものはコンピュートシェーダーの方で制限がかかっているため無理。
    for (int h = currentNumParticle / 2; h > 0; h /= 2)
    {
        for (int i = h; i < currentNumParticle; i += 1)
        {
            ParticleHeader k = particleHeaderBuffer[i];
            
            int j;
            for (j = i; j >= h && (particleHeaderBuffer[j - h].alive < k.alive); j -= h)
            {
                particleHeaderBuffer[j] = particleHeaderBuffer[j - h];
            }
            
            particleHeaderBuffer[j] = k;
        }

    }

}