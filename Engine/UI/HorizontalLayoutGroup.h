#pragma once
#include "LayoutGroup.h"

/** @brief HorizontalLayoutGroup を表すクラスです。 */
class HorizontalLayoutGroup : public LayoutGroup
{
	C_REFLECT(HorizontalLayoutGroup)
public:
	/**
	 * @brief HorizontalLayoutGroup を構築します。
	 */
	HorizontalLayoutGroup() = default;
	/**
	 * @brief HorizontalLayoutGroup を破棄します。
	 */
	~HorizontalLayoutGroup() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	/**
	 * @brief Start の処理を行います。
	 */
	void Start() override;

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