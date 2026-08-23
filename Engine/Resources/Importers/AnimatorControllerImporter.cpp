#include "pch.h"
#include "AnimatorControllerImporter.h"
#include <Engine\Animation\AnimatorController.h>

namespace CurryEngine
{
	namespace Resources
	{
		std::shared_ptr<Resource> AnimatorControllerImporter::Import(const AssetMeta& meta)
		{
			auto animatorController = std::make_shared<AnimatorController>();
			if (!animatorController->LoadFromFile(meta.path.string()))
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
