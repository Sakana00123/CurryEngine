#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Core/Transform.h"

using namespace CurryEngine::PropertyAttributes;

/**
 * @file
 * @brief シーンのビュー/プロジェクションを提供するカメラコンポーネント。
 * @details 視野角・アスペクト比・近遠クリップ面からプロジェクション行列を生成し、
 *          `Transform` と前方向/上方向ベクトルからビュー行列を生成します。
 */
class CameraComponent : public Component
{
	C_REFLECT(CameraComponent)
public:
	/** @brief 既定コンストラクタ。*/
	CameraComponent() = default;
	/**
	 * @brief デストラクタ。
	 */
	~CameraComponent() override = default;

	/** @brief 初期化処理。必要な参照の取得などを行います。*/
	void Initialize() override;

	/** @brief インスペクタ用のプロパティ描画。*/
	void DrawProperty() override;

	/** @brief 有効化時のコールバック。*/
	void OnEnable() override;

	/** @brief 無効化時のコールバック。*/
	void OnDisable() override;

	/**
	 * @brief スクリーン座標をワールド空間のレイに変換します。
	 * @param screenPos スクリーン座標（ピクセル単位）。左上が(0,0)で、右下が(画面幅, 画面高さ)。
	 * @param outOrigin レイの原点（ワールド空間）。
	 * @param outDirection レイの方向（ワールド空間、正規化されていることが期待される）。
	 */
	void ScreenPointToRay(const Vector2& screenPos, Vector3& outOrigin, Vector3& outDirection) const;


	/**
	 * @brief ビュー行列を取得します。
	 * @return ビュー行列。
	 */
	XMMATRIX GetViewMatrix() const;

	/**
	 * @brief プロジェクション行列を取得します。
	 * @return プロジェクション行列。
	 */
	XMMATRIX GetProjectionMatrix() const;


	float GetFieldOfView() const { return fieldOfView; }

	void SetFieldOfView(float fov) { fieldOfView = fov; }

	float GetAspect() const { return aspect; }

	void SetAspect(float aspect) { this->aspect = aspect; }

	float GetNearClip() const { return nearClip; }

	void SetNearClip(float nearClip) { this->nearClip = nearClip; }

	float GetFarClip() const { return farClip; }

	void SetFarClip(float farClip) { this->farClip = farClip; }

	bool IsMainCamera() const { return isMainCamera; }

private:
	/** @brief 視野角（度）。*/
	C_PROPERTY(Range(1.0f, 120.0f), Tooltip("fov"))
	float fieldOfView = 60.0f;
	/** @brief アスペクト比（幅/高さ）。*/
	C_PROPERTY(ReadOnly)
	float aspect = 1280.0f / 720.0f;
	/** @brief 近クリップ面距離。*/
	C_PROPERTY(Speed(0.01f), Range(0.01f, 10.0f))
	float nearClip = 0.1f;
	/** @brief 遠クリップ面距離。*/
	C_PROPERTY(Speed(1.0f), Range(100.0f, 10000.0f))
	float farClip = 1000.0f;
	/** @brief メインカメラかどうか。*/
	C_PROPERTY(HideInInspector)
	bool isMainCamera = false;
};