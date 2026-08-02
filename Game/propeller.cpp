#include "pch.h"
#include "propeller.h"

REGISTER_COMPONENT(Propeller, "Game")

void Propeller::Start()
{

	// ガジェットのタイプを味方ガジェットに設定
	SetGadgetType(GadgetType::AllyGadget);
}

void Propeller::Update(float deltaTime)
{
	    if (IsDisabled()) return; // ガジェットが無効化されている場合は何もしない

	// 回転速度をラジアンに変換
	float rotationSpeedRad = XMConvertToRadians(rotationSpeed);
	// フレームごとの回転量を計算
	float rotationAmount = rotationSpeedRad * deltaTime;
	// 現在の回転を取得
	Quaternion currentRotation = GetTransform()->GetRotation();
	// 回転軸（ここではY軸）を定義
	Vector3 rotationAxis(0, 1, 0);
	// 回転クォータニオンを作成
	Quaternion rotationQuat = Transform::QuaternionRotationAxis(rotationAxis, rotationAmount);
	// 新しい回転を計算
	Quaternion newRotation = Transform::QuaternionMultiply(rotationQuat, currentRotation);
	// 回転を適用
	GetTransform()->SetRotation(newRotation);
}

void Propeller::OnRoundEnd()
{
	DecreaseDurability(); // ガジェットの耐久値を減少させる
}

void Propeller::OnAttachment()
{
	// ガジェットがオブジェクトにアタッチされたときの処理をここに実装します。
	// 例えば、アタッチされたオブジェクトの特定のコンポーネントを取得して初期化するなどの処理が考えられます。
	Gadget::OnAttachment(); // 基底クラスの処理も呼び出す
}