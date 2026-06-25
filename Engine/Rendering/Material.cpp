#include "pch.h"
#include "Material.h"
#include "Engine/Editor/Console.h"
#include "Engine/Editor/Dialog.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Rendering/Pipeline/RenderContext.h"
#include "Engine/Editor/FileOpener.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

// 16バイトアラインメントにサイズを調整
size_t Align16(size_t size)
{
	// 16の倍数に切り上げ
	return (size + 15) & ~15;
}

void Material::SetShader(ID3D11Device* device, std::shared_ptr<Shader> shader)
{
	// シェーダステージ別のシェーダ格納配列に設定
	ShaderBinding& binding = m_ShaderBindings[static_cast<size_t>(shader->GetType())];

	// シェーダを設定
	binding.shader = shader;

	// Shaderのオーナーを設定
	shader->SetOwner(this);

	// シェーダのリフレクション情報を取得
	const ShaderReflectionData& reflection = shader->GetReflectionData();

	// 定数バッファの設定を更新
	UpdateCBufferBindings(device, binding, reflection);

	// テクスチャとサンプラの設定を更新
	UpdateTextureAndSamplerBindings(device, binding, reflection);

	// シェーダが変更されたのでフラグをクリア
	shader->SetDirty(false);
}

void Material::Reload(ID3D11Device* device)
{
	// 全シェーダタイプについて再読み込みを試みる
	for (size_t i = 0; i < static_cast<size_t>(ShaderType::EnumCount); ++i)
	{
		// シェーダが設定されていなければスキップ
		if (!m_ShaderBindings[static_cast<size_t>(i)].shader) continue;
		// 変更されていなければスキップ
		if (!m_ShaderBindings[static_cast<size_t>(i)].shader->IsDirty()) continue;
		// 再読み込み
		SetShader(device, m_ShaderBindings[static_cast<size_t>(i)].shader);
	}
}

std::shared_ptr<Shader> Material::GetShader(ShaderType type)
{
	return m_ShaderBindings[static_cast<size_t>(type)].shader;
}

void Material::SetTexture(const std::string& name, std::shared_ptr<Texture> texture)
{
	// テクスチャを設定
	textures[name] = texture;
	// 変更フラグを立てる
	m_IsTextureChanged = true;
}

void Material::SetValue(const std::string& name, void* value, size_t size)
{
	values[name] = std::vector<uint8_t>((uint8_t*)value, (uint8_t*)value + size);
	// 変更フラグを立てる
	for (auto& [cbName, cbData] : cbuffers)
	{
		cbData.dirty = true;
	}

	//// Shaderから変数情報を探す
	//for (ShaderBinding& binding : m_ShaderBindings)
	//{
	//	for (auto& [cbName, cbData] : binding.cbuffers)
	//	{
	//		if (!binding.shader) continue;
	//		//定数バッファレイアウトを取得
	//		const ShaderReflectionData::ConstantBufferLayout* layout = binding.shader->GetConstantBufferLayout(cbName);
	//		//存在しない場合はスキップ
	//		if (!layout) continue;

	//		//対象変数を探す
	//		auto it = std::find_if(layout->variables.begin(), layout->variables.end(),
	//			[&](const ShaderReflectionData::ShaderVariable& var) {return var.name == name; });
	//		if (it == layout->variables.end()) continue;
	//		//CPU側バッファに書き込み
	//		memcpy(cbData.localData.data() + it->offset, value, size);
	//		cbData.dirty = true;
	//		return;
	//	}
	//}
	////見つからなかった場合の警告
	//Console::LogWarning("Warning: Material::SetValue failed. Variable " + name + " not found.");
}

void Material::GetValue(const std::string& name, void* value, size_t size) const
{
	if (values.find(name) != values.end())
	{
		memcpy(value, values.at(name).data(), size);
	}

	//// Shaderから変数情報を探す
	//for (const ShaderBinding& binding : m_ShaderBindings)
	//{
	//	for (auto& [cbName, cbData] : binding.cbuffers)
	//	{
	//		if (!binding.shader) continue;
	//		//定数バッファレイアウトを取得
	//		const ShaderReflectionData::ConstantBufferLayout* layout = binding.shader->GetConstantBufferLayout(cbName);
	//		//存在しない場合はスキップ
	//		if (!layout) continue;

	//		//対象変数を探す
	//		auto it = std::find_if(layout->variables.begin(), layout->variables.end(),
	//			[&](const ShaderReflectionData::ShaderVariable& var) {return var.name == name; });
	//		if (it == layout->variables.end()) continue;
	//		//CPU側バッファから読み込み
	//		memcpy(value, cbData.localData.data() + it->offset, size);
	//		return;
	//	}
	//}
	////見つからなかった場合の警告
	//Console::LogWarning("Warning: Material::SetFloat failed. Variable " + name + " not found.");
}

