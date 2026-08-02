#include "pch.h"
#include "SceneTransitionTest.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Scenes/SceneManager.h"

REGISTER_COMPONENT(SceneTransitionTest, "Test")

void SceneTransitionTest::Start()
{

}
void SceneTransitionTest::Update(float deltaTime)
{
	if (InputSystem::GetInputState(transitionKey, InputStateMask::Trigger))
	{
		SceneManager::ChangeScene(nextSceneName);
	}
}
