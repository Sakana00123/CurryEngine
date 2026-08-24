#include "pch.h"
#include "AnimationImporter.h"
#include <Engine\Animation\AnimatorController.h>

namespace CurryEngine
{
	namespace Resources
	{
		std::shared_ptr<Resource> AnimationImporter::Import(const AssetMeta& meta)
		{
			auto animationClip = std::make_shared<AnimationClip>();
			if (!animationClip->LoadFromFile(meta.path.string()))
			{
				LOG_ERROR(u8"[AnimationImporter] アニメーションのインポートに失敗しました: " + meta.path.u8string());
				return nullptr;
			}
			return animationClip;
		}

		std::vector<std::string> AnimationImporter::GetSupportedExtensions() const
		{
			return { ".anim" };
		}
	}
}