void Material::Apply(RenderContext* rtx)
{
	ID3D11DeviceContext* immediateContext = rtx->immediateContext;

	//シェーダバインドと定数バッファ更新
	for (auto& binding : m_ShaderBindings)
	{
		// シェーダが設定されていなければスキップ
		if (binding.shader)
		{
			// シェーダをバインド
			binding.shader->Bind(immediateContext);
		}
		else continue;

		// シェーダのみ適用モードの場合はここでスキップ
		if (shaderOnly)
		{
			continue;
		}

		// 定数バッファ更新
		for (auto& [cbName, cbData] : cbuffers)
		{
			// シェーダが設定されていなければスキップ
			if (!binding.shader) continue;

			// バインドしない設定がされている場合はスキップ
			if (std::find(m_CBufferNotBindNames.begin(), m_CBufferNotBindNames.end(), cbName) != m_CBufferNotBindNames.end())
			{
				continue;
			}

			//定数バッファレイアウトを取得
			const ShaderReflectionData::ConstantBufferLayout* layout = binding.shader->GetConstantBufferLayout(cbName);
			// 存在しない場合はスキップ
			if (!layout) continue;

			// 変更されていれば GPU 側に更新
			//if (cbData.dirty)
			{
				// values から値をコピーして CPU 側バッファを更新
				for (const auto& var : layout->variables)
				{
					auto it = values.find(var.name);
					if (it != values.end())
					{
						// CPU 側バッファにコピー
						memcpy(cbData.localData.data() + var.offset, it->second.data(), (std::min)(var.size, it->second.size()));
					}
				}

				// GPU 側の定数バッファに更新
				immediateContext->UpdateSubresource(cbData.buffer.Get(), 0, nullptr, cbData.localData.data(), 0, 0);
				cbData.dirty = false;
#if 0
				const ShaderReflectionData::ConstantBufferLayout* layout = binding.shader->GetConstantBufferLayout(cbName);
				if (layout)
				{
					for (const auto& var : layout->variables)
					{
						// 変更された変数をログに出力
						Console::Log("Updated CBuffer: " + cbName + ", Variable: " + var.name);
						switch (var.typeDesc.Class)
						{
						case D3D_SVC_SCALAR:
							switch (var.typeDesc.Type)
							{
							case D3D_SVT_FLOAT:
							{
								float value;
								memcpy(&value, cbData.localData.data() + var.offset, sizeof(float));
								Console::Log(var.name + ": " + std::to_string(value));
							}
							break;
							default:
								break;
							};
							break;
						case D3D_SVC_VECTOR:
							switch (var.typeDesc.Type)
							{
							case D3D_SVT_FLOAT:
							{
								float values[4] = {};
								memcpy(values, cbData.localData.data() + var.offset, sizeof(float) * var.typeDesc.Elements);
								Console::Log(var.name + ": (" + std::to_string(values[0]) + ", " + std::to_string(values[1]) + ", " + std::to_string(values[2]) + ", " + std::to_string(values[3]) + ")");
							}
							};
							break;
						default:
							break;
						};
					}

					// バインド情報もログに出力
					static std::string shaderTypeNames[] = { "Pixel", "Vertex", "Geometry", "Compute", "Domain", "Hull" };
					Console::Log("Binding CBuffer: " + cbName + " to slot " + std::to_string(layout->slot) + " in " + shaderTypeNames[static_cast<size_t>(binding.shader->GetType())]);
					Console::Log("cbData.buffer ptr: " + std::to_string((uintptr_t)cbData.buffer.Get()));
				}
#endif // 0

			}

			// バインドスロット
			UINT slot = layout->slot;

			// シェーダタイプに応じてバインド
			switch (binding.shader->GetType())
			{
			case ShaderType::Pixel:		immediateContext->PSSetConstantBuffers(slot, 1, cbData.buffer.GetAddressOf()); break;
			case ShaderType::Vertex:	immediateContext->VSSetConstantBuffers(slot, 1, cbData.buffer.GetAddressOf()); break;
			case ShaderType::Geometry:	immediateContext->GSSetConstantBuffers(slot, 1, cbData.buffer.GetAddressOf()); break;
			case ShaderType::Compute:	immediateContext->CSSetConstantBuffers(slot, 1, cbData.buffer.GetAddressOf()); break;
			case ShaderType::Domain:	immediateContext->DSSetConstantBuffers(slot, 1, cbData.buffer.GetAddressOf()); break;
			case ShaderType::Hull:		immediateContext->HSSetConstantBuffers(slot, 1, cbData.buffer.GetAddressOf()); break;
			}
		}

		// テクスチャバインド
		for (auto& info : binding.shader->GetReflectionData().textureInfos)
		{
			// テクスチャが設定されていなければスキップ
			auto it = textures.find(info.name);
			if (it == textures.end()) continue;
			if (!it->second/*.lock()*/) continue;
			// シェーダリソースビューを取得
			ID3D11ShaderResourceView* srv = it->second/*.lock()*/->GetSRV(TextureSemantic::Default);
			if (!srv) continue;
			// バインドスロット
			UINT slot = info.bindPoint;
			// バインド数
			UINT count = info.bindCount;
			// シェーダタイプに応じてバインド
			switch (binding.shader->GetType())
			{
				case ShaderType::Vertex:	immediateContext->VSSetShaderResources(slot, count, &srv); break;
				case ShaderType::Pixel:		immediateContext->PSSetShaderResources(slot, count, &srv); break;
				case ShaderType::Geometry:	immediateContext->GSSetShaderResources(slot, count, &srv); break;
				case ShaderType::Compute:	immediateContext->CSSetShaderResources(slot, count, &srv); break;
				case ShaderType::Domain:	immediateContext->DSSetShaderResources(slot, count, &srv); break;
				case ShaderType::Hull:		immediateContext->HSSetShaderResources(slot, count, &srv); break;
			}
		}

	}

	//レンダーステート設定 (初期値から変更されてたらバインド)
	{
		if (blendState != BlendState::EnumCount)
		{
			rtx->renderState->BindBlendState(rtx->immediateContext, blendState);
		}
		if (depthStencilState != DepthStencilState::EnumCount)
		{
			rtx->renderState->BindDepthStencilState(rtx->immediateContext, depthStencilState);
		}
		if (rasterizerState != RasterizerState::EnumCount)
		{
			rtx->renderState->BindRasterizerState(rtx->immediateContext, rasterizerState);
		}
	}
}


