#include "ComputeParticle.hlsli"

//拡大行列生成
float4x4 MatrixScaling(float3 scale)
{
    float4x4 m;
    m._11 = scale.x;
    m._12 = 0.0f;
    m._13 = 0.0f;
    m._14 = 0.0f;
	
    m._21 = 0.0f;
    m._22 = scale.y;
    m._23 = 0.0f;
    m._24 = 0.0f;
	
    m._31 = 0.0f;
    m._32 = 0.0f;
    m._33 = scale.z;
    m._34 = 0.0f;
    
    m._41 = 0.0f;
    m._42 = 0.0f;
    m._43 = 0.0f;
    m._44 = 1.0f;
    return m;
}

//回転行列生成
float4x4 MatrixRotationRollPitchYaw(float3 rotation)
{
    float cp, sp;
    float cy, sy;
    float cr, sr;
    sincos(rotation.x, sp, cp);
    sincos(rotation.y, sy, cy);
    sincos(rotation.z, sr, cr);
    
    float4x4 m;
    m._11 = cr * cy + sr * sp * sy;
    m._12 = sr * cp;
    m._13 = sr * sp * cy - cr * sy;
    m._14 = 0.0f;
	
    m._21 = cr * sp * sy - sr * cy;
    m._22 = cr * cp;
    m._23 = sr * sy + cr * sp * cy;
    m._24 = 0.0f;
	
    m._31 = cp * sy;
    m._32 = -sp;
    m._33 = cp * cy;
    m._34 = 0.0f;
    
    m._41 = 0.0f;
    m._42 = 0.0f;
    m._43 = 0.0f;
    m._44 = 1.0f;
    return m;
}

//移動行列生成
float4x4 MatrixTranslation(float3 translation)
{
    float4x4 m;
    m._11 = 1.0f;
    m._12 = 0.0f;
    m._13 = 0.0f;
    m._14 = 0.0f;
	
    m._21 = 0.0f;
    m._22 = 1.0f;
    m._23 = 0.0f;
    m._24 = 0.0f;
	
    m._31 = 0.0f;
    m._32 = 0.0f;
    m._33 = 1.0f;
    m._34 = 0.0f;
    
    m._41 = translation.x;
    m._42 = translation.y;
    m._43 = translation.z;
    m._44 = 1.0f;
    return m;
}

//パーティクルバッファ
StructuredBuffer<ParticleData> particleDataBuffer : register(t0);
StructuredBuffer<ParticleHeader> particleHeaderBuffer : register(t1);

