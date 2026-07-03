#include "pch.h"
#include "History.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"


namespace CurryEngine
{


	UndoRedoStack& History::GetUndoRedoStack()
	{
		Scene* currentScene = SceneManager::GetCurrentScene();
		if (currentScene) { // 現在のシーンが存在する場合は、そのシーンのUndoRedoStackを返す
			return currentScene->undoRedoStack;
		}
		static UndoRedoStack dummyStack; // シーンがない場合のダミー
		return dummyStack;
	}
}