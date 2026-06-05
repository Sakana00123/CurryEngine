#pragma once
#include "Engine/Core/GameObject.h"
#include "Engine/Core/Transform.h"

#include "Engine/Input/InputSystem.h"
#include "Engine/Physics/Rigidbody.h"
#include "Engine/Rendering/Camera/Camera.h"
#include "Engine/Easing/EasingHandler.h"

#include "Engine/Audio/AudioSource.h"

#include <algorithm>

class Character : public Component
{
public:
	static constexpr Vector3 Up{ 0,0,1 };
	static constexpr Vector3 Down{ 0,0,-1 };
	static constexpr Vector3 Left{ -1,0,0 };
	static constexpr Vector3 Right{ 1,0,0 };
	static constexpr float offsetY = 0.5f;
protected:
	Vector3 direction{ 0,0,0 };
	int seLoopCount = 0;
	bool endTurnFront = false;
public:
	Character() = default;
	virtual ~Character() {}

	void OnEnable() override {}

	virtual void Initialize() override {

	}

	virtual void Update(float elapsedTime) override {
		if (isMoving) {
			XMFLOAT3 pos = gameObject->transform->WorldPosition();
			XMVECTOR Pos = XMLoadFloat3(&pos);
			XMVECTOR TargetPos = XMLoadFloat3(&targetPos);

			float progress = elapsedTime * 5.0f;

			if (handler.GetSequenceCount() > 0) {
				handler.Update(tweenProgress, elapsedTime);
				Pos = XMLoadFloat3(&startPos);
				progress = tweenProgress;
			}

			XMStoreFloat3(&pos, XMVectorLerp(Pos, TargetPos, progress));
			static const float threshold = 0.01f;
			if (abs(targetPos.x - pos.x) < threshold && abs(targetPos.z - pos.z) < threshold) {
				pos = targetPos;
				isMoving = false;
				OnCompleteMovement();
			}
			gameObject->transform->SetWorldPosition(pos);
		}
		if (destroyTimer > 0.f)
		{
			XMFLOAT3 pos = gameObject->transform->WorldPosition();
			XMVECTOR Pos = XMLoadFloat3(&pos);

			// 水平方向の計算
			// position = start + Vxz * directionXZ * elapsedTime;
			DirectX::XMVECTOR Horizontal = DirectX::XMVectorAdd(Pos, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&directionXZ), Vxz * elapsedTime));

			// 垂直方向の計算
			// position = start + Vy * t - 1/2 * g * t^2;
			float verticalY = pos.y + Vy * elapsedTime /*- 0.5f * gravity * elapsedTime * elapsedTime*/;

			// 
			DirectX::XMStoreFloat3(&newPosition, Horizontal);
			newPosition.y = verticalY;

			gameObject->transform->SetWorldPosition(newPosition);

			destroyTimer -= elapsedTime;
			if (destroyTimer <= 0.f) {
				Destroy();
			}
		}

#ifdef USE_IMGUI
		ImGui::Begin("Character Physics");
		ImGui::SliderFloat("degree", &theta, 0.0f, 360.0f, "%.2f");
		ImGui::SliderFloat("flyTime", &flyTime, 0.0f, 4.0f, "%.5f");
		ImGui::DragFloat("gravity", &gravity, 0.3f);
		ImGui::End();
		//ImGui::DragFloat("powerY", &powerY);
