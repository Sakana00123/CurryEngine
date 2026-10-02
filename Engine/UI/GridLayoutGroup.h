#pragma once
#include "LayoutGroup.h"

/** @brief GridLayoutGroup を表すクラスです。 */
class GridLayoutGroup : public LayoutGroup
{
	C_REFLECT(GridLayoutGroup)
public:
	/**
	 * @brief GridLayoutGroup を構築します。
	 */
	GridLayoutGroup() = default;
	/**
	 * @brief GridLayoutGroup を破棄します。
	 */
	~GridLayoutGroup() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	/**
	 * @brief Start の処理を行います。
	 */
	void Start() override;
	/**
	 * @brief 状態を更新します。
	 */
	void Update(float deltaTime) override;


	// 配置を更新する関数。子要素の位置を計算して配置します。
	/**
	 * @brief 状態を更新します。
	 */
	void UpdateLayout() override;

private:

	C_PROPERTY()
	Vector2 cellSize = { 100.0f, 100.0f }; // セルのサイズ

};