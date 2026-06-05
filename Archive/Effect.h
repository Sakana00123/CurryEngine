#pragma once
#include "Engine/Core/Component.h"
#include "EffectManager.h"
#include "Engine/Core/GameObject.h"

class Effect : public Component
{
public:
	Effect(const char* filePath) {
		//Effekseerエフェクト読み込み
		handle = EffectManager::LoadRequest(filePath);
	}
	~Effect() override {
		Stop();
	}

	void Update(float elapsedTime) override {
		if (isPlaying) {
			XMFLOAT3 position = gameObject->transform->WorldPosition();
			XMFLOAT3 scale = gameObject->transform->scale;
			XMFLOAT4 rotation = gameObject->transform->WorldRotation();

			EffectManager::SetScale(handle, scale);
			EffectManager::SetRotation(handle, rotation);
			EffectManager::SetPosition(handle, position);
		}
	}

	void OnEnable() override {
		Play();
	}

	void OnDisable() override {
		Stop();
	}

	void Play() {
		if (!isPlaying) {
			EffectManager::PlayRequest(handle);
			isPlaying = true;
		}
	}

	void Stop() {
		if (isPlaying) {
			EffectManager::StopRequest(handle);
			isPlaying = false;
		}
	}

	bool IsPlaying() const { return isPlaying; }

	void DrawProperty() override {
#ifdef USE_IMGUI
		//ImGui::Checkbox("isPlaying", &isPlaying);
		if (ImGui::Button(isPlaying ? "Stop" : "Play")) {
			if (isPlaying) Stop();
			else Play();
		}
		int handle = this->handle;
		ImGui::InputInt("handle", &handle);
		XMFLOAT3 world = gameObject->transform->WorldPosition();
		ImGui::InputFloat3("worldPosition", &world.x);
		//XMFLOAT3 s = gameObject->transform->WorldScale();
		//ImGui::InputFloat3("worldScale", &s.x);
		XMFLOAT4 r = gameObject->transform->WorldRotation();
		ImGui::InputFloat4("worldRotation", &r.x);
		XMFLOAT3 a = gameObject->transform->GetEulerAngles();
		ImGui::InputFloat3("eulerAngle", &a.x);
#endif // USE_IMGUI
	}

private:
	EffectManager::UserHandle handle = -1;
	bool isPlaying = false;
};