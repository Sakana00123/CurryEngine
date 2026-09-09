#include "pch.h"
#include "Engine/Core/Layer.h"

ENGINE_API const char* LayerManager_GetLayerName(Layer layer)
{
	if (layer < 0 || layer >= MAX_LAYERS)
	{
		return nullptr; // 無効なレイヤーIDの場合はnullptrを返す
	}
	return LayerManager::Get().GetLayerNames()[layer].c_str();
}

ENGINE_API int LayerManager_GetLayerMaskByName(const char* name)
{
	int layer = LayerManager::Get().GetLayerByName(name);
	return layer >= 0 ? ToMask(layer) : 0;
}
