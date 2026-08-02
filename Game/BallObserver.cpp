#include "pch.h"
#include "BallObserver.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Physics/Collider.h"
#include "Ball.h"
#include "Gate.h"
#include "Engine/Scenes/Scene.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(BallObserver, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(BallObserver, "UserScripts", ComponentAttributes::None, {})


void BallObserver::Start()
{
	ballCount = 0; // 衝突回数を初期化
	// コンポーネントが開始されたときの処理をここに実装します。
	Collider* collider = GetOwner()->GetComponent<Collider>();
	if (collider)
	{
		collider->AddOnTriggerEnterEvent([this](const TriggerInfo& info) { 
			// 衝突開始イベントの処理をここに実装します。
			if (info.other && info.other->GetComponent<Ball>()) // 衝突相手が Ball コンポーネントを持っているか確認
			{
				ballCount++; // 衝突回数を増加
			}
			Console::Log("BallObserver: Trigger Entered. Current Ball Count: " + std::to_string(ballCount));
		});
		//collider->AddOnTriggerStayEvent([this](const TriggerInfo& info) { 
		//	// 衝突継続イベントの処理をここに実装します。
		//	if (!info.other) // 衝突相手が存在するか確認
		//	{
		//		return; // 存在しない場合は処理を中断
		//	}
		//	if (info.other->GetComponent<Ball>()) // 衝突相手が Ball コンポーネントを持っているか確認
		//	{
		//		ballCount++; // 衝突回数を増加
		//		//Console::Log("BallObserver: Trigger Staying. Current Ball Count: " + std::to_string(ballCount));
		//	}
		//	});
		collider->AddOnTriggerExitEvent([this](const TriggerInfo& info) { 
			// 衝突終了イベントの処理をここに実装します。
			if (info.other && info.other->GetComponent<Ball>()) // 衝突相手が Ball コンポーネントを持っているか確認
			{
				ballCount--; // 衝突回数を減少
				if (ballCount <= 0) // カウンタが負の値にならないようにする
				{
					ballCount = 0; // カウンタをリセット
					Gate* gate = GetScene()->FindComponentById<Gate>(gateReference);
					if (gate)
					{
						gate->CloseGate(); // ゲートを閉じる処理を呼び出す
					}
				}
			}
			Console::Log("BallObserver: Trigger Exited. Current Ball Count: " + std::to_string(ballCount));
		});
	}
}

void BallObserver::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
	//ballCount = 0; // 毎フレーム、衝突回数をリセットしてから再計算
}

//void BallObserver::DrawProperty()
//{
//#ifdef USE_IMGUI
//	Component::DrawProperty(); // 基底クラスのプロパティ描画を呼び出す
//	// エディタでプロパティを描画するための処理をここに実装します。
//	ImGui::Text("Current Ball Count: %d", ballCount);
//#endif // 
//
//}
