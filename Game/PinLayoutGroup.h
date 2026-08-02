#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

class PinLayoutGroup : public Component
{
	C_REFLECT(PinLayoutGroup)
public:
	PinLayoutGroup() = default;
	~PinLayoutGroup() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	//void Update(float deltaTime) override;

	void UpdateLayout(); // レイアウトの更新処理を別関数として実装

	//void DrawProperty() override; // エディタでプロパティを描画するためのオーバーライド関数

private:

	// 一列目の一列のピンの最大数
	C_PROPERTY()
	int maxPerRow = 6;

	// 少ないほうの列のピンの数
	C_PROPERTY()
	int minPerRow = 5;

	// どっちの列が一番多くするか
	C_PROPERTY()
	bool isFirstRowLonger = true;


	C_PROPERTY()
	float spacing = 0.13f; // ピン同士のスペース


	C_PROPERTY()
	Vector3 startPosition = Vector3(-0.39f, 0.45f, 0.1f); // 一列目の最初のピンの位置

};
