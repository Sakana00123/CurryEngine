#pragma once
#include <string>
#include <DirectXMath.h>
#include <vector>
#include <unordered_map>

struct Bone
{
	std::string name; // ボーンの名前
	int parentIndex = -1; // 親ボーンのインデックス（-1はルートボーン）
	DirectX::XMFLOAT4X4 offsetMatrix{}; // オフセット行列（ボーンの初期姿勢を表す行列）
};

class Skeleton
{
	public:
	std::vector<Bone> bones; // ボーンのリスト
	std::unordered_map<std::string, int> boneNameToIndex; // ボーン名からインデックスへのマッピング
	// ボーンを追加する関数
	void AddBone(const std::string& name, int parentIndex, const DirectX::XMFLOAT4X4& offsetMatrix)
	{
		Bone bone;
		bone.name = name;
		bone.parentIndex = parentIndex;
		bone.offsetMatrix = offsetMatrix;
		bones.push_back(bone);
		boneNameToIndex[name] = static_cast<int>(bones.size() - 1);
	}
	// ボーンのインデックスを取得する関数
	int GetBoneIndex(const std::string& name) const
	{
		auto it = boneNameToIndex.find(name);
		if (it != boneNameToIndex.end())
		{
			return it->second;
		}
		return -1; // 見つからない場合は-1を返す
	}
};