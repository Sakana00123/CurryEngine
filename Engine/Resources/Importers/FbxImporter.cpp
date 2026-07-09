#include "pch.h"
#include "FbxImporter.h"
#include "Engine/Resources/SkinnedMesh.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Resources/ImportSettings/FbxImportSettings.h"

namespace CurryEngine::Resources
{
	std::shared_ptr<Resource> FbxImporter::Import(const AssetMeta& meta)
	{
		auto skinnedMesh = std::make_shared<SkinnedMesh>();
		FbxImportSettings settings = meta.GetImportSettings<FbxImportSettings>();
		if (!skinnedMesh->LoadFromFBX(Graphics::GetDevice(), meta.path.string().c_str(), meta.id.id.c_str(), &settings))
		{
			return nullptr;
		}
		return skinnedMesh;
	}
	std::vector<std::string> FbxImporter::GetSupportedExtensions() const
	{
		return { ".fbx" };
	}
}
