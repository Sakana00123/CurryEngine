#include "pch.h"
#include "OutArea.h"
#include "Engine/Physics/Collider.h"
#include "Gate.h"
#include "Engine/Scenes/Scene.h"
#include "Ball.h"
#include "AchievementManager.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(OutArea, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(OutArea, "UserScripts", ComponentAttributes::None, {})


void OutArea::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	if (Collider* collider = GetOwner()->GetComponent<Collider>())
	{
		collider->AddOnTriggerEnterEvent([this](const TriggerInfo& info) { OnTriggerEnter(info); });
	}
}

void OutArea::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void OutArea::OnTriggerEnter(const TriggerInfo& info)
{
	// トリガーに入ったときの処理をここに実装します。
	if (info.otherCollider == nullptr) return;
	Ball* ball = info.other->GetComponent<Ball>();
	if (!ball) return; // トリガーに入ったオブジェクトが Ball コンポーネントを持っていない場合は何もしない
	
	if (Gate* gate = GetScene()->FindComponentById<Gate>(gateReference))
	{
		if (gate->IsClosed())
		{
			// ゲートが閉じているときに、トリガーに入ったオブジェクトをシーンから削除する。

			// デバッガーアチーブメント
			AchievementManager::AddProgressToManager(GetScene(), "DEBUGGER", 1);

			ball->GetOwner()->Destroy();
		}
		
	}
}