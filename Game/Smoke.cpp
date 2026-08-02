#include "pch.h"
#include "Smoke.h"
#include "Engine/Effects/ParticleComponent.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/UI/RectTransform.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(Smoke, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(Smoke, "UserScripts", ComponentAttributes::None, {})


void Smoke::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。

	//ガジェットタイプを妨害ガジェットに設定
	SetGadgetType(GadgetType::ObtrusiveGadget);
}

void Smoke::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
	if(GetDurability() <= 0)
	{
		OnBreak(); // 耐久値が0以下になったときに OnBreak() を呼び出す
		smokeActive = false; // スモークが壊れたことを示すフラグを設定
	}

	// スモークがアクティブな場合の処理をここに実装します。
	Vector3 smokePos = GetOwner()->GetTransform()->GetWorldPosition();
	for (auto* gadget : GetScene()->FindComponents<Gadget>())
	{
		if (gadget)
		{
			Vector3 gadgetPos = gadget->GetTransform()->GetWorldPosition();


			if (Vector3::Distance(smokePos, gadgetPos) < 0.15f) // 例: スモークの効果範囲を半径0.15fとする
			{
				// ガジェットがスモークの効果範囲内にある場合の処理をここに実装します。
				if (gadget->durabilityRectTransform)
				{
					gadget->durabilityRectTransform->GetOwner()->SetActive(false);
				}
			}
		}
	}

}

void Smoke::OnActivate()
{
	// ガジェットがアクションを実行したときの処理をここに実装します。
	//if (ParticleComponent* particle = GetOwner()->GetComponentInChildren<ParticleComponent>())
	//{
	//	// パーティクルを再生するなどの処理をここに実装します。
	//	particle->Play();
	//	GetOwner()->GetTransform()->SetPosition(Vector3(0, -2.5f, 0)); // 例: スモークの位置を少し上に移動
	//}
}

void Smoke::OnDeactivate()
{
	// ガジェットがアクションを実行したときの処理をここに実装します。
	//if (ParticleComponent* particle = GetOwner()->GetComponentInChildren<ParticleComponent>())
	//{
	//	// パーティクルを停止するなどの処理をここに実装します。
	//	particle->Stop();
	//}
}

void Smoke::OnPreviewEnter()
{
	// ピンに近づけたときのプレビュー表示開始イベントの処理をここに実装します。
	if (ParticleComponent* particle = GetOwner()->GetComponentInChildren<ParticleComponent>())
	{
		// パーティクルを再生するなどの処理をここに実装します。
		particle->Play();
		GetOwner()->GetTransform()->SetPosition(Vector3(0, -2.5f, 0)); // 例: スモークの位置を少し上に移動
	}
}

void Smoke::OnPreviewExit()
{
	// ピンから離れたときのプレビュー表示終了イベントの処理をここに実装します。
	if (ParticleComponent* particle = GetOwner()->GetComponentInChildren<ParticleComponent>())
	{
		// パーティクルを停止するなどの処理をここに実装します。
		particle->Stop();
	}
}

void Smoke::OnAttachment()
{
	// ガジェットがオブジェクトにアタッチされたときの処理をここに実装します。
	if (ParticleComponent* particle = GetOwner()->GetComponentInChildren<ParticleComponent>())
	{
		// パーティクルを再生するなどの処理をここに実装します。
		particle->Play();
	}

	Gadget::OnAttachment(); // 基底クラスの OnAttachment() を呼び出すことで、必要な初期化処理やイベント登録などを行います。
}

void Smoke::OnRoundEnd()
{
	// ラウンド終了時の処理をここに実装します。
	DecreaseDurability(); // 例: 耐久値を減らす
}

void Smoke::OnBreak()
{
	// ガジェットが壊れたときの処理をここに実装します。
	// 例: スモークが壊れたことを示すフラグを設定するなどの処理をここに実装します。
	smokeActive = false; // スモークが壊れたことを示すフラグを設定

	auto* canvas = GetScene()->GetObjectManager()->Find("DurabilityCanvas");
	if (canvas)
	{
		for (auto* obj : canvas->GetChildren())
		{
			obj->SetActive(true); // 例: 耐久値表示のキャンバスをアクティブにする
		}
	}


	Gadget::OnBreak(); // 基底クラスの OnBreak() を呼び出すことで、必要な処理を行います。
}
