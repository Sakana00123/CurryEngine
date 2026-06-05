#ifndef CONSTANTS_INCLUDE
#define CONSTANTS_INCLUDE
cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 view;
    row_major float4x4 projection;
    row_major float4x4 viewProjection;
    row_major float4x4 inverseView;
    row_major float4x4 inverseProjection;
    row_major float4x4 inverseViewProjection;
    float4 cameraPositon;
    float time; // 経過時間
    float deltaTime; // 前フレームからの経過時間
    float unscaledDeltaTime; // 前フレームからの経過時間（TimeScaleの影響を受けない）
    float pad;
    float2 screenSize; // スクリーンサイズ
    float2 padding;
}

#endif