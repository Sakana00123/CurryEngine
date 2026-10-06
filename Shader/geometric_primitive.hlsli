#include "Constants.hlsli"

struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

cbuffer OBJECT_CONSTANT_BUFFER : register(b0)
{
    row_major float4x4 world;
    float4 materialColor;
};

struct VS_IN
{
    float4 position : POSITION;
    float2 texcoord : TEXCOORD;
};

// CASCADED_SHADOW_MAPS
struct VS_OUT_CSM
{
    float4 position : SV_POSITION;
    uint instanceId : INSTANCEID;
    float2 texcoord : TEXCOORD;
};
struct GS_OUTPUT_CSM
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
    float depth : DEPTH;
    uint renderTargetArrayIndex : SV_RENDERTARGETARRAYINDEX;
};