#endif // USE_IMGUI

	}
	void Render(RenderContext* immediateContext) override {}

	void DrawProperty() override {
#ifdef USE_IMGUI
		ImGui::DragFloat("tweenProgress", &tweenProgress);
		//ImGui::DragFloat("power", &power);
		//ImGui::DragFloat("powerY", &powerY);
		//ImGui::DragFloat("flyTime", &flyTime);
#endif // USE_IMGUI
	}

	virtual void Move(const XMFLOAT3& target, bool allowInterrupt) {
		if (allowInterrupt || !isMoving) {
			targetPos = target;
			isMoving = true;
			//AudioSourceを持っていたら再生する。
			if (AudioSource* source = gameObject->GetComponent<AudioSource>()) {
				source->Play();
			}
		}
	}

	virtual void EaseMove(const XMFLOAT3& target, bool allowInterrupt, EaseType type, bool endTurnFront = false) {
		if (allowInterrupt || !isMoving) {
			startPos = gameObject->transform->WorldPosition();
			targetPos = target;
			isMoving = true;
			float length = XMVectorGetX(XMVector3Length(XMLoadFloat3(&target) - XMLoadFloat3(&startPos)));
			length *= 0.5f;
			handler.Clear();
			handler.AddEasing(type, 0.f, 1.f, length);
			//AudioSourceを持っていたら再生する。
			if (AudioSource* source = gameObject->GetComponent<AudioSource>()) {
				source->Play();
			}
		}
		this->endTurnFront = endTurnFront;
	}

	void Death(const XMFLOAT3& target, const XMFLOAT3& start, float destroyTime = 2.0f)
	{
		Rigidbody* rb = gameObject->GetComponent<Rigidbody>();
		rb->useGravity = true;
#if 0
		XMFLOAT3 impulseDirection = target;

		impulseDirection.x = (target.x - start.x) / flyTime;
		impulseDirection.z = (target.z - start.z) / flyTime;

		float length = XMVectorGetX(XMVector3Length(XMLoadFloat3(&target) - XMLoadFloat3(&start)));
		impulseDirection.y = (target.y - start.y + 0.5f * gravity * flyTime * flyTime) / flyTime;

		float y = max(start.y, target.y) + height;
		float vy = sqrtf(2.0f * gravity * (y - start.y));


		//impulseDirection.y = 0.75f + 0.5f * powerFactor;

		//rb->AddForce(impulseDirection);
#else
		// length = target - position
		float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&start))));
		float v0 = length / (cosf(DirectX::XMConvertToRadians(theta)) * flyTime);
		Vxz = v0 * cosf(DirectX::XMConvertToRadians(theta));
		Vy = v0 * sinf(DirectX::XMConvertToRadians(theta));

		DirectX::XMVECTOR ToTargetDir = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&start)));
		DirectX::XMVECTOR DirectionXZ = DirectX::XMVectorSet(DirectX::XMVectorGetX(ToTargetDir), 0.0f, DirectX::XMVectorGetZ(ToTargetDir), 0.0f);
		DirectX::XMStoreFloat3(&directionXZ, DirectionXZ);
		//削除タイマーをセット
		destroyTimer = destroyTime;

		gameObject->GetComponent<BoxCollider>()->SetEnable(false);

		//SE再生
		if (GameObject* soundObj = ObjectManager::Find("BlowAwaySound")) {
			soundObj->GetComponent<AudioSource>()->Play();
		}
#endif
	}

	virtual void OnCompleteMovement() {};

	virtual void SetColor(const Color& color) { this->color = color; }

	//パラメータは、正規化しなければならない。
	void SetDirection(const Vector3& dir) { direction = dir; }
	void SetDirection(XMFLOAT3& target, bool flatMove = true) {
		XMFLOAT3 position = gameObject->transform->WorldPosition();
		if (flatMove) target.y = position.y = offsetY;
		XMVECTOR Dir = XMVector3Normalize(XMLoadFloat3(&target) - XMLoadFloat3(&position));
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&direction), Dir);
	}
	Vector3 GetDirection() const { return direction; }


	float speed = 4.0f;

	Color color;

	XMFLOAT3 startPos;
	XMFLOAT3 targetPos;
	bool isMoving = false;

private:
	float tweenProgress = 0.0f;

private:
	float height = 3.0f;
	//float gravity = 9.8f;

	float destroyTimer = 0.0f;
	float power = 300.f;
	float powerY = 200.0f;
	EasingHandler handler;

	float gravity = 20.0f;
	float flyTime = 0.5f;
	float theta = 36.0f; // degree
	float Vxz = 0.0f;
	float Vy = 0.0f;
	DirectX::XMFLOAT3 directionXZ = { 0.0f,0.0f,0.0f };
	DirectX::XMFLOAT3 newPosition = { 0.0f,0.0f,0.0f };
};