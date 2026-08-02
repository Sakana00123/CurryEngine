#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Editor/Console.h"

class PreserveValue : public Component
{
	C_REFLECT(PreserveValue)
public:
	PreserveValue() = default;
	~PreserveValue() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;

private:
	C_PROPERTY()
		int preservedValue = 0;

	//目標金額 (確認用のため、エディタで変更できないようにする)
	C_PROPERTY(CurryEngine::PropertyAttributes::NonSerialized, CurryEngine::PropertyAttributes::ReadOnly)
		int targetValue = 1000;

	// ** @brief 全体の金額の保存先 */
	C_PROPERTY()
		int totalValue = 0;

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId textReference; // 例: Text コンポーネントへの参照
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId textReference2; // 例: Text コンポーネントへの参照

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId totalTextReference; // 例: text コンポーネントへの参照
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Text"))
		ObjectId totalTextReference2; // 例: text コンポーネントへの参照

public:
	//ボールの価値を保存する関数
	/** @brief ボールの価値を保存します。*/
	void SaveBallValue(int value);

	/** @brief 保存されたボールの価値を取得します。*/
	int GetPreservedValue() const { return preservedValue; }

	/// ** @brief 目標金額に達しているかをチェックします。*/
	bool IsTargetValueReached() const { return totalValue >= targetValue; }

	// ** @brief 金額を減らす関数 */
	void DecreaseTotalValue(int amount);

	// ** @brief 金額を保存しておく関数 */
	void SaveTotalValue(int value, bool isBall = true);

	// ** @brief 全体の金額を取得します。*/
	int GetTotalValue() const { return totalValue; }

	// ** @brief 全体の金額をリセットする関数 */
	void ResetTotalValue();

	// ** @brief 現在の金額をリセットする関数 */
	void ResetPreservedValue();

	// ** @brief 目標金額を設定する関数 */
	void SetTargetValue(int value);

	int GetTargetValue() const { return targetValue; }

	// ** @brief 目標金額を達成したときの処理を行う関数 */
	void OnTargetValueAchieved();

	// ** @brief 金額を表示するテキストを更新する関数 */
	void UpdateUIText();

	void SetWasUseCoinLastShop(bool used) { wasUseCoinLastShop = used; }

	bool GetWasUseCoinLastShop() const { return wasUseCoinLastShop; }

private:
	bool isPowerOfTwo(int n) {
		// 1. n > 0 であることを確認 (0や負の数は2のべき乗ではない)
		// 2. n & (n - 1) が 0 になれば、ビットが1つしか立っていない＝2のべき乗
		return (n > 0) && ((n & (n - 1)) == 0);
	}

	int decreaseCounter = 0; // 金額減少の回数を追跡するカウンター
	bool wasUseCoinLastShop = false; // 前回のショップでコインを使用したかどうかを追跡するフラグ
};