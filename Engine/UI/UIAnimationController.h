#pragma once
#include "UIComponent.h"
#include "Engine/Easing/EasingHandler.h"
#include "Selectable.h"

/**
 * @file
 * @brief UI の状態遷移（Idle/Hovered/Selected/Pressed/Disabled/Custom）をアニメーション制御するコンポーネント。
 * @details 各状態ごとのトランジション設定（位置/サイズの補間、時間、Ease）を保持し、
 *          状態変更時に `EasingHandler` で補間を行います。
 */
class UIAnimationController : public UIComponent
{
public:
	/**
	 * @brief UI の状態。
	 */
	enum class State { Idle, Hovered, Selected, Pressed, Disabled, Custom, StateCount };

	/**
	 * @brief 状態遷移のパラメータ。
	 * @details 位置とサイズを個別に有効化して補間できます。
	 */
	struct Transition {
		bool enableTranslate = false; //!< 位置補間の有効/無効
		XMFLOAT2 position;            //!< 遷移先のローカル位置
		bool enableSizing = false;   //!< サイズ補間の有効/無効
		XMFLOAT2 size;               //!< 遷移先のサイズ
		float duration = 0.2f;       //!< 補間時間（秒）
		EaseType type;               //!< イージングタイプ
	};

	/**
	 * @brief 指定状態のトランジションを設定します。
	 * @param state 対象状態。
	 * @param transition 遷移パラメータ。
	 */
	void SetTransition(State state, const Transition& transition);

	/**
	 * @brief 毎フレーム更新。
	 * @param elapsedTime 経過時間（秒）。
	 * @details 現在の補間を進め、位置/サイズを反映します。
	 */
	void Update(float elapsedTime) override;

	/**
	 * @brief 状態を変更します。
	 * @param newState 新しい状態。
	 * @details 現在値を起点として、対応するトランジションで補間を開始します。
	 */
	void SetState(State newState);

private:
	/** @brief イージング進行を管理するハンドラ。*/
	EasingHandler handler;
	/** @brief 現在状態。*/
	State currentState;
	/** @brief 遷移先状態。*/
	State targetState;

	/** @brief 各状態のトランジション設定配列。*/
	Transition transitions[static_cast<size_t>(State::StateCount)];
	
	/** @brief 現在の状態遷移の進捗（0～1）。*/
	float transitionProgress;

	/** @brief 位置補間の開始/終了値。*/
	XMFLOAT2 fromPosition;
	XMFLOAT2 toPosition;

	/** @brief サイズ補間の開始/終了値。*/
	XMFLOAT2 fromSize;
	XMFLOAT2 toSize;
};