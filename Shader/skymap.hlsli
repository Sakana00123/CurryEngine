#include "Constants.hlsli"

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

struct VS_OUT_CUBOID
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD;
    float3 bearing : BEARING;
    uint instance_id : SV_INSTANCEID;
};
struct GS_OUT_CUBOID
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
    float3 bearing : BEARING;
    uint sv_render_target_array_index : SV_RENDERTARGETARRAYINDEX;
};