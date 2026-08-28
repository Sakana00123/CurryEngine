#include "pch.h"
#include "AnimatorControllerEditor.h"
#include "Engine/Resources/AssetDatabase.h"

AnimatorControllerEditor::AnimatorControllerEditor()
{
#ifdef USE_IMGUI
#endif // USE_IMGUI
}

AnimatorControllerEditor::~AnimatorControllerEditor()
{
#ifdef USE_IMGUI
#endif // USE_IMGUI
}

void AnimatorControllerEditor::Open()
{
	s_isOpen = true;
#ifdef USE_IMGUI
	s_editorWindow = std::make_unique<CurryEngine::Editor::AnimatorControllerEditorWindow>(s_animatorController);
#endif // USE_IMGUI

}

void AnimatorControllerEditor::Close()
{
	s_isOpen = false;
#ifdef USE_IMGUI
	s_editorWindow = nullptr;
#endif // USE_IMGUI

}

bool AnimatorControllerEditor::IsOpen()
{
	return s_isOpen;
}

void AnimatorControllerEditor::OpenAsset(const std::filesystem::path& path)
{
	auto meta = CurryEngine::Resources::AssetDatabase::GetOrImport(path);
	if (!meta)
	{
		LOG_ERROR("Failed to get or import asset meta: " + path.string());
		return;
	}

	// アセットの種類がAnimatorControllerであることを確認
	if (meta->type != AssetType::AnimatorController)
	{
		LOG_ERROR("Asset is not an AnimatorController: " + path.string());
		return;
	}

	// アセットIDを使用してAnimatorControllerをロード
	auto animatorController = CurryEngine::Resources::AssetDatabase::LoadAsset<AnimatorController>(meta->id);
	if (!animatorController)
	{
		LOG_ERROR("Failed to load AnimatorController asset: " + path.string());
		return;
	}

	// エディタを開き、ロードしたAnimatorControllerを設定
	s_animatorController = animatorController;
	Open();
}

void AnimatorControllerEditor::DrawGUI()
{
#ifdef USE_IMGUI
	if (!s_isOpen) return;
	if (!s_editorWindow) 
	{
		LOG_ERROR("AnimatorControllerEditorWindow is not initialized.");
		return;
	}
	// エディタウィンドウの描画
	s_editorWindow->Draw(&s_isOpen, s_animatorController);

#endif // USE_IMGUI
}
