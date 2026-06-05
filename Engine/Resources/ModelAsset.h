#pragma once
#include "Resource.h"
#include "Mesh.h"
#include "Skeleton.h"
#include "AnimationClip.h"
#include "Engine/Rendering/Material.h"


struct NodeDesc
{
	std::string name; // ノードの名前
	int parentIndex = -1; // 親ノードのインデックス（-1はルートノード）
	DirectX::XMFLOAT4X4 localTransform; // ローカルトランスフォーム行列
};

struct ModelAsset
{
	std::vector<std::shared_ptr<Mesh>> meshes; // モデルが持つメッシュのリスト
	std::vector<std::shared_ptr<Material>> materials; // モデルが持つマテリアルのリスト
	std::shared_ptr<Skeleton> skeleton; // モデルのスケルトン（アニメーション用）
	std::vector<std::shared_ptr<AnimationClip>> animations; // モデルのアニメーションクリップのリスト
	// ノード階層の情報
	std::vector<NodeDesc> nodes; // ノードのリスト
};