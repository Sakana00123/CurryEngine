#pragma once
#include "Engine/Core/Component.h"
#include "3DAudio.h"

#ifdef X3DAUDIO
/**
 * @brief オーディオリスナーコンポーネント。
 * @details 3D サウンドの聴取者（カメラ等）を表現します。
 */
class AudioListener : public Component
{
private:
	/** @brief 現在のリスナー。*/
	static inline AudioListener* listener;
public:
	/** @brief 既定コンストラクタ。*/
	AudioListener() = default;
	/** @brief デストラクタ。*/
	virtual ~AudioListener() override;

	/**
	 * @brief 初期化処理。
	 * @details 最初に生成されたリスナーを現在のリスナーとします。
	 */
	void Awake() override;

	/**
	 * @brief フレーム更新。
	 * @param deltaTime 経過時間（秒）。
	 */
	void Update(float deltaTime) override;

	/**
	 * @brief 現在のリスナーを取得します。
	 * @return 現在のリスナー（未設定時は `nullptr`）。
	 */
	static inline AudioListener* GetListener() { return listener; }

};
#endif // X3DAUDIO
