#pragma once
#include "Engine/Effects/EffectManager.h"
#include "Engine/Effects/ComputeParticleSystem.h"
#include "Engine/Rendering/Pipeline/RenderContext.h"

#include "Engine/Core/Transform.h"
#include "Engine/Core/Color.h"

/** @brief EffectEditor を表すクラスです。 */
class EffectEditor
{
public:
	/**
	 * @brief EffectEditor を構築します。
	 */
	EffectEditor() = default;
	/**
	 * @brief EffectEditor を破棄します。
	 */
	~EffectEditor() = default;

	//初期化
	/**
	 * @brief 初期化します。
	 */
	static void Initialize();

	//エディタ表示
	/**
	 * @brief Show の処理を行います。
	 */
	static void Show();
	//エディタが開いているか
	/**
	 * @brief IsOpen の条件を満たすか判定します。
	 * @return 処理結果を返します。
	 */
	static bool IsOpen();
	//プレビューウィンドウがフォーカスされているか
	/**
	 * @brief IsPreviewFocused の条件を満たすか判定します。
	 * @return 処理結果を返します。
	 */
	static bool IsPreviewFocused();

	//エディタGUI描画
	/**
	 * @brief 描画処理を行います。
	 */
	static void DrawGUI(RenderContext* context);

private:
#ifdef USE_IMGUI
	// 範囲指定スライダー描画 ( int 用 )
	/**
	 * @brief 描画処理を行います。
	 */
	static bool DrawRangeInt(const char* label, CurryEngine::Range<int>& range, int speed = 1, int min = 0, int max = 0);
	// 範囲指定スライダー描画 ( unsigned int 用 )
	/**
	 * @brief 描画処理を行います。
	 */
	static bool DrawRangeUInt(const char* label, CurryEngine::Range<unsigned int>& range, unsigned int speed = 1, unsigned int min = 0, unsigned int max = 0);
	// 範囲指定スライダー描画 ( float 用 )
	/**
	 * @brief 描画処理を行います。
	 */
	static bool DrawRangeFloat(const char* label, CurryEngine::Range<float>& range, float speed = 0.1f, float min = 0.0f, float max = 0.0f);
	// 範囲指定スライダー描画 ( Vector2 用 )
	/**
	 * @brief 描画処理を行います。
	 */
	static bool DrawRangeVector2(const char* label, CurryEngine::Range<Vector2>& range, float speed = 0.1f, float min = 0.0f, float max = 0.0f);
	// 範囲指定スライダー描画 ( Vector3 用 )
	/**
	 * @brief 描画処理を行います。
	 */
	static bool DrawRangeVector3(const char* label, CurryEngine::Range<Vector3>& range, float speed = 0.1f, float min = 0.0f, float max = 0.0f);
	// 範囲指定スライダー描画 ( Color 用 )
	/**
	 * @brief 描画処理を行います。
	 */
	static bool DrawRangeColor(const char* label, CurryEngine::Range<Color>& range);

	// スライダー描画 ( int 用 )
	/**
	 * @brief 描画処理を行います。
	 */
	static bool DrawInt(const char* label, int& value, int speed = 1, int min = 0, int max = 0);
	// スライダー描画 ( unsigned int 用 )
	/**
	 * @brief 描画処理を行います。
	 */
	static bool DrawUInt(const char* label, unsigned int& value, unsigned int speed = 1, unsigned int min = 0, unsigned int max = 0);
	// スライダー描画 ( float 用 )
	/**
	 * @brief 描画処理を行います。
	 */
	static bool DrawFloat(const char* label, float& value, float speed = 0.1f, float min = 0.0f, float max = 0.0f);
	// スライダー描画 ( Vector2 用 )
	/**
	 * @brief 描画処理を行います。
	 */
	static bool DrawVector2(const char* label, Vector2& value, float speed = 0.1f, float min = 0.0f, float max = 0.0f);
	// ベクトルスライダー描画 ( Vector3 用 )
	/**
	 * @brief 描画処理を行います。
	 */
	static bool DrawVector3(const char* label, Vector3& value, float speed = 0.1f, float min = 0.0f, float max = 0.0f);
	// カラーピッカー描画 ( Color 用 )
	/**
	 * @brief 描画処理を行います。
	 */
	static bool DrawColor(const char* label, Color& value);
	// チェックボックス描画
	/**
	 * @brief 描画処理を行います。
	 */
	static bool DrawCheckbox(const char* label, bool& value);
	// コンボボックス描画
	/**
	 * @brief 描画処理を行います。
	 */
	static bool DrawCombo(const char* label, int& currentItem, const char* const items[], int itemCount, std::function<void(int)> setter);

	// グラデーションエディタ描画
	/**
	 * @brief 描画処理を行います。
	 */
	static bool DrawGradient(uint32_t gradientId, ImGradientHDRState* state, ImGradientHDRTemporaryState* tempState);
	
#endif // USE_IMGUI

private:
	//エディタが開いているか
	static inline bool isOpen = false;
	static inline bool isPreviewFocused = false; // プレビューウィンドウがフォーカスされているか
	static inline EffectHandle currentEffectHandle = -1; // 現在編集中のエフェクトハンドル
};