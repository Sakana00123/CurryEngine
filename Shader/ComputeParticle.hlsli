#include "Constants.hlsli"

//パーティクルスレッド数
static const int NumParticleThread = 1024;

//生成パーティクル構造体
struct EmitParticleData
{
    float4 parameter; // x : 描画モード, y : 残り生存時間, z : 生存時間, w : グラデーションテクスチャインデックス(0～15)

    float4 position; // xyz: 生成座標, w: イージングモード
    float4 rotation; // xyz: 回転情報 (オイラー角), w: イージングモード
    float4 scale; // xy: 開始拡縮情報, z: イージングモード, w: 空き
    
    float4 targetPosition; // xyz: 目標座標, w: イージング時間(秒)
    float4 targetRotation; // xyz: 目標回転情報 (オイラー角), w: イージング時間(秒)
    float4 targetScale; // xy: 目標拡縮情報, z: イージング時間(秒), w: 空き

    float4 velocity; // xyz: 初速, w: 空き
    float4 acceleration; // 加速度
    
    float startSpeed; // 初速の大きさ
    float endSpeed; // 終速の大きさ
    float speedEasingMode; // 速度のイージングモード
    float speedEasingTime; // 速度のイージング時間(秒)
    
    float4 startColor; // 開始色情報
    float4 endColor; // 終了色情報
    
    float fadeInTime; //フェードイン時間
    float fadeOutTime; //フェードアウト時間
    float2 emitDummy; //空き
    
};

//パーティクル構造体
struct ParticleData
{
    float4 parameter; // x : 描画モード, y : 残り生存時間, z : 生存時間,　w : グラデーションテクスチャインデックス(0～15)

    float4 position; // xyz: 生成座標, w: イージングモード
    float4 rotation; // xyz: 回転情報 (オイラー角), w: イージングモード
    float4 scale; // xy: 拡縮情報, z: イージングモード, w: 空き
    
    float4 initialPosition; // xyz: 生成座標, w: 空き
    float4 initialRotation; // xyz: 回転情報 (オイラー角), w: 空き
    float4 initialScale; // xy: 開始拡縮情報, z: 空き, w: 空き
    
    float4 targetPosition; // xyz: 目標座標, w: イージング時間(秒)
    float4 targetRotation; // xyz: 目標回転情報 (オイラー角), w: イージング時間(秒)
    float4 targetScale; // xy: 目標拡縮情報, z: イージング時間(秒), w: 空き
    

    float4 velocity; // xyz: 速度, w: 空き
    float4 acceleration; // 加速度
    
    float4 initialVelocity; // xyz: 生成時の速度, w: 空き
    float startSpeed; // 初速の大きさ
    float endSpeed; // 終速の大きさ
    float speedEasingMode; // 速度のイージングモード
    float speedEasingTime; // 速度のイージング時間(秒)
    

    float4 texcoord; //  UV座標
    float4 color; // カラー情報
    float4 startColor; // 開始色情報
    float4 endColor; // 終了色情報
    
    
    float fadeInTime; //フェードイン時間
    float fadeOutTime; //フェードアウト時間
    float2 emitDummy; //空き
};

//パーティクルヘッダー構造体
struct ParticleHeader
{
    uint alive; //生存フラグ
    uint particleIndex; //パーティクル番号
    float depth; //深度
    uint dummy;
};

//IndirectDataBufferへのアクセス用バイトオフセット
static const uint IndirectArgumentsNumCurrentParticle = 0;
static const uint IndirectArgumentsNumPreviousParticle = 4;
static const uint IndirectArgumentsNumDeadParticle = 8;
static const uint IndirectArgumentsNumEmitParticleDispatchIndirect = 12;

//DrawIndirect用構造体
struct DrawIndirect
{
    uint vertexCountPerInstance;
    uint instanceCount;
    uint startVertexLocation;
    uint startInstanceLocation;
};
static const uint IndirectArgumentsUpdateParticleDispatchIndirect = 24;
static const uint IndirectArgumentsNumEmitParticleIndex = 36;
static const uint IndirectArgumentsNumEmitPixelParticleIndex = 40;
static const uint IndirectArgumentsDrawIndirect = 44;

//=========================================================================================
//  汎用情報
cbuffer COMPUTE_PARTICLE_COMMON_CONSTANT_BUFFER : register(b10)
{
    //float deltaTime;
    uint2 textureSplitCount;
    uint systemNumParticles;
    uint totalEmitCount;
    
    uint maxEmitParticles;
    uint3 commonDummy;
};

//バイトニックソート情報
cbuffer COMPUTE_PARTICLE_BITONIC_SORT_CONSTANT_BUFFER : register(b11)
{
    uint increment;
    uint direction;
    uint sortDummy[2];
};
static const uint BitonicSortB2Thread = 256;
static const uint BitonicSortC2Thread = 512;

//=========================================================================================
//  頂点シェーダーからジオメトリシェーダーに転送する情報
struct GS_IN
{
    uint vertexId : VERTEX_ID;
};

//  ジオメトリシェーダーからピクセルシェーダーに転送する情報
struct PS_IN
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};
