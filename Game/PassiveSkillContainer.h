#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "PassiveSkillData.h"
#include "PassiveSkillView.h"

class PassiveSkillContainer : public Component
{
	C_REFLECT(PassiveSkillContainer)
public:
	PassiveSkillContainer() = default;
	~PassiveSkillContainer() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;
	//void DrawProperty() override; // エディタでプロパティを描画するためのオーバーライド関数

	// プレイヤーが新しいパッシブスキルを獲得する関数
	void AddSkill(const PassiveSkillData& skillData);

	// プレイヤーが特定のスキルを持っているかを確認する関数
	bool HasSkill(const std::string& skillName) const;

	// プレイヤーが特定のスキルを獲得できるかを確認する関数
	bool CanAcquireSkill(const PassiveSkillData& skillData) const;

	// ショップ内のアイテムのプレハブパスを指定して、そのスキルを獲得できるかを確認する関数
	int GetCurrentStack(const std::string& shopPrefabPath) const;

	// ショップ内のアイテムのプレハブパスを指定して、そのスキルの最大スタック数を取得する関数
	int GetMaxStack(const std::string& shopPrefabPath) const;

	// 指定したプロパティに対する全てのスキルの効果値を合計して返す関数
	float GetModifier(const std::string& propertyName) const;


private:
	struct StackableSkillInfo
	{
		float totalModifier = 0.0f; // スキルの効果値の合計
		int currentStack = 0; // 現在のスタック数
		std::weak_ptr<PassiveSkillView> skillView; // スキルのUI表示を管理するコンポーネントへのポインタ
		PassiveSkillData skillData; // スキルの基本データ
	};

	std::unordered_map<std::string, StackableSkillInfo> acquiredSkills; // プレイヤーが獲得したパッシブスキルのリスト


	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId containerReference; // パッシブスキルのアイコンを表示するコンテナへの参照


};
