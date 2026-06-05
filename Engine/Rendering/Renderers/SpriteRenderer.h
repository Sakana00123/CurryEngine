#pragma once
#include "Engine/Core/Component.h"
#include "Archive/sprite.h"
#include <memory>
#include "Engine/Core/Color.h"
#include "Engine/UI/RectTransform.h"
#include "Engine/Core/GameObject.h"

#include "Engine/Rendering/Camera/CameraComponent.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"

class SpriteRenderer : public Component
{
public:
	SpriteRenderer(ID3D11Device* device, const wchar_t* filePath = nullptr) {
		sprite = filePath ? std::make_unique<Sprite>(device, filePath) : std::make_unique<Sprite>(device);
	}
	virtual ~SpriteRenderer() override = default;

	void Initialize() override {
		rect = gameObject->AddComponent<RectTransform>();
		rect->size.x = sprite->GetTextureWidth();
		rect->size.y = sprite->GetTextureHeight();
	}

	void Draw(RenderContext* rtx) override 
	{
		auto immediateContext = rtx->immediateContext;

		XMFLOAT2 screenPos = rect->GetWorldPosition();
		
		if (threeD) {
			//Œ»Ý‚ÌƒJƒƒ‰‚ÌView‚ÆProjection‚ð‹‚ß‚é
			auto cam = SceneManager::GetCurrentScene()->cameraSystem.GetMainCamera();
			XMMATRIX View = cam->GetViewMatrix();
			XMMATRIX Projection = cam->GetProjectionMatrix();
			float screenSizeX, screenSizeY;
			Graphics::GetScreenSize(screenSizeX, screenSizeY);

			XMVECTOR ScreenPosition = XMVector3Project(
				XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&GetTransform()->GetWorldPosition())),
				0.0f,
				0.0f,
				screenSizeX,
				screenSizeY,
				0.0f,
				1.0f,
				Projection,
				View,
				XMMatrixIdentity()
			);
			XMStoreFloat2(&screenPos, ScreenPosition);
		}

		sprite->Render(immediateContext, screenPos.x, screenPos.y, rect->size.x, rect->size.y, color, angle);
	}

	void DrawProperty() override {
#ifdef USE_IMGUI
		ImGui::ColorEdit4("Color", &color.r);
		ImGui::DragFloat("Angle", &angle);
#endif // USE_IMGUI
	}

public:
	Color color;
	float angle = 0.f;
	RectTransform* rect = nullptr;
	bool threeD = false;
private:
	std::unique_ptr<Sprite> sprite;
};
