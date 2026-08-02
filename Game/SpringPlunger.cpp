#include "pch.h"
#include "SpringPlunger.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Audio/Audio.h"
#include "PhaseManager.h"
#include "Engine/Scenes/Scene.h"

REGISTER_COMPONENT(SpringPlunger, "SpringPlunger")

void SpringPlunger::Initialize()
{
	// 初期位置を保存
	initialPosition = GetTransform()->GetPosition();
	currentDownOffset = Vector3::Zero;
	isPressed = false;
	pressedElapsed = 0.0f;
	maxReturnSpeed = 3.0f;
	returnSpeedCurveExponent = 0.5f;
}

void SpringPlunger::Update(float deltaTime)
{
	PhaseManager* phaseManager = GetScene()->GetObjectManager()->Find("PhaseManager")->GetComponent<PhaseManager>();
	if (!phaseManager || phaseManager->GetCurrentPhase() != PhaseManager::Playing)
		return;

	// マウス左クリック判定
	bool mouseDown = ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0) && canInteract;

	if (mouseDown && !isPressed)
	{
		// クリック開始
		isPressed = true;
		pressedElapsed = 0.0f;
	}
	else if (!mouseDown && isPressed)
	{
		// クリック解放 - ボール発射
		isPressed = false;
		LaunchBall(pressedElapsed);
		Audio::PlayOneShot(L"./Assets/Sounds/SE/launchBall.wav", 0.5f);
	}

	// 下降処理
	if (isPressed)
	{
		//マウスの移動量を取得
		int mouseDx, mouseDy;
		InputSystem::GetMouseDelta(mouseDx, mouseDy);

		float sensitivity = 0.0008f; // 基本のマウス移動の感度

		// マウスを押し上げている（mouseDy < 0、つまりz座標を正の値に戻そうとしている）場合は感度を下げる
		if (mouseDy < 0)
		{
			// 0.2fなどの倍率をかけることで、押し上げ時の移動量をゆっくりにする
			sensitivity *= 0.5f;
		}

		// マウスを下(Yの正方向)に一定量動かしたら、プランジャーをZの負方向に下げる
		float newDownDistance = currentDownOffset.z - mouseDy * sensitivity;

		// 上限(0.0f)と下限(-maxDownDistance)にクランプ
		newDownDistance = (std::min)(newDownDistance, 0.0f);
		newDownDistance = (std::max)(newDownDistance, -maxDownDistance);

		currentDownOffset.z = newDownDistance;

		// 引っ張った距離から、擬似的な経過時間を逆算
		float pullRatio = std::abs(currentDownOffset.z) / maxDownDistance;
		pressedElapsed = pullRatio * maxPressTime;

		// 横揺れ計算
		// 押下時間の比率（0?1）からカーブで振幅を決定
		float pressRatio = (std::min)(pressedElapsed / maxPressTime, 1.0f);
		float curvedRatio = std::pow(pressRatio, shakeCurveExponent);
		float amplitude = maxShakeAmplitude * curvedRatio;

		shakeTime += deltaTime;
		currentShakeOffset = amplitude * std::sin(shakeTime * shakeFrequency * 2.0f * 3.14159f);

	}
	else
	{
		// 押下時間に応じた戻る速度を取得
		float currentReturnSpeed = GetReturnSpeed(pressedElapsed);

		// 元の位置に戻す（Z軸）
		if (std::abs(currentDownOffset.z) > 0.001f)
		{
			currentDownOffset.z += currentReturnSpeed * deltaTime;
			if (currentDownOffset.z > 0.0f)
				currentDownOffset.z = 0.0f;

			//戻るときも残り距離に応じてpressedElapsedを増やす
			float returnRatio = std::abs(currentDownOffset.z) / maxDownDistance;
			pressedElapsed = returnRatio * maxPressTime;
		}

		// 横揺れを素早く減衰させて止める
		shakeTime = 0.0f;
		currentShakeOffset *= 0.85f;   // 指数減衰（毎フレーム85%に）
		if (std::abs(currentShakeOffset) < 0.001f)
			currentShakeOffset = 0.0f;
	}

	//位置を更新（X軸シェイク込み）
	GetTransform()->SetPosition(
		initialPosition + Vector3(currentShakeOffset, 0.0f, currentDownOffset.z)
	);
}

void SpringPlunger::LaunchBall(float pressTime)
{
	// 最小押下時間に達していない場合は発射しない
	if (pressTime < minPressTime)
	{
		return;
	}

	// 押下時間からパワーを計算
	float powerRatio = (std::min)(pressTime / maxPressTime, 1.0f);

	// パワーカーブの適用
	float curvedRatio = std::pow(powerRatio, 1.0f / powerCurveExponent);

	// 最小パワーから最大パワーまで補間
	float launchPower = minLaunchPower + (maxLaunchPower - minLaunchPower) * curvedRatio;

	// 押下時間が非常に短い場合でも最小パワーを保証
	if (pressTime > minPressTime)
	{
		launchPower = (std::max)(launchPower, minLaunchPower);
	}

	
}

float SpringPlunger::GetReturnSpeed(float pressTime) const
{
	// 押下時間の比率を計算（0.0 = 最小、1.0 = 最大）
	float pressTimeRatio = (std::min)(pressTime / maxPressTime, 1.0f);

	// 非線形曲線を適用：短い時間でも速度が上がりやすくする
	float curvedRatio = std::pow(pressTimeRatio, 1.0f / returnSpeedCurveExponent);

	// 押下時間に応じて戻る速度を補間
	float returnSpeed = minReturnSpeed + (maxReturnSpeed - minReturnSpeed) * curvedRatio;

	return returnSpeed;
}