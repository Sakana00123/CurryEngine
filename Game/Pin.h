#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Core/GameObject.h"
class ParticleComponent;

class Pin : public Component
{
	C_REFLECT(Pin)

public:

	//ゴールデンピンかどうか
	C_PROPERTY()
		bool isGoldenPin = false;

	GameObject* equippedObject = nullptr; // 装備されているオブジェクトへのポインタ
	ParticleComponent* gadgetSpawnEffect = nullptr; // ガジェットがスポーンする際のエフェクト
	ParticleComponent* obtrusiveGadgetSpawnEffect = nullptr; // 妨害ガジェットがスポーンする際のエフェクト
	ParticleComponent* gadgetBreakEffect = nullptr; // ガジェットが壊れる際のエフェクト
public:
	Pin() = default;
	virtual ~Pin() = default;
	void Start() override;
	void Update(float deltaTime) override;

	//ピンにオブジェクトを装備する
	virtual void Equip(GameObject* obj, const Vector3& spawnPosition, const Quaternion& spawnRotation, const Vector3& spawnScale);

	//ピンからオブジェクトを外す
	virtual GameObject* Unequip();

	// 壊れるときのエフェクトを再生
	void PlayBreakEffect();

	//装備しているか
	bool IsEquipped() const { return equippedObject != nullptr; }

	//ゴールデンピンかどうか
	virtual bool IsGoldenPin() const { return isGoldenPin; }

	//ピンかどうか
	bool IsPin() const { return !IsGoldenPin(); }

};

class GoldenPin : public Pin
{
	C_REFLECT(GoldenPin)
public:
	GoldenPin() { isGoldenPin = true; }

	void Start() override;
	void Update(float deltaTime) override;

	void Equip(GameObject* obj, const Vector3& spawnPosition, const Quaternion& spawnRotation, const Vector3& spawnScale) override;

	GameObject* Unequip() override;

	bool IsGoldenPin() const override { return true; }

	bool isHit = false; // 当たったかどうかを管理
	void Reset() { isHit = false; }

};