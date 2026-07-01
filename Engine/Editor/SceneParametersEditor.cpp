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
		ImGui::Begin("SceneParametersEditor", &isOpen);

		ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
		ImGui::Text("DeltaTime: %f", Time::DeltaTime());
		ImGui::Text("UnScaledDeltaTime: %f", Time::UnscaledDeltaTime());
		ImGui::DragFloat("timeScale", &Time::timeScale, 0.01f, 0.0f, 100.0f);

		// シーン名表示
		if (Scene* scene = SceneManager::GetCurrentScene())
		{
			ImGui::SeparatorText("EditorCamera");
			scene->GetEditorCamera(EDITOR_CAMERA_SCENE_VIEW)->DrawProperty();
			ImGui::SeparatorText(scene->name.c_str());
			// シーンパラメータ表示
			scene->DrawGUI();
		}
		ImGui::End();
	}
#endif // USE_IMGUI
}