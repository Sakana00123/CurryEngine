#include "particle.hlsli"

VS_OUT main(uint vertexId : SV_VERTEXID)
{
    VS_OUT vout;
    vout.vertexId = vertexId;
	return vout;
}