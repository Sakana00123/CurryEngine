#include "pch.h"
#include "Resource.h"
#include "AssetDatabase.h"

bool Resource::Load(const CurryEngine::Resources::AssetId& assetId)
{
	if (!assetId.IsValid())
	{
		LOG_ERROR(u8"[Resource] アセットの読み込みに失敗しました。AssetId が無効です。");
		return false;
	}
	// アセットIDからアセットメタデータを取得
	auto meta = CurryEngine::Resources::AssetDatabase::Find(assetId);
	if (!meta)
	{
		LOG_ERROR(u8"[Resource] アセットの読み込みに失敗しました。AssetId が見つかりません。: " + std::u8string(assetId.ToString().begin(), assetId.ToString().end()));
		return false;
	}

	// アセットのパスを取得
	_path = meta->path.string();

	// ファイルからロード
	return LoadFromFile(_path);
}
