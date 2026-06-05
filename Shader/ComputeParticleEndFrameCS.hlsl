#include "ComputeParticle.hlsli"

RWByteAddressBuffer indirectDataBuffer : register(u2);

[numthreads(1, 1, 1)]
void main()
{
    uint currentNumParticle = indirectDataBuffer.Load(IndirectArgumentsNumCurrentParticle);

    //死亡カウンターを取得＆初期化
    uint destroyCounter;
    indirectDataBuffer.InterlockedExchange(IndirectArgumentsNumDeadParticle, 0, destroyCounter);
    
    //現在のフレームでのパーティクル総数を再計算
    indirectDataBuffer.Store(IndirectArgumentsNumCurrentParticle, currentNumParticle - destroyCounter);
    
    //描画コール数をここで決める
    indirectDataBuffer.Store(IndirectArgumentsDrawIndirect, currentNumParticle - destroyCounter);
}