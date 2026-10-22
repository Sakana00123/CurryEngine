#include "pch.h"
#include "SceneParametersEditor.h"
#include "Engine/Core/Time.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"
#include "Engine/Rendering/Camera/EditorCamera.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void SceneParametersEditor::Show()
{
	isOpen = true;
}

void SceneParametersEditor::DrawGUI()
{
#ifdef USE_IMGUI
	if (isOpen)
	{
		ImGui::Begin(reinterpret_cast<const char*>(u8"シーンパラメーター###SceneParametersEditor"), &isOpen);

		ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
		ImGui::Text(reinterpret_cast<const char*>(u8"経過時間: %f"), Time::DeltaTime());
		ImGui::Text(reinterpret_cast<const char*>(u8"非スケール経過時間: %f"), Time::UnscaledDeltaTime());
		ImGui::DragFloat(reinterpret_cast<const char*>(u8"時間倍率"), &Time::timeScale, 0.01f, 0.0f, 100.0f);

		// シーン名表示
		if (Scene* scene = SceneManager::GetCurrentScene())
		{
			ImGui::SeparatorText(reinterpret_cast<const char*>(u8"エディターカメラ"));
			scene->GetEditorCamera(EDITOR_CAMERA_SCENE_VIEW)->DrawProperty();
			ImGui::SeparatorText(scene->name.c_str());
			// シーンパラメータ表示
			scene->DrawGUI();
		}
		ImGui::End();
	}
#endif // USE_IMGUI
}