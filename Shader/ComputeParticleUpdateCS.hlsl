#include "ComputeParticle.hlsli"
#include "Sampler.hlsli"

RWStructuredBuffer<ParticleData> particleDataBuffer : register(u0); //パーティクル管理バッファ
AppendStructuredBuffer<uint> particleUnusedBuffer : register(u1); //パーティクル番号管理バッファ（末尾への追加専用）

RWByteAddressBuffer indirectDataBuffer : register(u2); // インダイレクト用バッファ

RWStructuredBuffer<ParticleHeader> particleHeaderBuffer : register(u3); //パーティクルヘッダー管理バッファ


Texture1DArray<float4> gradientTextures : register(t1); //グラデーションテクスチャ

// UV座標の計算
void CalculateUV( inout ParticleData data )
{
    uint frameCount = textureSplitCount.x * textureSplitCount.y;
    //切り取り座標を算出
    float lifeRatio = (data.parameter.z - data.parameter.y) / data.parameter.z; // 生存時間に対する経過時間の割合
    uint type = min((uint) (lifeRatio * frameCount), frameCount - 1); // clamping
    
    float w = 1.0 / textureSplitCount.x;
    float h = 1.0 / textureSplitCount.y;
    float2 uv = float2((type % textureSplitCount.x) * w, (type / textureSplitCount.x) * h);
    data.texcoord.xy = uv;
    data.texcoord.zw = float2(w, h);
}

// イージング関数
float Ease(float t, float easingMode)
{
    static const float PI = 3.14159265f;
    
    switch ((int) (easingMode + 0.5f))
    {
        case 0: // Linear
            return t;
        case 1: // EaseInQuad
            return t * t;
        case 2: // EaseOutQuad
            return t * (2 - t);
        case 3: // EaseInOutQuad
            return t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t;
        case 4: // EaseInCubic
            return t * t * t;
        case 5: // EaseOutCubic
            return (--t) * t * t + 1;
        case 6: // EaseInOutCubic
            return t < 0.5 ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
        case 7: // EaseInQuart
            return t * t * t * t;
        case 8: // EaseOutQuart
            return 1 - (--t) * t * t * t;
        case 9: // EaseInOutQuart
            return t < 0.5 ? 8 * t * t * t * t : 1 - 8 * (--t) * t * t * t;
        case 10: // EaseInQuint
            return t * t * t * t * t;
        case 11: // EaseOutQuint
            return 1 + (--t) * t * t * t * t;
        case 12: // EaseInOutQuint
            return t < 0.5 ? 16 * t * t * t * t * t : 1 + 16 * (--t) * t * t * t * t;
        case 13: // EaseInSine
            return 1 - cos(t * (PI / 2));
        case 14: // EaseOutSine
            return sin(t * (PI / 2));
        case 15: // EaseInOutSine
            return 0.5f * (1 - cos(PI * t));
        case 16: // EaseInExpo
            return t == 0 ? 0 : pow(2, 10 * (t - 1));
        case 17: // EaseOutExpo
            return t == 1 ? 1 : 1 - pow(2, -10 * t);
        case 18: // EaseInOutExpo
            return t == 0 ? 0 : t == 1 ? 1 : (t < 0.5f ? pow(2, 20 * t - 10) : 1 - pow(2, -20 * t + 10)) * 0.5f;
        case 19: // EaseInCirc
            return 1 - sqrt(1 - t * t);
        case 20: // EaseOutCirc
            return sqrt(1 - (--t) * t);
        case 21: // EaseInOutCirc
            return t < 0.5f ? (1 - sqrt(1 - 4 * t * t)) * 0.5f : (sqrt(1 - 4 * (--t) * t) + 1) * 0.5f;
        case 22: // EaseInBack
            return t * t * ((1.70158f + 1) * t - 1.70158f);
        case 23: // EaseOutBack
            return (--t) * t * ((1.70158f + 1) * t + 1.70158f) + 1;
        case 24: // EaseInOutBack
            return t < 0.5f ? (t * t * ((2.70158f + 1) * t - 2.70158f)) * 0.5f : ((--t) * t * ((2.70158f + 1) * t + 2.70158f) + 2) * 0.5f;
        case 25: // EaseInElastic
            return t == 0 ? 0 : t == 1 ? 1 : -pow(2, 10 * (t - 1)) * sin((t - 1.075f) * (2 * PI) / 0.3f);
        case 26: // EaseOutElastic
            return t == 0 ? 0 : t == 1 ? 1 : pow(2, -10 * t) * sin((t - 0.075f) * (2 * PI) / 0.3f) + 1;
        case 27: // EaseInOutElastic
            return t == 0 ? 0 : t == 1 ? 1 : (t < 0.5f ? -pow(2, 20 * t - 10) * sin((20 * t - 11.125f) * (2 * PI) / 0.45f) : pow(2, -20 * t + 10) * sin((20 * t - 11.125f) * (2 * PI) / 0.45f) + 1) * 0.5f;
        default:
            return t; // デフォルトは線形
    }
}