void Material::DrawProperty()
{
#ifdef USE_IMGUI
	//シェーダーが設定されていなければ何もしない
	//if (!m_Shader) return;

	ImGui::PushID(this); // Material インスタンスごとに一意の ID を確保

	//ImGuiでプロパティ編集
	if (ImGui::TreeNodeEx("Material", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf))
	{
		int id = 0;

		// シェーダーステージごとに表示
		for (size_t i = 0; i < static_cast<size_t>(ShaderType::EnumCount); ++i)
		{
			ImGui::PushID(id++); // シェーダーステージごとに一意の ID を確保

			auto& shader = m_ShaderBindings[i].shader;

			// シェーダセレクター
			DrawShaderSelector(Graphics::GetDevice(), i);

			//シェーダーが設定されていなければスキップ
			if (!shader)
			{
				ImGui::PopID();
				continue;
			}

			// シェーダのみ適用モードの場合はスキップ
			if (shaderOnly)
			{
				ImGui::PopID();
				continue;
			}

			//定数バッファごとに表示
			DrawCBufferVariables(i);

			ImGui::Separator();

			// テクスチャスロットごとに表示
			DrawTextureSlots(i);

			ImGui::Separator();

			// サンプラースロットごとに表示
			//DrawSamplerSlots(i);

			ImGui::PopID();
		}

		// レンダーステート
		ImGui::Separator();
		ImGui::Text("Render State");

		// レンダーステートの名前リスト
		const char* blendStateNames[] = { "Opaque",	"Transparency",	"Additive",	"Subtraction", "Multiply", "Skip" };
		const char* depthStencilStateNames[] = { "TestAndWrite", "WriteOnly", "TestOnly", "NoTestNoWrite", "Skip" };
		const char* rasterizerStateNames[] = { "SolidCullNone", "SolidCullFront", "SolidCullBack", "WireCullNone", "WireCullBack", "UseScissorRects", "Skip" };

		// BlendState, DepthStencilState, RasterizerState のコンボボックスを表示
		ImGui::Combo("Blend State", (int*)&blendState, blendStateNames, static_cast<int>(BlendState::EnumCount));
		ImGui::Combo("DepthStencil State", (int*)&depthStencilState, depthStencilStateNames, static_cast<int>(DepthStencilState::EnumCount));
		ImGui::Combo("Rasterizer State", (int*)&rasterizerState, rasterizerStateNames, static_cast<int>(RasterizerState::EnumCount));
		
		ImGui::TreePop();
	}

	ImGui::PopID();

#endif // USE_IMGUI
}

