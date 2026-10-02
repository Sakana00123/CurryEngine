#pragma once

#define X3DAUDIO
#ifdef X3DAUDIO
#include <x3daudio.h>

class GameObject;

/** @brief C3DAudio を表すクラスです。 */
class C3DAudio
{
private:
	static inline X3DAUDIO_HANDLE x3dAudioHandle;
public:
	/**
	 * @brief 初期化します。
	 */
	static void Initialize();
	static inline X3DAUDIO_HANDLE* GetHandle() { return &x3dAudioHandle; }

	/**
	 * @brief Culculate3DAudio の処理を行います。
	 */
	static void Culculate3DAudio(GameObject* source);
};

#endif // X3DAUDIO