#include "skymap.hlsli"
#include "GBuffer.hlsli"
#include "ShaderFunctions.hlsli"
#include "Lights.hlsli"

Texture2D texture0 : register(t0); // latitude-longitude mapped texture
SamplerState sampler0 : register(s0);

PSGBufferOut main(VS_OUT pin)
{
    // 視線ベクトル
    float3 E = normalize(pin.position.xyz - cameraPositon.xyz); // 視線ベクトル
    
    // スカイボックスから色を取得する
    float3 baseColor = SampleSkybox(texture0, sampler0, E);
    
    // 環境光の色を適用しておく
    float3 ambient = ambientColor.rgb * ambientColor.a;
    
    // GBufferDataに出力情報をまとめる
    GBufferData data = (GBufferData) 0;
    data.shadingModel = shaderModelUnlit; // ライティング無し
    data.emissiveColor = ambient * baseColor.rgb; // 環境光を自己発光色として扱う
    data.wPosition = pin.position.xyz; // 一応ワールド座標を入れておく
    data.wNormal = -E; // 視線方向を法線として扱う
    
    return EncodeGBuffer(data, viewProjection);
}