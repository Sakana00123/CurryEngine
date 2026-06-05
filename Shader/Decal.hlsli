cbuffer OBJECT_CONSTANT_BUFFER : register(b0)
{
    row_major float4x4 world;
    row_major float4x4 decalInverseProjection;
    float alpha;
    float fade;
    //row_major float4x4 inverse_transpose_world; // Correct normal transformation under non-uniform scaling
}

#include "Constants.hlsli"
