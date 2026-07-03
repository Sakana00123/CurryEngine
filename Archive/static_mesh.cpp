#include "pch.h"
#include "static_mesh.h"
using namespace DirectX;
#include <fstream>
#include "Engine/Resources/Texture.h"
#include "Engine/Resources/Shader.h"
#include <filesystem>

StaticMesh::StaticMesh(ID3D11Device* device, const wchar_t* obj_filename)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	uint32_t current_index{ 0 };

	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT3> normals;
	std::vector<XMFLOAT2> texcoords;
	std::vector<std::wstring> mtl_filenames;

	bool isFlip = true;//テクスチャを反転させるかどうかのフラグ

	std::wifstream fin(obj_filename);
	_ASSERT_EXPR(fin, L"OBJ file not found.");
	wchar_t command[256];
	//OBJファイルバーサー
	{
		while (fin)
		{
			fin >> command;
			if (0 == wcscmp(command, L"v"))
			{
				float x, y, z;
				fin >> x >> y >> z;
				positions.push_back({ x,y,z });
				fin.ignore(1024, L'\n');
			}
			else if (0 == wcscmp(command, L"vn"))
			{
				float i, j, k;
				fin >> i >> j >> k;
				normals.push_back({ i,j,k });
				fin.ignore(1024, L'\n');
			}
			else if (0 == wcscmp(command, L"vt"))
			{
				float u, v;
				fin >> u >> v;
				texcoords.push_back({ u,(isFlip) ? 1.0f - v : v });
				fin.ignore(1024, L'\n');
			}
			else if (0 == wcscmp(command, L"f"))
			{
				for (size_t i = 0; i < 3; i++)
				{
					Vertex vertex;
					size_t v, vt, vn;

					fin >> v;
					vertex.position = positions.at(v - 1);
					if (L'/' == fin.peek())
					{
						fin.ignore(1);
						if (L'/' != fin.peek())
						{
							fin >> vt;
							vertex.texcoord = texcoords.at(vt - 1);
						}
						if (L'/' == fin.peek())
						{
							fin.ignore(1);
							fin >> vn;
							vertex.normal = normals.at(vn - 1);
						}
					}
					vertices.push_back(vertex);
					indices.push_back(current_index++);
				}
				fin.ignore(1024, L'\n');
			}
			else if (0 == wcscmp(command, L"mtllib"))
			{
				wchar_t mtllib[256];
				fin >> mtllib;
				mtl_filenames.push_back(mtllib);
			}
			else if (0 == wcscmp(command, L"usemtl"))
			{
				wchar_t usemtl[MAX_PATH]{ 0 };
				fin >> usemtl;
				subsets.push_back({ usemtl, static_cast<uint32_t>(indices.size()), 0 });
			}
			else
			{
				fin.ignore(1024, L'\n');
			}
		}
		fin.close();
	}
	//サブセットのインデックス数を計算する
	{
		std::vector<SubSet>::reverse_iterator iterator = subsets.rbegin();
		iterator->index_count = static_cast<uint32_t>(indices.size()) - iterator->index_start;
		for (iterator = subsets.rbegin() + 1; iterator != subsets.rend(); ++iterator)
		{
			iterator->index_count = (iterator - 1)->index_start - iterator->index_start;
		}
	}

	//バッファ生成
	CreateComBuffers(device, vertices.data(), vertices.size(), indices.data(), indices.size());

	//MTLファイルをロードし、テクスチャファイル名を取得
	{
		std::filesystem::path mtl_filename(obj_filename);
		mtl_filename.replace_filename(std::filesystem::path(mtl_filenames[0]).filename());

		fin.open(mtl_filename);
		//_ASSERT_EXPR(fin, L"MTL file not found.");

		while (fin)
		{
			fin >> command;
			if (0 == wcscmp(command, L"map_Kd"))
			{
				fin.ignore();
				wchar_t map_Kd[256];
				fin >> map_Kd;

				std::filesystem::path path(obj_filename);
				path.replace_filename(std::filesystem::path(map_Kd).filename());
				materials.rbegin()->texture_filenames[0] = path;
				fin.ignore(1024, L'\n');
			}
			else if (0 == wcscmp(command, L"map_bump") || 0 == wcscmp(command, L"bump"))
			{
				fin.ignore();
				wchar_t map_bump[256];
				fin >> map_bump;

				std::filesystem::path path(obj_filename);
				path.replace_filename(std::filesystem::path(map_bump).filename());
				materials.rbegin()->texture_filenames[1] = path;
				fin.ignore(1024, L'\n');
			}
			else if (0 == wcscmp(command, L"newmtl"))
			{
				fin.ignore();
				wchar_t newmtl[256];
				Material material;
				fin >> newmtl;
				material.name = newmtl;
				materials.push_back(material);
			}
			else if (0 == wcscmp(command, L"Kd"))
			{
				float r, g, b;
				fin >> r >> g >> b;
				materials.rbegin()->Kd = { r,g,b,1 };
				fin.ignore(1024, L'\n');
			}
			else if (0 == wcscmp(command, L"Ka"))
			{
				float r, g, b;
				fin >> r >> g >> b;
				materials.rbegin()->Ka = { r,g,b,1 };
				fin.ignore(1024, L'\n');
			}
			else if (0 == wcscmp(command, L"Ks"))
			{
				float r, g, b;
				fin >> r >> g >> b;
				materials.rbegin()->Ks = { r,g,b,1 };
				fin.ignore(1024, L'\n');
			}
			else
			{
				fin.ignore(1024, L'\n');
			}
		}
		fin.close();
	}

	//マテリアルがなかったら、ダミーマテリアルを追加
	if (materials.size() == 0)
	{
		for (const SubSet& subset : subsets)
		{
			materials.push_back({ subset.usemtl });
		}
	}

	//シェーダーリソースビューオブジェクトの生成
	D3D11_TEXTURE2D_DESC texture2d_desc{};
	
	//すべてのマテリアルのテクスチャをロードし、シェーダーリソースビューを生成する
	for (Material& material : materials)
	{
		if (material.texture_filenames[0] == L"")
		{
			//ダミーカラーマップテクスチャ作成
			MakeDummyTexture(device, material.shader_resource_views[0].GetAddressOf(), 0xFFFFFFFF, 16);
			//ダミー法線マップテクスチャ作成
			MakeDummyTexture(device, material.shader_resource_views[1].GetAddressOf(), 0xFFFF7F7F, 16);
		}
		else
		{
			LoadTextureFromFile(device, material.texture_filenames[0].c_str(),
				material.shader_resource_views[0].GetAddressOf(), &texture2d_desc);
			LoadTextureFromFile(device, material.texture_filenames[1].c_str(),
				material.shader_resource_views[1].GetAddressOf(), &texture2d_desc);
		}
	}

	//シェーダーオブジェクトの生成
	HRESULT hr{ S_OK };

	D3D11_INPUT_ELEMENT_DESC input_element_desc[]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		  D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		  D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
		  D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	CreateVertexShaderFromCSO(device, "./Data/Shaders/static_mesh_vs.cso", vertex_shader.GetAddressOf(),
		input_layout.GetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
	CreatePixelShaderFromCSO(device, "./Data/Shaders/static_mesh_ps.cso", pixel_shader.GetAddressOf());

	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(Constants);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = device->CreateBuffer(&buffer_desc, nullptr, constant_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void StaticMesh::CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertex_count,
	uint32_t* indices, size_t index_count)
{
	HRESULT hr{ S_OK };

	D3D11_BUFFER_DESC buffer_desc{};
	D3D11_SUBRESOURCE_DATA subresource_data{};
	buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertex_count);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	subresource_data.pSysMem = vertices;
	subresource_data.SysMemPitch = 0;
	subresource_data.SysMemSlicePitch = 0;
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	buffer_desc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * index_count);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	subresource_data.pSysMem = indices;
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, index_buffer.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void StaticMesh::Render(ID3D11DeviceContext* immediate_context,
	const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& material_color)
{
	uint32_t stride{ sizeof(Vertex) };
	uint32_t offset{ 0 };
	immediate_context->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);
	immediate_context->IASetIndexBuffer(index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	immediate_context->IASetInputLayout(input_layout.Get());
	//頂点シェーダーとピクセルシェーダーセット
	immediate_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
	immediate_context->PSSetShader(pixel_shader.Get(), nullptr, 0);

	for (const Material& material : materials)
	{
		//シェーダーリソースのバインド
		immediate_context->PSSetShaderResources(0, 1, material.shader_resource_views[0].GetAddressOf());
		immediate_context->PSSetShaderResources(1, 1, material.shader_resource_views[1].GetAddressOf());

		Constants data{ world, material_color };
		XMStoreFloat4(&data.matrial_color, XMLoadFloat4(&material_color) * XMLoadFloat4(&material.Kd));
		immediate_context->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
		immediate_context->VSSetConstantBuffers(0, 1, constant_buffer.GetAddressOf());

		for (const SubSet& subset : subsets)
		{
			if (material.name == subset.usemtl)
			{
				immediate_context->DrawIndexed(subset.index_count, subset.index_start, 0);
			}
		}
	}
}