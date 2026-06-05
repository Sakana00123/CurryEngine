#include "pch.h"
#include "Texture.h"
#include <memory>
#include <filesystem>
using namespace std;

#include "Engine/Rendering/Pipeline/Graphics.h"

HRESULT LoadTextureFromFile(ID3D11Device* device, const wchar_t* filename,
	ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc)
{
	HRESULT hr{ S_OK };
	ComPtr<ID3D11Resource> resource;

	// TODO: ResourceManagerにキャッシュ機能があるので、そちらに統合する。（この関数が色んな所で呼ばれているため、ResourceManagerに統合するのが大変）
	auto it = resources.find(filename);
	if (it != resources.end())
	{
		*shader_resource_view = it->second.Get();
		(*shader_resource_view)->AddRef();
		(*shader_resource_view)->GetResource(resource.GetAddressOf());
	}
	else
	{
		std::filesystem::path ddsFilename(filename);
		ddsFilename.replace_extension("dds");
		if (std::filesystem::exists(ddsFilename.c_str()))
		{
			hr = CreateDDSTextureFromFile(device, ddsFilename.c_str(), resource.GetAddressOf(), shader_resource_view);
			_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
		}
		else
		{
			hr = CreateWICTextureFromFile(device, filename, resource.GetAddressOf(), shader_resource_view);
			_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
		}
		resources.insert(make_pair(filename, *shader_resource_view));
	}

	if (texture2d_desc) {
		ComPtr<ID3D11Texture2D> texture2d;
		hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
		texture2d->GetDesc(texture2d_desc);
	}

	return hr;
}

void ReleaseAllTextures()
{
	resources.clear();
}

HRESULT MakeDummyTexture(ID3D11Device* device, ID3D11ShaderResourceView** shader_resource_view,
	DWORD value/*0xAABBGGRR*/, UINT dimension)
{
	HRESULT hr{ S_OK };

	D3D11_TEXTURE2D_DESC texture2d_desc{};
	texture2d_desc.Width = dimension;
	texture2d_desc.Height = dimension;
	texture2d_desc.MipLevels = 1;
	texture2d_desc.ArraySize = 1;
	texture2d_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture2d_desc.SampleDesc.Count = 1;
	texture2d_desc.SampleDesc.Quality = 0;
	texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
	texture2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	size_t texels = dimension * dimension;
	unique_ptr<DWORD[]> system{ make_unique<DWORD[]>(texels) };
	for (size_t i = 0; i < texels; ++i) system[i] = value;

	D3D11_SUBRESOURCE_DATA subresource_data{};
	subresource_data.pSysMem = system.get();
	subresource_data.SysMemPitch = sizeof(DWORD) * dimension;

	ComPtr<ID3D11Texture2D> texture2d;
	hr = device->CreateTexture2D(&texture2d_desc, &subresource_data, &texture2d);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{};
	shader_resource_view_desc.Format = texture2d_desc.Format;
	shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_resource_view_desc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(texture2d.Get(), &shader_resource_view_desc,
		shader_resource_view);
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	return hr;
}

HRESULT LoadTextureFromMemory(ID3D11Device* device, const void* data, size_t size, ID3D11ShaderResourceView** shaderResourceView) {
	HRESULT hr{ S_OK };
	ComPtr<ID3D11Resource> resource;

	hr = CreateDDSTextureFromMemory(device, reinterpret_cast<const uint8_t*>(data), size, resource.GetAddressOf(), shaderResourceView);
	if (hr != S_OK) {
		hr = CreateWICTextureFromMemory(device, reinterpret_cast<const uint8_t*>(data), size, resource.GetAddressOf(), shaderResourceView);
		_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
	}

	return hr;
}

bool AssetTexture::LoadFromFile(const std::string& filePath)
{
	_path = filePath;
	auto device = Graphics::GetDevice();
	if (filePath.empty())
		return MakeDummy(device);

	return Load(device, std::wstring(filePath.begin(), filePath.end()));
}

bool AssetTexture::Load(ID3D11Device* device, const std::wstring& filePath)
{
	HRESULT hr = LoadTextureFromFile(device, filePath.c_str(), m_Srv.ReleaseAndGetAddressOf(), &m_Desc);
	return SUCCEEDED(hr);
}

bool AssetTexture::MakeDummy(ID3D11Device* device, DWORD value, UINT dimension)
{
	HRESULT hr = MakeDummyTexture(device, m_Srv.ReleaseAndGetAddressOf(), value, dimension);
	return SUCCEEDED(hr);
}

void AssetTexture::Release()
{
	m_Srv.Reset();
}