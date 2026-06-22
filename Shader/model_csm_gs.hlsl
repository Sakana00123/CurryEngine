#include "model.hlsli"

[maxvertexcount(3)]
void main(
    triangle VS_OUT_CSM          input[3],
    inout TriangleStream<GS_OUTPUT_CSM> output)
{
    GS_OUTPUT_CSM element = (GS_OUTPUT_CSM) 0;
    element.renderTargetArrayIndex = input[0].instanceId;

    element.position = input[0].position;
    element.texcoord = input[0].texcoord;
    element.depth    = input[0].position.z / input[0].position.w;
    output.Append(element);

    element.position = input[1].position;
    element.texcoord = input[1].texcoord;
    element.depth    = input[1].position.z / input[1].position.w;
    output.Append(element);

    element.position = input[2].position;
    element.texcoord = input[2].texcoord;
    element.depth    = input[2].position.z / input[2].position.w;
    output.Append(element);

    output.RestartStrip();
}
