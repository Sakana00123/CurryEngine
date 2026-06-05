#include "ComputeParticle.hlsli"
RWByteAddressBuffer indirectDataBuffer : register(u2);

[numthreads(1, 1, 1)]
void main( uint3 dispatchThreadId : SV_DispatchThreadID )
{
    uint index = dispatchThreadId.x;
    
    //1フレーム前の総パーティクル数＋現在のフレームの生成パーティクル数＝”仮の”現在のフレームの総パーティクル数
    uint previousNumParticle = indirectDataBuffer.Load(IndirectArgumentsNumCurrentParticle);
    //ピクセルエミットパーティクルの総数を加算
    uint numEmitParticle = totalEmitCount + indirectDataBuffer.Load(IndirectArgumentsNumEmitPixelParticleIndex);
    indirectDataBuffer.Store(IndirectArgumentsNumEmitPixelParticleIndex, 0);
    numEmitParticle = min(numEmitParticle, maxEmitParticles);
    
    uint currentNumParticle = previousNumParticle + numEmitParticle;
    
    //現在のフレームの総パーティクル数はシステムの総パーティクル数で制限
    currentNumParticle = min(systemNumParticles, currentNumParticle);
    
    //総数を記録
    indirectDataBuffer.Store(IndirectArgumentsNumCurrentParticle, currentNumParticle);
    indirectDataBuffer.Store(IndirectArgumentsNumPreviousParticle, previousNumParticle);
    
    //死亡カウンターを初期化
    indirectDataBuffer.Store(IndirectArgumentsNumDeadParticle, 0);
    
    //エミッター用のDispatchIndirectに起動数を設定する
    uint3 emitDispatch;
    emitDispatch.x = currentNumParticle - previousNumParticle;
    emitDispatch.y = 1;
    emitDispatch.z = 1;
    indirectDataBuffer.Store3(IndirectArgumentsNumEmitParticleDispatchIndirect, emitDispatch);
    
    //エミッターの生成番号を設定
    //ソートするので１フレームのパーティクル数がそのままエミット番号になる
    //Append/ConsumeStructuredBufferは事実上不要になる
    indirectDataBuffer.Store(IndirectArgumentsNumEmitParticleIndex, previousNumParticle);
    
    //更新用のDispatchIndirectに起動数を設定する
    uint3 updateDispatch;
    updateDispatch.x = ((currentNumParticle + (NumParticleThread - 1)) / NumParticleThread);
    updateDispatch.y = 1;
    updateDispatch.z = 1;
    indirectDataBuffer.Store3(IndirectArgumentsUpdateParticleDispatchIndirect, updateDispatch);
}