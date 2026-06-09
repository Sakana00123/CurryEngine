#include "pch.h"
#include "Skymap.h"
#include "Engine/Resources/Texture.h"
#include "Engine/Resources/Shader.h"

#include "Engine/Core/Misc.h"
#include "Engine/Editor/Dialog.h"
#include "Engine/Resources/ResourceManager.h"

Skymap::Skymap(ID3D11Device* device)
{
	// デフォルトのスカイマップテクスチャを読み込み
	texture = ResourceManager::GetOrLoad<AssetTexture>("./Assets/environments/skybox.dds");
	const auto& texture2dDesc = texture->GetDesc();
	if (texture2dDesc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
	{
		isTextureCube = true;
	}
	else
	{
		isTextureCube = false;
	}
	
	std::string dir = EnginePaths::ShadersDataDir;
	CreateVertexShaderFromCSO(device,(dir + "skymap_vs.cso").c_str(), skymap_vs.GetAddressOf(), NULL, NULL, 0);
	CreatePixelShaderFromCSO(device, (dir + "skymap_ps.cso").c_str(), skymap_ps.GetAddressOf());
	CreatePixelShaderFromCSO(device, (dir + "skybox_ps.cso").c_str(), skybox_ps.GetAddressOf());
}

void Skymap::Draw(ID3D11DeviceContext* immediateContext)
{
	immediateContext->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	immediateContext->IASetInputLayout(NULL);

	immediateContext->VSSetShader(skymap_vs.Get(), 0, 0);
	immediateContext->PSSetShader(isTextureCube ? skybox_ps.Get() : skymap_ps.Get(), 0, 0);

	immediateContext->PSSetShaderResources(0, 1, texture->GetSRVAddress());

	immediateContext->Draw(4, 0);

	immediateContext->VSSetShader(NULL, 0, 0);
	immediateContext->PSSetShader(NULL, 0, 0);
}

void Skymap::DrawProperty()
{
#ifdef USE_IMGUI
	if (ImGui::TreeNode("Skymap"))
	{
		std::string texturePath = texture ? texture->GetPath() : "None";
		ImGui::Text("Texture: %s", texturePath.c_str());
		// 画像のプレビュー
		if (texture && texture->GetSRV())
		{
			// ImageButton でプレビュー表示
			if (ImGui::ImageButton("##SkymapTexturePreview", ImTextureRef((ImTextureID)texture->GetSRV()),
				ImVec2(128, 128), ImVec2(0, 0), ImVec2(1, 1)))
			{
				// クリックされたら新しいテクスチャを選択
				std::string newFilePath = OpenFileDialog("Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.dds\0All Files\0*.*\0");
				if (!newFilePath.empty())
				{
					// テクスチャを再読み込み
					texture = ResourceManager::Load<AssetTexture>(newFilePath);
					// シェーダリソースビューを更新
					const auto& texture2dDesc = texture->GetDesc();
					if (texture2dDesc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
					{
						isTextureCube = true;
					}
					else
					{
						isTextureCube = false;
					}
				}
			}
		}
		else
		{
			ImGui::Text("No texture loaded.");
		}

		ImGui::TreePop();
	}
#endif // DEBUG

}

json Skymap::Serialize() const
{
	json j;
	std::string filePath = texture->GetPath();
	if (!filePath.empty())
	{
		j["filePath"] = filePath;
	}
	return j;
}

void Skymap::Deserialize(const json& j)
{
	std::string filePath = j.value("filePath", "./Assets/environments/skybox.dds");
	// テクスチャを再読み込み
	texture = ResourceManager::GetOrLoad<AssetTexture>(filePath);
}