void Material::DrawShaderSelector(ID3D11Device* device, size_t type)
{
#ifdef USE_IMGUI

	// シェーダーステージ名を取得
	std::string typeNames[] = { "PS:", "VS:", "GS:", "HS:", "DS:", "CS:" };
	std::string currentShaderName = "None";
	
	// ファイル名からシェーダーターゲットを推定
	std::filesystem::path path;
	if (m_ShaderBindings[type].shader)
	{
		path = (m_ShaderBindings[type].shader->GetPath()); // シェーダのパス(csoファイル)
		std::string stem = path.stem().string(); // ファイル名（拡張子なし）
		currentShaderName = stem;
	}
	
	// シェーダーのリストを表示

	if (path != std::filesystem::path())
	{
		if (ImGui::Button("Edit"))
		{
			std::filesystem::path shaderPath = "./Shader" / (path.filename().replace_extension(".hlsl")); // 元のシェーダーファイルのパスを取得

			// シェーダーファイルを既定のアプリケーションで開く
			OpenFileWithDefaultApplication(shaderPath.wstring());
		}

		ImGui::SameLine();
	}

	int id = 0;
	if (ImGui::BeginCombo((typeNames[static_cast<size_t>(type)] + currentShaderName).c_str(), currentShaderName.c_str()))
	{
		for (const std::filesystem::path& path : ResourceManager::GetShaderPaths())
		{
			std::string stem = path.stem().string(); // ファイル名（拡張子なし）
			std::string suffix = stem.substr(stem.size() - 2, 2); // ファイル名の末尾2文字を取得
			std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower); // 小文字に変換
			// シェーダータイプに合わないものはスキップ
			if ((type == static_cast<size_t>(ShaderType::Pixel) && suffix != "ps") ||
				(type == static_cast<size_t>(ShaderType::Vertex) && suffix != "vs") ||
				(type == static_cast<size_t>(ShaderType::Geometry) && suffix != "gs") ||
				(type == static_cast<size_t>(ShaderType::Hull) && suffix != "hs") ||
				(type == static_cast<size_t>(ShaderType::Domain) && suffix != "ds") ||
				(type == static_cast<size_t>(ShaderType::Compute) && suffix != "cs"))
			{
				continue;
			}

			// 一意の ID を確保
			ImGui::PushID(id++);
			std::string name = stem; // 拡張子を除いたファイル名
			// 選択可能なアイテムとして表示
			if (ImGui::Selectable(name.c_str(), false))
			{
				// 選択されたらシェーダをロードして設定
				std::shared_ptr<Shader> shader;
				switch (type)
				{
					case static_cast<size_t>(ShaderType::Pixel): shader = ResourceManager::Load<PixelShader>(path.string()); break;
					case static_cast<size_t>(ShaderType::Vertex): shader = ResourceManager::Load<VertexShader>(path.string()); break;
					case static_cast<size_t>(ShaderType::Geometry): shader = ResourceManager::Load<GeometryShader>(path.string()); break;
					case static_cast<size_t>(ShaderType::Hull): shader = ResourceManager::Load<HullShader>(path.string()); break;
					case static_cast<size_t>(ShaderType::Domain): shader = ResourceManager::Load<DomainShader>(path.string()); break;
					case static_cast<size_t>(ShaderType::Compute): shader = ResourceManager::Load<ComputeShader>(path.string()); break;
				default:
					break;
				}
				if (shader)
				{
					SetShader(device, shader);
				}
			}
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	
#endif // USE_IMGUI
}

void Material::DrawCBufferVariables(size_t shaderType)
{
#ifdef USE_IMGUI
	int id = 0;
	auto& shader = m_ShaderBindings[shaderType].shader;

	for (const ShaderReflectionData::ConstantBufferLayout& cbLayout : shader->GetAllConstantBufferLayouts())
	{
		bool isNotBind = std::find(m_CBufferNotBindNames.begin(), m_CBufferNotBindNames.end(), cbLayout.name) != m_CBufferNotBindNames.end();
		if (isNotBind)
		{
			//バインドしない設定がされている場合はスキップ
			//continue;
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f); // 全体の透明度を下げる
		}
		ImGui::PushID(id++);

		//定数バッファ名を表示
		//ImGui::SeparatorText(cbLayout.name.c_str());
		if (ImGui::TreeNodeEx("##CBuffer", isNotBind ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_DefaultOpen, cbLayout.name.c_str()))
		{
			for (const ShaderReflectionData::ShaderVariable& var : cbLayout.variables)
			{
				ImGui::PushID(id++);

				std::string label = var.name;

				//バインドしない設定がされている場合は、UIにその旨を表示して編集不可にする
				if (isNotBind)
				{
					label += " (Not Bound)";
					ImGui::Text(label.c_str());
					ImGui::PopID();
					continue;
				}
				bool edited = false;
				std::vector<uint8_t> buffer(var.size);
				GetValue(var.name, buffer.data(), var.size);

				const char* floatFormat = "%.6f";
				//型情報に基づいて適切なUIを表示
				switch (var.typeDesc.Class)
				{
				case D3D_SVC_SCALAR:
				{
					switch (var.typeDesc.Type)
					{
					case D3D_SVT_FLOAT: edited = ImGui::DragFloat(label.c_str(), (float*)buffer.data(), 0.00001f, 0, 0, floatFormat); break;
					case D3D_SVT_INT: edited = ImGui::DragInt(label.c_str(), (int*)buffer.data(), 1, INT_MIN, INT_MAX); break;
					case D3D_SVT_BOOL: edited = ImGui::Checkbox(label.c_str(), (bool*)buffer.data()); break;
					}
					break;
				}
				case D3D_SVC_VECTOR:
				{
					switch (var.typeDesc.Type)
					{
					case D3D_SVT_FLOAT:
					{
						switch (var.typeDesc.Columns)
						{
						case 2: edited = ImGui::DragFloat2(label.c_str(), (float*)buffer.data(), 0.01f, 0, 0, floatFormat); break;
						case 3: edited = ImGui::DragFloat3(label.c_str(), (float*)buffer.data(), 0.01f, 0, 0, floatFormat); break;
						case 4: edited = ImGui::DragFloat4(label.c_str(), (float*)buffer.data(), 0.01f, 0, 0, floatFormat); break;
						}
						break;
					}
					case D3D_SVT_INT:
					{
						switch (var.typeDesc.Columns)
						{
						case 2: edited = ImGui::DragInt2(label.c_str(), (int*)buffer.data(), 1, INT_MIN, INT_MAX); break;
						case 3: edited = ImGui::DragInt3(label.c_str(), (int*)buffer.data(), 1, INT_MIN, INT_MAX); break;
						case 4: edited = ImGui::DragInt4(label.c_str(), (int*)buffer.data(), 1, INT_MIN, INT_MAX); break;
						}
						break;
					}
					}
					break;
				}
				case D3D_SVC_MATRIX_ROWS:
				{
					if (var.typeDesc.Type == D3D_SVT_FLOAT && var.typeDesc.Rows == 4 && var.typeDesc.Columns == 4)
					{
						// 4x4行列専用のUI
						if (ImGui::TreeNodeEx("##Matrix", ImGuiTreeNodeFlags_DefaultOpen, "%s (Matrix)", var.name.c_str()))
						{
							// 行列は行ごとに4つ表示すると見やすい
							float* mat = (float*)buffer.data();
							for (UINT r = 0; r < var.typeDesc.Rows; r++)
							{
								std::string rowLabel = label + "[" + std::to_string(r) + "]";
								ImGui::DragFloat4(rowLabel.c_str(), mat + r * var.typeDesc.Columns, 0.01f, 0, 0, "%.3f", ImGuiSliderFlags_NoInput);
							}
							ImGui::TreePop();
						}
					}
					break;
				}
				case D3D_SVC_STRUCT:
				{
					ImGui::Text("%s (Struct)", var.name.c_str());
					//構造体の中身を再帰的に表示するには、ShaderVariableに子変数の情報を持たせる必要がある
					break;
				}
				default:
					break;
				}

				if (edited)
				{
					SetValue(var.name, buffer.data(), var.size);
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}

		if (isNotBind)
		{
			ImGui::PopStyleVar(); // 透明度を元に戻す
		}

		ImGui::PopID();
	}
#endif // USE_IMGUI
}

void Material::DrawTextureSlots(size_t shaderType)
{
#ifdef USE_IMGUI
	//auto& shader = m_ShaderBindings[shaderType].shader;

	for (auto& [texName, textureShared] : textures)
	{
		ImGui::PushID(texName.c_str());
		//テクスチャ名を表示
		if (ImGui::TreeNodeEx("##Texture", ImGuiTreeNodeFlags_DefaultOpen, texName.c_str()))
		{
			auto texture = std::dynamic_pointer_cast<Texture2D>(textureShared);

			//テクスチャのプレビュー表示
			if (texture)
			{
				if (texture->GetSRV(TextureSemantic::Default))
				{
					// テクスチャの配列サイズが1なら通常のプレビュー、複数なら配列テクスチャとして表示
					if (texture->GetDesc().ArraySize == 1)
					{
						ImGui::Image(texture->GetSRV(TextureSemantic::Default), ImVec2(64, 64));
					}
					else
					{
						// 配列テクスチャのプレビュー表示のためだけにSRVを作成するのはコストが高いので、ここでは単純に「Array Texture」と表示するだけにする
						ImGui::Text("Array Texture (ArraySize: %d)", texture->GetDesc().ArraySize);
						//ImGui::Image(texture->GetSRV(TextureSemantic::Default), ImVec2(64, 64)); // 配列テクスチャはそのまま表示すると出力ログにエラーが出るので一旦コメントアウト

#if 0
						// 配列テクスチャは1枚目をプレビュー表示
								// Texture2DArray の 0 番目の要素だけを指す SRV を作成
						D3D11_TEXTURE2D_DESC textureDesc = texture->GetDesc();
						ID3D11ShaderResourceView* srv = texture->GetSRV(TextureSemantic::Default);
						D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
						srv->GetDesc(&srvDesc);
						srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D; // Array ではなく 2D として定義
						srvDesc.Texture2D.MipLevels = 1;
						srvDesc.Texture2D.MostDetailedMip = 0;

						Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2d;
						Graphics::GetDevice()->CreateTexture2D(&textureDesc, nullptr, tex2d.GetAddressOf());

						Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> singleSrv;
						HRESULT hr = Graphics::GetDevice()->CreateShaderResourceView(tex2d.Get(), &srvDesc, singleSrv.GetAddressOf());
						if (SUCCEEDED(hr))
						{
							ImGui::Image(singleSrv.Get(), ImVec2(64, 64));
						}
						else
						{
							Console::LogWarning("Failed to create single-slice SRV for texture preview: " + std::to_string(hr));
						}
#endif // 0

					}
				}
				else // テクスチャが設定されていない場合はダミーを表示
				{
					std::shared_ptr<AssetTexture> dummyTex = std::make_shared<AssetTexture>();
					dummyTex->MakeDummy(Graphics::GetDevice(), 0xFFFF00FF, 16); // マゼンタのダミーテクスチャ
					SetTexture(texName, dummyTex);
				}
			}
			//テクスチャのパス表示
			if (auto assetTexture = std::dynamic_pointer_cast<AssetTexture>(texture))
			{
				std::string path = assetTexture->GetPath();
				if (path.empty()) path = "None";
				ImGui::TextWrapped("Path: %s", path.c_str());
			}
			//テクスチャの変更ボタン
			if (ImGui::Button("Change Texture"))
			{
				//ファイルダイアログを開いてテクスチャを選択
				const char* filter = "*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.dds";
				
				char filepath[260] = "";
				if (Dialog::OpenFileName(filepath, sizeof(filepath), filter, "Select Texture") == DialogResult::OK)
				{
					//選択されたテクスチャをロードして設定
					std::shared_ptr<AssetTexture> newTexture = ResourceManager::Load<AssetTexture>(filepath);
					if (newTexture)
					{
						SetTexture(texName, newTexture);
					}
				}
			}
			//テクスチャの削除ボタン
			if (texture)
			{
				ImGui::SameLine();
				if (ImGui::Button("Clear Texture"))
				{
					//テクスチャをクリアしてダミーに戻す
					std::shared_ptr<AssetTexture> dummyTex = std::make_shared<AssetTexture>();
					dummyTex->MakeDummy(Graphics::GetDevice()); // ダミーテクスチャ
					SetTexture(texName, dummyTex);
				}
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

#endif // USE_IMGUI
}

void Material::DrawSamplerSlots(size_t shaderType)
{
#ifdef USE_IMGUI
	auto& shader = m_ShaderBindings[shaderType].shader;
	for (const ShaderReflectionData::SamplerInfo& samplerInfo : shader->GetReflectionData().samplerInfos)
	{
		ImGui::PushID(samplerInfo.name.c_str());

		// サンプラー名を表示
		ImGui::Text("%s (Sampler)", samplerInfo.name.c_str());

		ImGui::PopID();
	}

#endif // USE_IMGUI
}

void Material::UpdateCBufferBindings(ID3D11Device* device, ShaderBinding& binding, const ShaderReflectionData& reflection)
{
	// 既存の定数バッファを破棄
	cbuffers.clear();

	// 必要な定数バッファの数だけ確保
	cbuffers.reserve(reflection.constantBufferLayouts.size());

	// Shaderの定数バッファレイアウトを取得し、バッファを作成
	for (const ShaderReflectionData::ConstantBufferLayout& layout : reflection.constantBufferLayouts)
	{
		// 16バイトアラインメントにサイズを調整
		size_t alignedSize = Align16(layout.size);
		// サイズがアラインメントされていない場合は警告
		if (alignedSize != layout.size)
		{
			Console::LogWarning("Warning: CBuffer " + layout.name + " size " + std::to_string(layout.size) +
				" is not 16-byte aligned. Aligned size: " + std::to_string(alignedSize));
		}

		// 定数バッファを作成
		auto& cbData = cbuffers[layout.name];
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.ByteWidth = static_cast<UINT>(alignedSize);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, cbData.buffer.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

		// CPU 側のローカルコピー用バッファを確保
		cbData.localData.resize(alignedSize);

		// ZeroMemory で初期化
		ZeroMemory(cbData.localData.data(), alignedSize);

		// 初期値がシェーダに定義されている場合はローカルバッファにコピーしておく
		for (const ShaderReflectionData::ShaderVariable& var : layout.variables)
		{
			if (var.defaultValue == nullptr) continue; // 初期値が定義されていない場合はスキップ
			if (var.offset + var.size > layout.size) // 初期値がバッファサイズを超えている場合は警告を出してスキップ
			{
				Console::LogWarning("Warning: Default value of variable " + var.name + " in CBuffer " + layout.name +
					" exceeds buffer size. Variable offset: " + std::to_string(var.offset) +
					", Variable size: " + std::to_string(var.size) +
					", Buffer size: " + std::to_string(layout.size));
				continue;
			}
			//memcpy(cbData.localData.data() + var.offset, var.defaultValue, var.size);
			//m_isValuesDirty = true; // 初期値をコピーしたので GPU 側に更新が必要
			SetValue(var.name, var.defaultValue, var.size); // SetValue を使って初期値を設定
		}

	}
}

void Material::UpdateTextureAndSamplerBindings(ID3D11Device* device, ShaderBinding& binding, const ShaderReflectionData& reflection)
{
	//// 既存のテクスチャ設定をクリア
	//textures.clear();
	//// 必要なテクスチャ数だけ確保
	//textures.reserve(reflection.textureInfos.size());
	// Shaderのテクスチャバインド情報を取得し、テクスチャスロットを確保
	for (const ShaderReflectionData::TextureInfo& texInfo : reflection.textureInfos)
	{
		// テクスチャ変数名をキーとして登録
		if (textures.find(texInfo.name) == textures.end())
		{
			std::shared_ptr<AssetTexture> defaultTex = std::make_shared<AssetTexture>();
			defaultTex->MakeDummy(device); // ダミーテクスチャを生成
			textures[texInfo.name] = defaultTex;
		}
	}

#if 0
	// 既存のサンプラー設定をクリア
	binding.samplers.clear();
	// 必要なサンプラー数だけ確保
	binding.samplers.reserve(reflection.samplerInfos.size());

	// サンプラーは現状UIでの編集のみで、Material側での保持はしない
	for (const ShaderReflectionData::SamplerInfo& samplerInfo : reflection.samplerInfos)
	{
		// サンプラー変数名をキーとして登録
		if (binding.samplers.find(samplerInfo.name) == binding.samplers.end())
		{
			// デフォルトはWrap, MinMagMipLinear
			binding.samplers[samplerInfo.name] = SamplerState::LinearWrap;
		}
	}
#endif // 0
}

json Material::Serialize() const
{
	json j;
	// シェーダーパスの保存
	for (size_t i = 0; i < static_cast<size_t>(ShaderType::EnumCount); ++i)
	{
		auto& shader = m_ShaderBindings[i].shader;
		if (shader)
		{
			j["shaders"][std::to_string(i)] = shader->GetPath();
		}
	}
	// テクスチャの保存
	//for (size_t i = 0; i < static_cast<size_t>(ShaderType::EnumCount); ++i)
	{
		//auto& textures = m_ShaderBindings[i].textures;
		int texIndex = 0;
		for (const auto& [texName, texture] : textures)
		{
			if (auto assetTexture = std::dynamic_pointer_cast<AssetTexture>(texture))
			{
				j["textures"][std::to_string(texIndex)][texName] = assetTexture->GetPath();
			}
			texIndex++;
		}
	}
	// 定数バッファの値の保存
	for (size_t i = 0; i < static_cast<size_t>(ShaderType::EnumCount); ++i)
	{
		//auto& cbuffers = m_ShaderBindings[i].cbuffers;
		
		for (const auto& [cbName, cbData] : cbuffers)
		{
			if (std::find(m_CBufferNotBindNames.begin(), m_CBufferNotBindNames.end(), cbName) != m_CBufferNotBindNames.end())
			{
				// バインドしない設定がされている場合は保存しない
				continue;
			}
			const ShaderReflectionData::ConstantBufferLayout* layout = m_ShaderBindings[i].shader->GetConstantBufferLayout(cbName);
			if (!layout)
			{
				Console::LogWarning("Warning: Material::Serialize skipped CBuffer " + cbName + " because its layout was not found in the shader.");
				continue;
			}
			
			// 定数バッファのローカルデータを保存してしまうとサイズが大きくなりすぎてしまうので
			//　ShaderVariableの型情報をもとに、保存する値を必要最低限に絞る
			for (const ShaderReflectionData::ShaderVariable& var : layout->variables)
			{
				switch (var.typeDesc.Class)
				{
				case D3D_SVC_SCALAR:
				{
					switch (var.typeDesc.Type)
					{
					case D3D_SVT_FLOAT:
					{
						float value;
						GetValue(var.name, &value, sizeof(float));
						j["cbuffers"][std::to_string(i)][var.name] = value;
						break;
					}
					case D3D_SVT_INT:
					{
						int value;
						GetValue(var.name, &value, sizeof(int));
						j["cbuffers"][std::to_string(i)][var.name] = value;
						break;
					}
					case D3D_SVT_BOOL:
					{
						bool value;
						GetValue(var.name, &value, sizeof(bool));
						j["cbuffers"][std::to_string(i)][var.name] = value;
						break;
					}
					default:
						Console::LogWarning("Warning: Material::Serialize skipped CBuffer variable " + var.name + " because its scalar type is not supported for serialization.");
						break;
					};
					break;
				}
				case D3D_SVC_VECTOR:
				{
					switch (var.typeDesc.Type)
					{
					case D3D_SVT_FLOAT:
					{
						std::vector<float> vecData(var.size / sizeof(float));
						GetValue(var.name, vecData.data(), var.size);
						j["cbuffers"][std::to_string(i)][var.name] = vecData;
						break;
					}
					case D3D_SVT_INT:
					{
						std::vector<int> vecData(var.size / sizeof(int));
						GetValue(var.name, vecData.data(), var.size);
						j["cbuffers"][std::to_string(i)][var.name] = vecData;
						break;
					}
					default:
						Console::LogWarning("Warning: Material::Serialize skipped CBuffer variable " + var.name + " because its vector type is not supported for serialization.");
						break;
					};
					break;
				}
				case D3D_SVC_STRUCT:
				{
					//構造体の中身を保存するには、ShaderVariableに子変数の情報を持たせる必要がある
					Console::LogWarning("Warning: Material::Serialize skipped CBuffer variable " + var.name + " because struct type is not supported for serialization.");
					break;
				}
				default:
					Console::LogWarning("Warning: Material::Serialize skipped CBuffer variable " + var.name + " because its type is not supported for serialization.");
					break;
				};
			}
		}
	}


	// レンダーステートの保存
	j["blendState"] = static_cast<int>(blendState);
	j["depthStencilState"] = static_cast<int>(depthStencilState);
	j["rasterizerState"] = static_cast<int>(rasterizerState);
	return j;
}

bool Material::Deserialize(const json& j)
{
	// シェーダーパスの読み込み
	for (size_t i = 0; i < static_cast<size_t>(ShaderType::EnumCount); ++i)
	{
		std::string key = std::to_string(i);
		if (j.contains("shaders") && j["shaders"].contains(key))
		{
			std::string shaderPath = j["shaders"][key];
			std::filesystem::path path = shaderPath;
			std::string stem = path.stem().string(); // 拡張子を除いたファイル名
			std::shared_ptr<Shader> shader;
			switch (i)
			{
			case static_cast<size_t>(ShaderType::Pixel): shader = ResourceManager::GetOrLoadShader<PixelShader>(stem); break;
			case static_cast<size_t>(ShaderType::Vertex): shader = ResourceManager::GetOrLoadShader<VertexShader>(stem); break;
			case static_cast<size_t>(ShaderType::Geometry): shader = ResourceManager::GetOrLoadShader<GeometryShader>(stem); break;
			case static_cast<size_t>(ShaderType::Hull): shader = ResourceManager::GetOrLoadShader<HullShader>(stem); break;
			case static_cast<size_t>(ShaderType::Domain): shader = ResourceManager::GetOrLoadShader<DomainShader>(stem); break;
			case static_cast<size_t>(ShaderType::Compute): shader = ResourceManager::GetOrLoadShader<ComputeShader>(stem); break;
			default:
				break;
			}
			if (shader)
			{
				SetShader(Graphics::GetDevice(), shader);
			}
		}
	}
	// テクスチャの読み込み
	for (size_t i = 0; i < static_cast<size_t>(ShaderType::EnumCount); ++i)
	{
		std::string key = std::to_string(i);
		if (j.contains("textures") && j["textures"].contains(key))
		{
			for (auto& [texName, texPath] : j["textures"][key].items())
			{
				std::shared_ptr<AssetTexture> texture;
				std::string texPathStr = texPath;
				if (!texPathStr.empty())
				{
					texture = ResourceManager::GetOrLoad<AssetTexture>(texPath);
				}
				else
				{
					texture = std::make_shared<AssetTexture>();
					texture->MakeDummy(Graphics::GetDevice()); // ダミーテクスチャ
				}
				if (texture)
				{
					SetTexture(texName, texture);
				}
			}
		}
	}
	// 定数バッファの値の読み込み
	for (size_t i = 0; i < static_cast<size_t>(ShaderType::EnumCount); ++i)
	{
		std::string key = std::to_string(i);
		if (j.contains("cbuffers") && j["cbuffers"].contains(key))
		{
			for (auto& [varName, varValue] : j["cbuffers"][key].items())
			{
				if (varValue.is_array())
				{
					std::vector<float> vecData = varValue.get<std::vector<float>>();
					SetValue(varName, vecData.data(), vecData.size() * sizeof(float));
				}
				else if (varValue.is_number_float())
				{
					float floatData = varValue.get<float>();
					SetValue(varName, &floatData, sizeof(float));
				}
				else if (varValue.is_number_integer())
				{
					int intData = varValue.get<int>();
					SetValue(varName, &intData, sizeof(int));
				}
				else if (varValue.is_boolean())
				{
					bool boolData = varValue.get<bool>();
					SetValue(varName, &boolData, sizeof(bool));
				}
			}
		}
	}
	
	
	// レンダーステートの読み込み
	if (j.contains("blendState"))
	{
		blendState = static_cast<BlendState>(j["blendState"].get<int>());
	}
	if (j.contains("depthStencilState"))
	{
		depthStencilState = static_cast<DepthStencilState>(j["depthStencilState"].get<int>());
	}
	if (j.contains("rasterizerState"))
	{
		rasterizerState = static_cast<RasterizerState>(j["rasterizerState"].get<int>());
	}
	return true;
}