[numthreads(NumParticleThread, 1, 1)]
void main( uint3 dispatchThreadId : SV_DispatchThreadID )
{
    uint headerIndex = dispatchThreadId.x;
    
    ParticleHeader header = particleHeaderBuffer[headerIndex];
    
    uint dataIndex = header.particleIndex;
    
    //有効フラグが立っているものだけ処理
    if (header.alive == 0)
        return;
    
    ParticleData data = particleDataBuffer[dataIndex];
    
    //経過時間分減少させる
    data.parameter.y -= deltaTime;
    if (data.parameter.y < 0)
    {
        //寿命が尽きたら未使用リストに追加
        header.alive = 0;
        particleUnusedBuffer.Append(dataIndex);
        
        //　ヘッダー情報更新
        particleHeaderBuffer[headerIndex] = header;
        particleDataBuffer[dataIndex] = data;
        
        //死亡数をカウントする
        indirectDataBuffer.InterlockedAdd(IndirectArgumentsNumDeadParticle, 1);
        return;
    }
    
    // 更新処理
    {
        //速度更新
        data.velocity.xyz += data.acceleration.xyz * deltaTime;
        
        // 目標速度へのイージング
        if (data.speedEasingTime > 0)
        {
            float easingTime = data.speedEasingTime;
            float elapsedTime = easingTime - data.parameter.y; // 経過時間
            float t = saturate(elapsedTime / easingTime); // 0～1の範囲に正規化
            float easingMode = data.speedEasingMode; // イージングモード
            float easedT = Ease(t, easingMode);
            data.velocity.xyz = data.initialVelocity.xyz * lerp(data.startSpeed, data.endSpeed, easedT);
        }
        
        //位置更新
        data.position.xyz += data.velocity.xyz * deltaTime;
        
        //イージング処理
        if (data.targetPosition.w > 0)
        {
            // 位置のイージング
            float easingTime = data.targetPosition.w;
            float elapsedTime = easingTime - data.parameter.y; // 経過時間
            float t = saturate(elapsedTime / easingTime); // 0～1の範囲に正規化
            float easingMode = data.position.w; // イージングモード
            float easedT = Ease(t, easingMode);
            data.position.xyz = lerp(data.initialPosition.xyz, data.targetPosition.xyz, easedT);
        }
        if (data.targetRotation.w > 0)
        {
            // 回転のイージング
            float easingTime = data.targetRotation.w;
            float elapsedTime = easingTime - data.parameter.y; // 経過時間
            float t = saturate(elapsedTime / easingTime); // 0～1の範囲に正規化
            float easingMode = data.rotation.w; // イージングモード
            float easedT = Ease(t, easingMode);
            data.rotation.xyz = lerp(data.initialRotation.xyz, data.targetRotation.xyz, easedT);
        }
        if (data.targetScale.z > 0)
        {
            // 拡縮のイージング
            float easingTime = data.targetScale.z;
            float elapsedTime = easingTime - data.parameter.y; // 経過時間
            float t = saturate(elapsedTime / easingTime); // 0～1の範囲に正規化
            float easingMode = data.scale.z; // イージングモード
            float easedT = Ease(t, easingMode);
            data.scale.xy = lerp(data.initialScale.xy, data.targetScale.xy, easedT);
        }
        
        // ライフタイム比率算出(z:寿命、y:残り寿命)
        float lifeTime = data.parameter.z;
        float remainingLife = data.parameter.y;
        float elapsedLife = lifeTime - remainingLife;
        float lifeRatio = (lifeTime - remainingLife) / lifeTime;
        lifeRatio = saturate(lifeRatio);
        
        // UV座標の計算
        CalculateUV(data);
            
        // グラデーションテクスチャから色を取得
        if ((int) (data.parameter.w) >= 0) // グラデーションテクスチャインデックスが有効な場合
        {
            float gradientSlot = data.parameter.w; // emitterIndex (float -> arraySlice)
            float4 gradientColor = gradientTextures.SampleLevel(samplerStates[LINEAR], float2(lifeRatio, gradientSlot), 0);
            data.color = gradientColor;
        }
        else
        {
            // 開始色から終了色へ線形補間
            data.color = lerp(data.startColor, data.endColor, lifeRatio);
        }
            
        // フェードイン・アウトの処理
        if (all(data.fadeInTime) && elapsedLife < data.fadeInTime)
        {
            float fadeInRatio = elapsedLife / data.fadeInTime;
            data.color.a *= fadeInRatio; // フェードイン
        }
        if (all(data.fadeOutTime) && remainingLife < data.fadeOutTime)
        {
            float fadeOutRatio = remainingLife / data.fadeOutTime;
            data.color.a *= fadeOutRatio; // フェードアウト
        }
    }
        
    //深度ソート値算出
    header.depth = mul(float4(data.position.xyz, 1), viewProjection).w;

    //更新情報を格納
    particleHeaderBuffer[headerIndex] = header;
    particleDataBuffer[dataIndex] = data;
}