#include "ComputeParticle.hlsli"

RWStructuredBuffer<ParticleData> particleDataBuffer : register(u0); // パーティクル管理バッファ
AppendStructuredBuffer<uint> particleUnusedBuffer : register(u1); // パーティクル番号管理バッファ（末尾への追加専用）

RWStructuredBuffer<ParticleHeader> particleHeaderBuffer : register(u3); // パーティクルヘッダー管理バッファ

[numthreads(NumParticleThread, 1, 1)]
void main( uint3 dispatchThreadId : SV_DispatchThreadID )
{
    int index = dispatchThreadId.x;
    
    //パーティクル情報初期化
    particleHeaderBuffer[index].alive = 0;
    particleHeaderBuffer[index].particleIndex = 0;
    
    //未使用リスト（AppendStructuredBuffer）の末尾に追加
    particleUnusedBuffer.Append(index);
}