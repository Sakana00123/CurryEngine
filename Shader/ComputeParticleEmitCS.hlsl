#include "ComputeParticle.hlsli"

RWStructuredBuffer<ParticleData> particleDataBuffer : register(u0); // パーティクル管理バッファ
ConsumeStructuredBuffer<uint> particlePoolBuffer : register(u1); // パーティクル番号管理バッファ（末尾から取り出し専用）

StructuredBuffer<EmitParticleData> emitParticleBuffer : register(t0); //パーティクル生成情報バッファ
RWByteAddressBuffer indirectDataBuffer : register(u2); //インダイレクト用バッファ

RWStructuredBuffer<ParticleHeader> particleHeaderBuffer : register(u3); //パーティクルヘッダー管理バッファ

[numthreads(1, 1, 1)]
void main( uint3 dispatchThreadId : SV_DispatchThreadID )
{
    //未使用リストの末尾から未使用パーティクルのindexを取得
    uint particleIndex = particlePoolBuffer.Consume();
    uint emitIndex = dispatchThreadId.x;

    //ヘッダーの末尾から取得
    uint headerIndex = 0;
    indirectDataBuffer.InterlockedAdd(IndirectArgumentsNumEmitParticleIndex, 1, headerIndex);
    
    //パーティクル生成処理
    particleHeaderBuffer[headerIndex].alive = 1; //生存フラグ
    particleHeaderBuffer[headerIndex].particleIndex = particleIndex; //パーティクルデータバッファの座標
    particleHeaderBuffer[headerIndex].depth = 1; //深度
    particleHeaderBuffer[headerIndex].dummy = 0; //空き
    
    //パーティクル生成処理
    particleDataBuffer[particleIndex].parameter.x = emitParticleBuffer[emitIndex].parameter.x; //描画モード
    particleDataBuffer[particleIndex].parameter.y = emitParticleBuffer[emitIndex].parameter.y; //生存時間カウント用
    particleDataBuffer[particleIndex].parameter.z = emitParticleBuffer[emitIndex].parameter.y; //生存時間記録用
    particleDataBuffer[particleIndex].parameter.w = emitParticleBuffer[emitIndex].parameter.w; //テクスチャインデックス
    
    particleDataBuffer[particleIndex].position = emitParticleBuffer[emitIndex].position;
    particleDataBuffer[particleIndex].rotation = emitParticleBuffer[emitIndex].rotation;
    particleDataBuffer[particleIndex].scale = emitParticleBuffer[emitIndex].scale;
    
    // 初期値の保存 (イージング用)
    particleDataBuffer[particleIndex].initialPosition.xyz = emitParticleBuffer[emitIndex].position.xyz;
    particleDataBuffer[particleIndex].initialRotation.xyz = emitParticleBuffer[emitIndex].rotation.xyz;
    particleDataBuffer[particleIndex].initialScale.xy = emitParticleBuffer[emitIndex].scale.xy;
    
    // 目標値の保存 (イージング用)
    particleDataBuffer[particleIndex].targetPosition = emitParticleBuffer[emitIndex].targetPosition;
    particleDataBuffer[particleIndex].targetRotation = emitParticleBuffer[emitIndex].targetRotation;
    particleDataBuffer[particleIndex].targetScale = emitParticleBuffer[emitIndex].targetScale;
    
    particleDataBuffer[particleIndex].velocity = emitParticleBuffer[emitIndex].velocity;
    particleDataBuffer[particleIndex].initialVelocity = emitParticleBuffer[emitIndex].velocity;
    particleDataBuffer[particleIndex].acceleration = emitParticleBuffer[emitIndex].acceleration;
    
    particleDataBuffer[particleIndex].startSpeed = emitParticleBuffer[emitIndex].startSpeed;
    particleDataBuffer[particleIndex].endSpeed = emitParticleBuffer[emitIndex].endSpeed;
    particleDataBuffer[particleIndex].speedEasingMode = emitParticleBuffer[emitIndex].speedEasingMode;
    particleDataBuffer[particleIndex].speedEasingTime = emitParticleBuffer[emitIndex].speedEasingTime;
    
    particleDataBuffer[particleIndex].startColor = emitParticleBuffer[emitIndex].startColor;
    particleDataBuffer[particleIndex].endColor = emitParticleBuffer[emitIndex].endColor;
    
    // フェードイン・アウトの設定
    particleDataBuffer[particleIndex].fadeInTime = emitParticleBuffer[emitIndex].fadeInTime;
    particleDataBuffer[particleIndex].fadeOutTime = emitParticleBuffer[emitIndex].fadeOutTime;
    
}