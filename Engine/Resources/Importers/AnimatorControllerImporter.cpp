#include "pch.h"
#include "AnimatorControllerImporter.h"
#include <Engine\Animation\AnimatorController.h>
#include <Engine\Resources\ResourceManager.h>

namespace CurryEngine
{
	namespace Resources
	{
		std::shared_ptr<Resource> AnimatorControllerImporter::Import(const AssetMeta& meta)
		{
			auto animatorController = ResourceManager::GetOrLoad<AnimatorController>(meta.path.string());
			if (!animatorController)
			{
				LOG_ERROR(u8"[AnimatorControllerImporter] AnimatorControllerのインポートに失敗しました: " + meta.path.u8string());
				return nullptr;
			}
			return animatorController;
		}
		std::vector<std::string> AnimatorControllerImporter::GetSupportedExtensions() const
		{
			return { ".controller" };
		}
	}
}
