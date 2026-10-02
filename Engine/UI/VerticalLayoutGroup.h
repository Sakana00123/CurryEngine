#pragma once
#include "LayoutGroup.h"

/** @brief VerticalLayoutGroup を表すクラスです。 */
class VerticalLayoutGroup : public LayoutGroup
{
	C_REFLECT(VerticalLayoutGroup)
public:
	/**
	 * @brief VerticalLayoutGroup を構築します。
	 */
	VerticalLayoutGroup() = default;
	/**
	 * @brief VerticalLayoutGroup を破棄します。
	 */
	~VerticalLayoutGroup() = default;

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

protected:

	// 配置を更新する関数。子要素の位置を計算して配置します。
	/**
	 * @brief 状態を更新します。
	 */
	void UpdateLayout() override;


private:

	C_PROPERTY()
	bool childForceExpandWidth = false; // 子要素の幅を強制的に親の幅に合わせるかどうか

	C_PROPERTY()
	bool childForceExpandHeight = false; // 子要素の高さを強制的に親の高さに合わせるかどうか

	C_PROPERTY()
	bool childControlWidth = true; // 子要素の幅をレイアウトグループが制御するかどうか

	C_PROPERTY()
	bool childControlHeight = true; // 子要素の高さをレイアウトグループが制御するかどうか

};