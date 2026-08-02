#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

class SceneTransitionTest : public Component
{
	C_REFLECT(SceneTransitionTest)
public:
	/**@brief シーン遷移のトリガーとなるキー。*/
	C_PROPERTY()
	std::string transitionKey = "Enter";

	/**@brief 遷移先のシーン名。*/
	C_PROPERTY()
	std::string nextSceneName = ""; // 遷移先のシーン名

public:
	SceneTransitionTest() = default;
	virtual ~SceneTransitionTest() = default;
	void Start() override;
	void Update(float deltaTime) override;
};