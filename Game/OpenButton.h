#pragma once
#include "Engine/UI/UIComponent.h"

class OpenButton : public Component
{
	C_REFLECT(OpenButton)
public:
	OpenButton() = default;
	~OpenButton() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

	void OnClick(); // ボタンがクリックされたときの処理を行う関数

private:

	C_PROPERTY()
	std::string openPath;

};