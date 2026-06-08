#pragma once

#define X3DAUDIO
#ifdef X3DAUDIO
#include <x3daudio.h>

class GameObject;

class C3DAudio
{
private:
	static inline X3DAUDIO_HANDLE x3dAudioHandle;
public:
	static void Initialize();
	static inline X3DAUDIO_HANDLE* GetHandle() { return &x3dAudioHandle; }

	static void Culculate3DAudio(GameObject* source);
};

#endif // X3DAUDIO