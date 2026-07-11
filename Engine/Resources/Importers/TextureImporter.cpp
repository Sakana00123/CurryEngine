#include "pch.h"
#include "TextureImporter.h"
#include <Engine\Resources\Texture.h>
#include <Engine\Resources\ImportSettings\TextureImportSettings.h>
#include "Engine/Resources/AssetMeta.h"

#include <filesystem>
#include <fstream>

#include <DirectXTex.h>

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
			// アーティファクトのパスを生成
			std::filesystem::path artifactStem = meta.id.id;
			std::filesystem::path artifactPath(EnginePaths::ArtifactDir / artifactStem);
			artifactPath.replace_extension("");

			// ファイルの拡張子を小文字に変換して取得
			std::filesystem::path filePath(meta.path);
			std::string extension = filePath.extension().string();
			std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
			HRESULT hr = S_OK;

			// インポート設定の取得
			TextureImportSettings settings = meta.GetImportSettings<TextureImportSettings>();

			// 読み込み関数の引数の設定
			ID3D11Device* device = Graphics::GetDevice();
			std::wstring						filePathW = filePath.wstring();

			DirectX::ScratchImage scratchImage;

			// アーティファクトが存在しない場合、元のテクスチャファイルから読み込む
			if (!std::filesystem::exists(artifactPath))
			{
				if (extension == ".dds")
				{
					// DirectXTex を使用して DDS テクスチャを読み込む
					hr = DirectX::LoadFromDDSFile(
						filePathW.c_str(),
						DirectX::DDS_FLAGS_NONE,
						nullptr,
						scratchImage
					);
				}
				else if (extension == ".tga")
				{
					// DirectXTex を使用して TGA テクスチャを読み込む
					hr = DirectX::LoadFromTGAFile(
						filePathW.c_str(),
						DirectX::TGA_FLAGS_NONE,
						nullptr,
						scratchImage
					);
				}
				else if (extension == ".hdr")
				{
					// DirectXTex を使用して HDR テクスチャを読み込む
					hr = DirectX::LoadFromHDRFile(
						filePathW.c_str(),
						nullptr,
						scratchImage
					);
				}
				else
				{
					// DirectXTex を使用してその他のテクスチャを読み込む
					hr = DirectX::LoadFromWICFile(
						filePathW.c_str(),
						DirectX::WIC_FLAGS_NONE,
						nullptr,
						scratchImage
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

				// DirectXTex を使用して ID3D11Resource を作成する
				hr = DirectX::CreateTexture(
					device,
					scratchImage.GetImages(),
					scratchImage.GetImageCount(),
					scratchImage.GetMetadata(),
					textureResource.ReleaseAndGetAddressOf()
				);
				if (FAILED(hr))
				{
					std::string hrStr = "HRESULT: " + std::to_string(hr);
					std::u8string hrU8Str(hrStr.begin(), hrStr.end());
					std::u8string errorMsg = u8"テクスチャリソース作成失敗: " + meta.path.u8string() + u8", " + hrU8Str;
					LOG_ERROR(errorMsg);
					return false;
				}

				// ID3D11ShaderResourceView を作成する
				hr = device->CreateShaderResourceView(
					textureResource.Get(),
					nullptr,
					textureView.ReleaseAndGetAddressOf()
				);
				if (FAILED(hr))
				{
					std::string hrStr = "HRESULT: " + std::to_string(hr);
					std::u8string hrU8Str(hrStr.begin(), hrStr.end());
					std::u8string errorMsg = u8"シェーダーリソースビュー作成失敗: " + meta.path.u8string() + u8", " + hrU8Str;
					LOG_ERROR(errorMsg);
					return false;
				}

				// 成功した場合、テクスチャをアーティファクトとして保存する
				std::filesystem::path artifactStem = meta.id.id;
				std::filesystem::path artifactPath(EnginePaths::ArtifactDir / artifactStem);
				artifactPath.replace_extension("");

				// 中身はDDS形式で保存する
				hr = DirectX::SaveToDDSFile(
					scratchImage.GetImages(),
					scratchImage.GetImageCount(),
					scratchImage.GetMetadata(),
					DirectX::DDS_FLAGS_NONE,
					artifactPath.wstring().c_str()
				);
				if (FAILED(hr))
				{
					std::string hrStr = "HRESULT: " + std::to_string(hr);
					std::u8string hrU8Str(hrStr.begin(), hrStr.end());
					std::u8string errorMsg = u8"アーティファクト保存失敗: " + artifactPath.u8string() + u8", " + hrU8Str;
					LOG_ERROR(errorMsg);
					return false;
				}
			}
			else
			{
				// アーティファクトが存在する場合、DDSファイルとしてテクスチャを読み込む
				hr = DirectX::LoadFromDDSFile(
					artifactPath.wstring().c_str(),
					DirectX::DDS_FLAGS_NONE,
					nullptr,
					scratchImage
				);
				if (FAILED(hr))
				{
					std::string hrStr = "HRESULT: " + std::to_string(hr);
					std::u8string hrU8Str(hrStr.begin(), hrStr.end());
					std::u8string errorMsg = u8"アーティファクト読み込み失敗: " + artifactPath.u8string() + u8", " + hrU8Str;
					LOG_ERROR(errorMsg);
					return false;
				}
				// DirectXTex を使用して ID3D11Resource を作成する
				hr = DirectX::CreateTexture(
					device,
					scratchImage.GetImages(),
					scratchImage.GetImageCount(),
					scratchImage.GetMetadata(),
					textureResource.ReleaseAndGetAddressOf()
				);
				if (FAILED(hr))
				{
					std::string hrStr = "HRESULT: " + std::to_string(hr);
					std::u8string hrU8Str(hrStr.begin(), hrStr.end());
					std::u8string errorMsg = u8"アーティファクトからのテクスチャリソース作成失敗: " + artifactPath.u8string() + u8", " + hrU8Str;
					LOG_ERROR(errorMsg);
					return false;
				}
				// ID3D11ShaderResourceView を作成する
				hr = device->CreateShaderResourceView(
					textureResource.Get(),
					nullptr,
					textureView.ReleaseAndGetAddressOf()
				);
				if (FAILED(hr))
				{
					std::string hrStr = "HRESULT: " + std::to_string(hr);
					std::u8string hrU8Str(hrStr.begin(), hrStr.end());
					std::u8string errorMsg = u8"アーティファクトからのシェーダーリソースビュー作成失敗: " + artifactPath.u8string() + u8", " + hrU8Str;
					LOG_ERROR(errorMsg);
					return false;
				}
			}
			return true;
		}
	}
}
