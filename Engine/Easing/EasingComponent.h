#pragma once
#include "Engine/Core/Component.h"
#include "EasingHandler.h"
#include "Engine/Core/GameObject.h"

/**
 * @file
 * @brief イージングを使って任意プロパティを時間的に補間するコンポーネント。
 * @details `EasingHandler` と `PropertyAccessor` を組み合わせ、任意の float プロパティを
 *          直列シーケンスで補間できます。インスペクタ描画や非スケール時間の使用切替にも対応。
 */

template<typename T>
struct PropertyAccessor
{
	/** @brief 値の取得関数。*/
	std::function<T()> getter;      // 値を読む
	/** @brief 値の設定関数。*/
	std::function<void(T)> setter;  // 値を書き込む
};

/**
 * @brief イージング制御コンポーネント。
 */
class EasingComponent : public Component
{
public:
	/** @brief 既定コンストラクタ。*/
	EasingComponent() = default;

	/**
	 * @brief イージングハンドラを開始します。
	 * @param handler 実行する `EasingHandler`。
	 * @param accessor 対象プロパティのゲッター/セッター。
	 */
	void StartHandler(const EasingHandler& handler, PropertyAccessor<float> accessor) {
		handlers.push_back(std::make_pair(accessor, handler));
	}

	/** @brief 全てのハンドラをクリアします。*/
	void Clear()
	{
		handlers.clear();
	}

	/** @brief 全てのハンドラが完了したかを返します。*/
	bool IsAllCompleted() const
	{
		return handlers.empty();
	}

	/**
	 * @brief 毎フレーム更新。
	 * @param deltaTime 経過時間（秒）。
	 */
	void Update(float deltaTime) override;

	/** @brief インスペクタ用のプロパティ描画。*/
	void DrawProperty() override;

protected:
	/** @brief デバッグ/GUI 用のイージング要素（内部管理）。*/
	std::vector<std::pair<int, EasingHandler::EaseItem>> easeItems;

	/** @brief 実行中ハンドラの集合（プロパティアクセサとペア）。*/
	std::vector<std::pair<PropertyAccessor<float>, EasingHandler>> handlers;

	/** @brief 非スケール時間を使用するか。*/
	bool useUnscaledTime = true;

public:
	/** @brief 非スケール時間を使用するかを返します。*/
	bool UsesUnscaledTime() const { return useUnscaledTime; }
	/** @brief テスト用プロパティ。*/
	std::string test = "guruhuji";
};