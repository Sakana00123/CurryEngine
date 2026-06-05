#pragma once

class Scene;
class CameraComponent;

/**
 * @file
 * @brief シーン内のカメラ管理を行うシステムクラス。
 */
class CameraSystem
{
public:
	/** @brief 既定コンストラクタ。*/
	CameraSystem() = default;
	/** @brief デストラクタ。*/
	~CameraSystem() = default;

	/** @brief 初期化処理。*/
	void Initialize(Scene* scene);

	/**
	 * @brief シーン内のカメラコンポーネントを検索し、メインカメラを設定します。
	 * @param scene カメラを検索するシーン。
	 */
	void ResolveMainCamera();

	/**
	 * @brief メインカメラを取得します。
	 * @return メインカメラの `CameraComponent*`。存在しなければ nullptr。
	 */
	CameraComponent* GetMainCamera();

	/**
	 * @brief カメラの変更を通知します。これにより、次のフレームでメインカメラの再解決が行われます。
	 * @details カメラコンポーネントの有効/無効切替や、シーン内のカメラ構成の変更があった場合に呼び出されるべきです。
	 */
	void NotifyCameraChanged();

private:
	Scene* scene = nullptr; // カメラシステムが所属するシーンへの参照
	CameraComponent* mainCamera = nullptr;
	bool needsMainCameraResolve = false; // メインカメラの再解決が必要かどうかのフラグ
};