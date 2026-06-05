#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/GameObject.h"

#if 0
//#define NOMINMAX
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
//#define TINYGLTF_NO_EXTERNAL_IMAGE
//#define TINYGLTF_NO_STB_IMAGE
//#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>
#else
#include "Archive/gltf_model.h"
#endif
#include "Engine/Core/Color.h"

#include "Engine/Resources/Shader.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

class ModelRenderer : public Component
{
public:
	ModelRenderer(ID3D11Device* device, const std::string& filePath);
	virtual ~ModelRenderer() override = default;

	void Update(float elapsedTime) override;

	void Render(RenderContext* rtx) override;

	void DrawProperty() override;

public:
	//置き換えるピクセルシェーダー設定（※全体にかかる）
	void SetReplacePixelShader(const char* filePath) {
		CreatePixelShaderFromCSO(Graphics::GetDevice(), filePath, replacePixelShader.GetAddressOf());
	}
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	//置き換える頂点シェーダー設定
	void SetReplaceVertexShader(const char* filePath) {
		// TODO: This is a force-brute programming, may cause bugs.
		const std::map<std::string, GltfModel::BufferView>& vertexBufferViews{
			model->meshes.at(0).primitives.at(0).vertexBufferViews
		};
		D3D11_INPUT_ELEMENT_DESC input_element_desc[]
		{
			{ "POSITION", 0, vertexBufferViews.at("POSITION").format, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "NORMAL", 0, vertexBufferViews.at("NORMAL").format, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TANGENT", 0, vertexBufferViews.at("TANGENT").format, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, vertexBufferViews.at("TEXCOORD_0").format, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "JOINTS", 0, vertexBufferViews.at("JOINTS_0").format, 4, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "WEIGHTS", 0, vertexBufferViews.at("WEIGHTS_0").format, 5, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		CreateVertexShaderFromCSO(Graphics::GetDevice(), filePath, replaceVertexShader.ReleaseAndGetAddressOf(),
			inputLayout.GetAddressOf(), input_element_desc, _countof(input_element_desc));
	}

	void SetLoop(bool loop) { model->loop = loop; }

	void SetAnimation(int index) { _ASSERT_EXPR(GetMaxAnimations() > index, L"インデックス値が範囲外です。"); model->SetAnimation(index); }
	void SetAnimation(const std::string& name) {
		model->SetAnimation(GetAnimationIndex(name));
	}
	int GetAnimationIndex(const std::string& name) const {
		for (int i = 0; i < GetMaxAnimations(); i++) {
			if (model->animations[i].name == name) {
				return i;
			}
		}
		return -1;
	}
	//現在再生中のアニメーションの名前取得
	std::string GetCurrentAnimationName() const {
		if (model->animations.size() > model->animationIndex) {
			return model->animations[model->animationIndex].name;
		}
		return "";
	}

	int GetMaxAnimations() const { return static_cast<int>(model->animations.size()); }

	GltfModel::Node* FindNode(const std::string& name) const { return model->FindNode(name); }

	bool IsAnimationCompleted() const { return model->IsAnimationCompleted(); }

	bool animationEneble;
	Color color{ 1,1,1,1 };

	std::unique_ptr<GltfModel> model;
public:
	//std::string filePath;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> replacePixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> replaceVertexShader;

};