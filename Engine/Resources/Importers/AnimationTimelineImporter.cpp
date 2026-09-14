#include "pch.h"
#include "AnimationTimelineImporter.h"
#include <Engine\Resources\AnimationEvent.h>
#include <Engine\Resources\ResourceManager.h>

namespace CurryEngine
{
	namespace Resources
	{
		std::shared_ptr<Resource> AnimationTimelineImporter::Import(const AssetMeta& meta)
		{
			auto timeline = ResourceManager::GetOrLoad<AnimationTimeline>(meta.path.string());
			if (!timeline)
			{
				LOG_ERROR(u8"[AnimationTimelineImporter] AnimationTimelineのインポートに失敗しました: " + meta.path.u8string());
				return nullptr;
			}
			return timeline;
		}
		std::vector<std::string> AnimationTimelineImporter::GetSupportedExtensions() const
		{
			return { ".animtimeline" };
		}
	}
}
