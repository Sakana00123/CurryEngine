#pragma once
#include "Engine/Core/Transform.h"

/**
 * @file
 * @brief デバッグ用途の自由視点カメラ。
 * @details 入力に応じて移動・回転できる仮想カメラです。速度や回転速度、
 *          X 軸の仰俯角クランプを持ち、プロパティから調整できます。
 */
class EditorCamera
{
private:
	static inline Vector3 position;
	static inline Quaternion rotation = { 0,0,0,1 };

	/** @brief ターゲットからの距離。*/
	static inline float distance = 10.0f;
	/** @brief 最小距離。*/
	static inline float minDistance = 0.1f;
	/** @brief 最大距離。*/
	static inline float maxDistance = 1000.f;
	/** @brief 平行移動の速度（ユニット/秒）。*/
	static inline float speed = 3.0f;
	/** @brief 回転速度（度/秒）。*/
	static inline float rotateSpeed = 30.0f;
	/** @brief X 軸の最大角（度）。*/
	static inline float maxAngleX = 70.0f;
	/** @brief X 軸の最小角（度）。*/
	static inline float minAngleX = -70.0f;

	/** @brief 視野角（度）。*/
	static inline float fieldOfView = 60.0f;
	/** @brief アスペクト比（幅/高さ）。*/
	static inline float aspect = 1280.0f / 720.0f;
	/** @brief 近クリップ面。*/
	static inline float nearZ = 0.1f;
	/** @brief 遠クリップ面。*/
	static inline float farZ = 1000.0f;
public:
	/** @brief 初期化処理。入力と内部状態をセットアップします。*/
	static void Initialize();

	/**
	 * @brief 毎フレーム更新。
	 * @param elapsedTime 経過時間（秒）。
	 * @details 入力による移動/回転と角度クランプを適用します。
	 */
	static void Update(float elapsedTime);

	/**
	 * @brief カメラの注視点位置を設定します。
	 * @param pos 注視点位置。
	 */
	static void SetPosition(const Vector3& pos) { position = pos; }

	/**
	 * @brief カメラ位置を取得します。
	 * @return カメラ位置。
	 */
	static Vector3 GetPosition() { return position; }

	/**
	 * @brief ビュー行列を取得します。
	 * @return ビュー行列。
	 */
	static XMMATRIX GetViewMatrix();

	/**
	 * @brief 射影行列を取得します。
	 * @return 射影行列。
	 */
	static XMMATRIX GetProjectionMatrix();

	/**
	 * @brief スクリーン座標をワールド空間のレイに変換します。
	 * @param screenPos スクリーン座標（ピクセル単位）。左上が (0,0)。
	 * @param outOrigin レイの原点（カメラ位置）。
	 * @param outDirection レイの方向（正規化されたベクトル）。
	 */
	static void ScreenPointToRay(const Vector2& screenPos, Vector3& outOrigin, Vector3& outDirection);


	/** @brief インスペクタ用プロパティ描画。*/
	static void DrawProperty();

	/**
	 * @brief 距離のクランプ範囲を設定します。
	 * @param min 最小距離。
	 * @param max 最大距離。
	 */
	static void SetClampDistance(float min, float max) { minDistance = min, maxDistance = max; }


	/** @brief シリアライズ。*/
	static json Serialize();

	/** @brief デシリアライズ。*/
	static void Deserialize(const json& j);

};