[maxvertexcount(4)]
void main(point GS_IN gin[1], inout TriangleStream<PS_IN> output)
{
    uint vertexId = gin[0].vertexId;
    
    //ヘッダーのParticleIndexがそのままパーティクルデータバッファのindexになる。
    bool isAlive = particleHeaderBuffer[vertexId].alive != 0;
    uint particleIndex = particleHeaderBuffer[vertexId].particleIndex;
    
    //残り生存時間と誕生時の生存時間から割合を計算する。
    float lifeTimeRate = !isAlive ? 0 : particleDataBuffer[particleIndex].parameter.y / particleDataBuffer[particleIndex].parameter.z;
    
    //生存していない場合はスケールを０にしておく
    //float2 size = !isAlive ? float2(0, 0) : lerp(particleDataBuffer[particleIndex].scale.zw, particleDataBuffer[particleIndex].scale.xy, lifeTimeRate);
    float2 size = !isAlive ? float2(0, 0) : particleDataBuffer[particleIndex].scale.xy;
    
    //ビルボード行列生成（ビュー行列の逆行列でいい。ただし移動値はいらない）
    //TODO: InverseView行列を定数バッファに設定する。
    float4x4 billbordMatrix = inverseView;
    billbordMatrix._41_42_43 = float3(0, 0, 0);
    billbordMatrix._44 = 1.0f;
    
    //ワールド行列生成
    float4x4 scaleMatrix = MatrixScaling(float3(size, 1));
    
    //回転行列生成
    float4x4 rotationMatrix = (float4x4) 0;
    
    //各種情報取得
    float4 texcoord = particleDataBuffer[particleIndex].texcoord;
    float4 color = particleDataBuffer[particleIndex].color;
    
    //頂点生成
    static const float4 vertexPositions[4] =
    {
        float4(-0.5f, -0.5f, 0, 1),
        float4(+0.5f, -0.5f, 0, 1),
        float4(-0.5f, +0.5f, 0, 1),
        float4(+0.5f, +0.5f, 0, 1),
    };
    static const float2 vertexTexcoords[4] =
    {
        float2(0, 0),
        float2(1, 0),
        float2(0, 1),
        float2(1, 1),
    };
    
    //描画モードによって回転行列を変更
    int renderMode = (int) (particleDataBuffer[particleIndex].parameter.x + 0.5f);
    // ============================================================
    // ScreenSpace (case 3) は専用パスで出力して終わり
    // ============================================================
    [branch]
    if (renderMode == 3)
    {
        float4 clipPos = mul(float4(particleDataBuffer[particleIndex].position.xyz, 1.0f), viewProjection);
        float2 ndc = clipPos.xy / clipPos.w;
        float2 ndcSize = size / screenSize * 2.0f;

        float sinR, cosR;
        sincos(particleDataBuffer[particleIndex].rotation.z, sinR, cosR);

        [unroll]
        for (uint i = 0; i < 4; i++)
        {
            float2 local = vertexPositions[i].xy;
            float2 rotated = float2(
                local.x * cosR - local.y * sinR,
                local.x * sinR + local.y * cosR);
            float2 screenPos = ndc + rotated * ndcSize;

            PS_IN element;
            element.position = float4(screenPos * clipPos.w, clipPos.z / clipPos.w * clipPos.w, clipPos.w);
            element.texcoord = texcoord.xy + texcoord.zw * vertexTexcoords[i];
            element.color = color;
            output.Append(element);
        }
        output.RestartStrip();
    }
    else
    // ============================================================
    // Billboard / StretchedBillboard / FixedRotation は共通パス
    // ============================================================
    {
        switch (renderMode)
        {
            case 0: // Billboard
                rotationMatrix = mul(billbordMatrix, MatrixRotationRollPitchYaw(particleDataBuffer[particleIndex].rotation.xyz));
                break;
        
            case 1: // StretchedBillboard
            {
                float3 velocity = particleDataBuffer[particleIndex].velocity;
                float speed = length(velocity);
                float3 dir = speed > 1e-5f ? normalize(velocity) : float3(0, 0, 1);

                float3 camRight = normalize(inverseView._m00_m01_m02);
                float3 camUp = normalize(cross(dir, camRight));
                camRight = normalize(cross(camUp, dir));

                rotationMatrix = float4x4(
                    float4(camRight, 0),
                    float4(camUp, 0),
                    float4(dir, 0),
                    float4(0, 0, 0, 1)
                );
                break;
            }
            case 2: // FixedRotation
                rotationMatrix = MatrixRotationRollPitchYaw(particleDataBuffer[particleIndex].rotation.xyz);
                break;
        }
        float4x4 translationMatrix = MatrixTranslation(particleDataBuffer[particleIndex].position.xyz);
        float4x4 worldMatrix = mul(mul(scaleMatrix, rotationMatrix), translationMatrix);
        float4x4 worldViewProjectionMatrix = mul(worldMatrix, viewProjection);
    
        for (uint i = 0; i < 4; i++)
        {
            PS_IN element;
            element.position = mul(vertexPositions[i], worldViewProjectionMatrix);
            element.texcoord = texcoord.xy + texcoord.zw * vertexTexcoords[i];
            element.color = color;
            output.Append(element);
        }
        output.RestartStrip();
    }
}