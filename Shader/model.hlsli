#include "Constants.hlsli"
#include "Lights.hlsli"

// ============================================================
// 頂点入力構造体
// ============================================================

/// スキニングなしメッシュ（StaticVertex に対応）
struct VS_IN_STATIC
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float4 tangent  : TANGENT;  // .w = handedness (sigma)
    float2 texcoord : TEXCOORD;
};

/// スキニングありメッシュ（SkinnedVertex に対応）
struct VS_IN_SKINNED
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float4 tangent  : TANGENT;  // .w = handedness (sigma)
    float2 texcoord : TEXCOORD;
    uint4  joints   : JOINTS;
    float4 weights  : WEIGHTS;
};

// ============================================================
// 頂点出力構造体
// ============================================================

struct VS_OUT
{
    float4 position       : SV_POSITION;
    float4 world_position : POSITION;
    float4 world_normal   : NORMAL;
    float4 world_tangent  : TANGENT;   // .w = sigma
    float2 texcoord       : TEXCOORD;
};

// ============================================================
// 定数バッファ
// ============================================================

/// プリミティブごとの定数（VS/PS スロット b0）
/// C++ 側の ModelRenderer::PrimitiveConstants と一致させること
cbuffer PRIMITIVE_CONSTANT_BUFFER : register(b5)
{
    row_major float4x4 world;
    int  materialIndex; // AssetModel::materials へのインデックス
    bool has_tangent;
    int  skinIndex;     // AssetModel::skins へのインデックス（なければ -1）
    int  _pad;
};

/// スキニング用ジョイント行列（VS スロット b6）
static const uint PRIMITIVE_MAX_JOINTS = 512;
cbuffer PRIMITIVE_JOINT_CONSTANTS : register(b6)
{
    row_major float4x4 joint_matrices[PRIMITIVE_MAX_JOINTS];
};

// ============================================================
// カスケードシャドウマップ用
// ============================================================

struct VS_OUT_CSM
{
    float4 position   : SV_POSITION;
    uint   instanceId : INSTANCEID;
    float2 texcoord   : TEXCOORD;
};

struct GS_OUTPUT_CSM
{
    float4 position              : SV_POSITION;
    float2 texcoord              : TEXCOORD;
    float  depth                 : DEPTH;
    uint   renderTargetArrayIndex: SV_RENDERTARGETARRAYINDEX;
};
