#include "pch.h"
#include "CoinDestroyArea.h"
#include "Engine/Physics/Collider.h"
#include "Coin.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(CoinDestroyArea, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(CoinDestroyArea, "UserScripts", ComponentAttributes::None, {})


void CoinDestroyArea::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	// トリガーイベントの登録
	if (Collider* collider = GetOwner()->GetComponent<Collider>())
	{
		collider->AddOnTriggerEnterEvent([this](const TriggerInfo& info) { OnTriggerEnter(info); });
	}
}

void CoinDestroyArea::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}

void CoinDestroyArea::OnTriggerEnter(const TriggerInfo& info)
{
	if (info.other == nullptr)return;
	// 衝突相手がCoinコンポーネントを持っているか確認
	Coin* coin = info.other->GetComponent<Coin>();
	if (coin != nullptr)
	{
		// Coinコンポーネントが存在する場合、そのGameObjectを破壊する
		coin->GetOwner()->Destroy();
	}
}