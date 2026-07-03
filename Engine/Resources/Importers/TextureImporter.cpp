#include "pch.h"
#include "TextureImporter.h"
#include <Engine\Resources\Texture.h>
#include <Engine\Resources\ImportSettings\TextureImportSettings.h>
#include "Engine/Resources/AssetMeta.h"

#include <filesystem>

#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>

#include <wrl.h>
using namespace Microsoft::WRL;

#include "Engine/Rendering/Pipeline/Graphics.h"
#include <Engine\Resources\ResourceManager.h>

namespace CurryEngine
{
	namespace Resources
	{
		std::shared_ptr<Resource> TextureImporter::Import(const AssetMeta& meta)
		{
			std::shared_ptr<AssetTexture> texture = std::make_shared<AssetTexture>();

			ComPtr<ID3D11ShaderResourceView> textureView;
			ComPtr<ID3D11Resource> textureResource;
			if (!LoadTextureFromFile(meta, textureView, textureResource))
			{
				LOG_ERROR(u8"テクスチャのインポートに失敗しました: " + meta.path.u8string());
				return nullptr;
			}
			ComPtr<ID3D11Texture2D> texture2D;
			HRESULT hr = textureResource.Get()->QueryInterface<ID3D11Texture2D>(texture2D.GetAddressOf());
			if (FAILED(hr))
			{
				std::string hrStr = "HRESULT: " + std::to_string(hr);
				std::u8string hrU8Str(hrStr.begin(), hrStr.end());
				std::u8string errorMsg = u8"リソースから ID3D11Texture2D を取得できませんでした: " + meta.path.u8string() + u8", " + hrU8Str;
				LOG_ERROR(errorMsg);
				return nullptr;
			}
			D3D11_TEXTURE2D_DESC desc;
			texture2D->GetDesc(&desc);

			texture->SetSRV(textureView.Get(), desc);
			return texture;
		}
		std::vector<std::string> TextureImporter::GetSupportedExtensions() const
		{
			return { ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".dds" };
		}

		bool TextureImporter::LoadTextureFromFile(const AssetMeta& meta, ComPtr<ID3D11ShaderResourceView>& textureView, ComPtr<ID3D11Resource>& textureResource)
		{
			std::filesystem::path filePath(meta.path);
			std::string extension = filePath.extension().string();
			std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
			HRESULT hr = S_OK;

			// インポート設定の取得
			TextureImportSettings settings = meta.GetImportSettings<TextureImportSettings>();

			// 読み込み関数の引数の設定
			ID3D11Device*						device			= Graphics::GetDevice();
			std::wstring						filePathW		= filePath.wstring();
			size_t								maxsize			= 0; // 0は制限なし
			D3D11_USAGE							usage			= D3D11_USAGE_DEFAULT;
			unsigned int						bindFlags		= D3D11_BIND_SHADER_RESOURCE;
			unsigned int						cpuAccessFlags	= 0;
			unsigned int						miscFlags		= 0;
			DirectX::WIC_LOADER_FLAGS			wicLoadFlags	= DirectX::WIC_LOADER_DEFAULT;
			DirectX::DX11::DDS_LOADER_FLAGS		ddsLoadFlags	= DirectX::DDS_LOADER_DEFAULT;
			DDS_ALPHA_MODE						alphaMode		= DDS_ALPHA_MODE_UNKNOWN;

			if (extension == ".dds")
			{
				// DirectX::CreateDDSTextureFromFileEx を使用してDDSテクスチャを読み込む
				hr = DirectX::CreateDDSTextureFromFileEx(
					device,
					filePathW.c_str(),
					maxsize,
					usage,
					bindFlags,
					cpuAccessFlags,
					miscFlags,
					ddsLoadFlags,
					textureResource.ReleaseAndGetAddressOf(),
					textureView.ReleaseAndGetAddressOf(),
					&alphaMode
				);
			}
			else
			{
				// DirectX::CreateWICTextureFromFileEx を使用してその他のテクスチャを読み込む
				hr = DirectX::CreateWICTextureFromFileEx(
					device,
					filePathW.c_str(),
					maxsize,
					usage,
					bindFlags,
					cpuAccessFlags,
					miscFlags,
					wicLoadFlags,
					textureResource.ReleaseAndGetAddressOf(),
					textureView.ReleaseAndGetAddressOf()
				);
			}
			if (FAILED(hr))
			{
				std::string hrStr = "HRESULT: " + std::to_string(hr);
				std::u8string hrU8Str(hrStr.begin(), hrStr.end());
				std::u8string errorMsg = u8"テクスチャ読み込み失敗: " + meta.path.u8string() + u8", " + hrU8Str;
				LOG_ERROR(errorMsg);
				return false;
			}
			return true;
		}
	}
}