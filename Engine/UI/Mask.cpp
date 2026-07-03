#include "pch.h"
#include "Mask.h"

REGISTER_COMPONENT(Mask, "UI");

D3D11_RECT Mask::GetScissorRect() const
{
	D3D11_RECT scissorRect{};
	if (RectTransform* rect = GetOwner()->GetComponent<RectTransform>())
	{
		scissorRect.left = static_cast<LONG>(rect->TopLeft().x);
		scissorRect.top = static_cast<LONG>(rect->TopLeft().y);
		scissorRect.right = static_cast<LONG>(rect->BottomRight().x);
		scissorRect.bottom = static_cast<LONG>(rect->BottomRight().y);
	}
	return scissorRect;
}

//
//void Mask::Begin(RenderContext* rtx)
//{
//#if 0
//	// maskRectが設定されていなかったら何もしない
//	if (!maskRect) 
//	{
//		return;
//	}
//
//	ID3D11DeviceContext* immediateContext = rtx->immediateContext;
//
//	Graphics::GetRenderState()->BindRasterizerState(immediateContext, RasterizerState::UseScissorRects);
//	D3D11_VIEWPORT viewport{};
//	UINT numViewports{ 1 };
//	immediateContext->RSGetViewports(&numViewports, &viewport);
//	XMFLOAT2 _pos = maskRect->GetWorldPosition();
//	XMFLOAT2 _size = maskRect->GetWorldSize();
//	D3D11_RECT scissorRect{};
//	scissorRect.left = static_cast<LONG>(_pos.x);
//	scissorRect.top = static_cast<LONG>(_pos.y);
//	scissorRect.right = static_cast<LONG>(_pos.x + _size.x);
//	scissorRect.bottom = static_cast<LONG>(_pos.y + _size.y);
//	immediateContext->RSSetScissorRects(1, &scissorRect);
//#endif
//}
//
//void Mask::End(RenderContext* rtx)
//{
//	/*ID3D11DeviceContext* immediateContext = rtx->immediateContext;
//	Graphics::GetRenderState()->BindRasterizerState(immediateContext, RasterizerState::SolidCullBack);*/
//}
//
//void Mask::DrawProperty()
//{
//	//maskRect->DrawProperty();
//}