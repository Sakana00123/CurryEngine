#include "pch.h"
#include "ModelRenderer.h"
#include "Engine/Core/Transform.h"
#if 0
//#define TINYGLTF_IMPLEMENTATION
#include "../tinygltf-release/tiny_gltf.h"
#include "Misc.h"

#include <stack>
#include <functional>
#include <map>
#include <unordered_map>

#include "Shader.h"
#include "Texture.h"

#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#endif

ModelRenderer::ModelRenderer(ID3D11Device* device, const std::string& filePath)
{
	model = std::make_unique<GltfModel>(device, filePath);
	animationEneble = model->animations.size();
}

void ModelRenderer::Update(float elapsedTime)
{
	if (animationEneble) {
		model->UpdateAnimation(elapsedTime);
	}
}

void ModelRenderer::Render(RenderContext* rtx)
{
	model->Render(rtx->immediateContext, gameObject->transform->GetWorld(), {}, replacePixelShader.Get(), replaceVertexShader.Get());
}

void ModelRenderer::DrawProperty()
{
#ifdef USE_IMGUI
	ImGui::Checkbox("AnimationEnable", &animationEneble);
	ImGui::Checkbox("isLoop", &model->loop);
	ImGui::SliderFloat("animationBlendTime", &model->animationBlendTime, 0.f, 20.f);
	int i = 0;
	for (GltfModel::Animation& animation : model->animations) {
		ImGui::Text(std::to_string(i).c_str());
		ImGui::SameLine();
		if (ImGui::Button(animation.name.c_str())) {
			SetAnimation(animation.name);
		}
		i++;
	}
	//ImGui::ColorEdit4("Color", &color.r);
#endif // USE_IMGUI
}