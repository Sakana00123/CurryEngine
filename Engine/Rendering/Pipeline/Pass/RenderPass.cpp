#include "pch.h"
#include "RenderPass.h"

void RenderPass::OnSizeChanged(ID3D11Device* device, uint32_t width, uint32_t height)
{
	for (RenderTexture* rt : resizableRenderTargets)
	{
		if (rt)
		{
			rt->Resize(device, width, height);
		}
	}
}

void RenderPass::RegisterResizableRenderTexture(RenderTexture* rt)
{
	if (rt)
	{
		resizableRenderTargets.push_back(rt);
	}
}