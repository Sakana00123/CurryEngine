#include "pch.h"
#include "ShakeButton.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Core/GameObject.h"
#include "Engine/UI/Button.h"
#include "Shaker.h"
#include "Ball.h"
#include <Engine\Physics\Rigidbody.h>

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(ShakeButton, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(ShakeButton, "UserScripts", ComponentAttributes::None, {})


void ShakeButton::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
	if (Button* button = GetOwner()->GetComponent<Button>())
	{
		button->AddOnClickEvent([this]() {
			std::vector<Ball*> balls = GetScene()->FindComponents<Ball>(); // シーン内の全ての Ball コンポーネントを取得する例
			for (Ball* ball : balls)
			{
				GameObject* ballObject = ball->GetOwner();
				if (ballObject)
				{
					Rigidbody* rb = ball->GetOwner()->GetComponent<Rigidbody>();
					if (rb)
					{
						rb->WakeUp(); // すべての Ball コンポーネントの Rigidbody を起こす例
						rb->SetKinematic(false); // すべての Ball コンポーネントの Rigidbody をキネマティックでなくする
						Vector3 randomForce = Vector3(
							(static_cast<float>(rand()) / RAND_MAX - 0.5f), // X 軸のランダムな力
							0.0f, // Y 軸の力はゼロにする例
							(static_cast<float>(rand()) / RAND_MAX - 0.5f)  // Z 軸のランダムな力
						);
						randomForce = randomForce.Normalize() * 0.01f; // 力の大きさを調整する例

						rb->AddForce(randomForce, ForceMode::Impulse); // ランダムな力を加える例
					}
				}
			}
			// Shaker コンポーネントへの参照を取得
			if (Shaker* shaker = GetScene()->FindComponentById<Shaker>(shakerReference))
			{
				shaker->Shake(0.1f, 0.03f); // 仮の Shake メソッドを呼び出す例
			}

			GetOwner()->SetActive(false); // ボタンを非アクティブにする例
			});
	}
}

void ShakeButton::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
}