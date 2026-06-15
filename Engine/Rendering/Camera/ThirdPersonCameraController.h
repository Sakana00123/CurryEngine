#pragma once
#include "CameraComponent.h"
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

/**
 * @file
 * @brief 三人称視点カメラコントローラーコンポーネント。
 * @details `ThirdPersonCamera` を利用して、三人称視点カメラ制御を行います。
 */
class ThirdPersonCameraController : public Component
{
	C_REFLECT(ThirdPersonCameraController)
public:
	/** @brief 既定コンストラクタ。*/
	ThirdPersonCameraController() = default;
	/** @brief デストラクタ。*/
	~ThirdPersonCameraController() override = default;

	/** @brief 初期化処理。*/
	void Initialize() override;

	/** @brief 開始処理。*/
	void Start() override;

	/** @brief 更新処理。*/
	void Update(float deltaTime) override;

#ifdef USE_IMGUI
	/** @brief インスペクタ用のプロパティ描画。*/
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI


	// シリアライズ
	json Serialize() const override;

	// デシリアライズ
	void Deserialize(const json& j) override;

private:
	
	//Transform* targetTransform = nullptr; // 追従ターゲットのTransform
	//Transform* lookAtTransform = nullptr; // 注視ターゲットのTransform

	C_PROPERTY(ObjectReference("Transform"))
	ObjectId targetTransformId = ObjectId::Invalid(); // 追従ターゲットのTransformのオブジェクトID
	C_PROPERTY(ObjectReference("Transform"))
	ObjectId lookAtTransformId = ObjectId::Invalid(); // 注視ターゲットのTransformのオブジェクトID

	//C_PROPERTY(ObjectReference("GameObject"))
	//ObjectId targetObjectId = ObjectId::Invalid(); // 追従ターゲットのオブジェクトID
	//C_PROPERTY(ObjectReference("GameObject"))
	//ObjectId lookAtObjectId = ObjectId::Invalid(); // 注視ターゲットのオブジェクトID

	C_PROPERTY()
	bool isEnableAxisInput = true; // 入力（Axis）を有効化するか
	C_PROPERTY()
	bool isEnableZoomInput = true; // ズーム入力を有効化するか
	
	C_PROPERTY()
	bool invertX = false; // X軸反転
	C_PROPERTY()
	bool invertY = false; // Y軸反転
	C_PROPERTY()
	bool invertZoom = false; // ズーム反転

	C_PROPERTY()
	bool useSmoothMovement = false; // スムーズ移動を使用するか
	C_PROPERTY()
	bool useSmoothZoom = true; // スムーズズームを使用するか

	C_PROPERTY(Range(1, 500))
	float rotationSpeed = 10.f; // 回転速度

	C_PROPERTY(Speed(0.1f), Range(0.1f,10.f))
	float zoomAmount = 3.f; // ズーム量
	C_PROPERTY(Range(1,20))
	float zoomSpeed = 10.f; // ズーム速度(スムーズ時)

	C_PROPERTY(ReadOnly)
	float distance = 25.f; // カメラ距離(現在値)
	C_PROPERTY(ReadOnly)
	float targetDistance = 25.f; // 目標距離(ズーム先)
	C_PROPERTY(ReadOnly)
	float minDistance = 2.f; // 最小距離
	C_PROPERTY(ReadOnly)
	float maxDistance = 50.f; // 最大距離

	C_PROPERTY(Speed(0.1f), Range(1.0f, 20.0f))
	float followSpeed = 10.f; // 追従速度
	C_PROPERTY(Speed(0.1f), Range(1.0f, 20.0f))
	float lookAtSpeed = 10.f; // 注視速度
	C_PROPERTY()
	float minAngleX = 0.f; // X軸最小角度
	C_PROPERTY()
	float maxAngleX = 45.f; // X軸最大角度
};