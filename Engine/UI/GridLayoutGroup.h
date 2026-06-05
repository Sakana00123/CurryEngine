#pragma once
#include "LayoutGroup.h"

class GridLayoutGroup : public LayoutGroup
{
	C_REFLECT(GridLayoutGroup)
public:
	GridLayoutGroup() = default;
	~GridLayoutGroup() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;


	// 配置を更新する関数。子要素の位置を計算して配置します。
	void UpdateLayout() override;

private:

	C_PROPERTY()
	Vector2 cellSize = { 100.0f, 100.0f }; // セルのサイズ

};