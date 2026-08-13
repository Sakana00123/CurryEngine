#pragma once
#include "Engine/Effects/ComputeParticleSystem.h"
#include "Engine/Rendering/Pipeline/RenderContext.h"

#include "Engine/Core/Transform.h"
#include "Engine/Core/Color.h"
#include "Engine/Types/Range.h"

// エフェクトハンドル
typedef int EffectHandle;

class EffectManager
{
public:
	EffectManager() = default;
	~EffectManager() = default;
public:

	// エフェクトデータクリア
	static void ClearAll();

	// 新しいエフェクトデータ追加用のハンドル取得
	static EffectHandle CreateEffectData();

	// エフェクトデータ読み込み
	static EffectHandle LoadEffectData(const std::string& filePath);

	// エフェクトデータ読み込み（ダイアログ表示）
	static EffectHandle LoadEffectDataWithDialog();

	// エフェクトデータ保存
	static void SaveEffectData(EffectHandle handle, const std::string& filePath);

	// エフェクトデータ保存（ダイアログ表示）
	static void SaveEffectDataWithDialog(EffectHandle handle);

	// エフェクト再生 (return: 再生インスタンスID)
	static int Play(EffectHandle handle, const Vector3& position = {}, const Vector3& rotationEulerDegree = {});

	// エフェクト停止
	static void Stop(EffectHandle handle);

	// エフェクト停止（インスタンスID指定）
	static void StopImmediate(int instanceID);

	// エフェクト再生中か
	static bool IsPlaying(EffectHandle handle);

	// 全エフェクト停止
	static void StopAll();

	// エフェクトデータコピー
	static EffectHandle CopyEffectData(EffectHandle srcHandle);

	// エフェクトデータ取得
	struct EffectData;
	static EffectData& GetEffectData(EffectHandle handle);

public:

	//初期化
	static void Initialize();

	//更新
	static void Update(float deltaTime);

	//描画
	static void Render(RenderContext* rtx);

	//エディタGUI描画
	//static void DrawGUI();

private:

	static void ClearEffectData(); // エフェクトデータクリア

	static void ReInitializeParticleSystem(); // パーティクルシステム再初期化

	// エミット処理
	struct EmitterPlayState;
	static void EmitOnce(const EmitterPlayState& state);

	// 形状エミッタ設定適用
	struct EmitterShapeData;
	static void ApplyShapeEmitterSettings(const EmitterShapeData& settings, ComputeParticleSystem::EmitParticleData& emitData, int index, int emitCount);

	
	// ランダム値取得
	static float Random(float min, float max);

	// ランダムなボックス内位置取得
	static Vector3 RandomBoxPosition(const Vector3& size);

	// ランダム方向ベクトル取得
	static Vector3 RandomDirection();

	// ランダム上半球方向ベクトル取得
	static Vector3 RandomHemisphereDirection(const Vector3& normal);

	// 指定角度内のランダム方向ベクトル取得
	static Vector3 RandomConeDirection(const Vector3& dir, float coneAngle);
public:
	// 描画モード
	enum class RenderingMode : uint8_t
	{
		Billboard = 0,		// ビルボード
		StretchedBillboard,	// ストレッチドビルボード
		FixedRotation,		// 固定回転
		ScreenSpace,		// スクリーンスペース
	};
	// 形状定義
	enum class ShapeType : uint8_t
	{
		Point = 0,			// 点
		Ring,				// リング
		Sphere,				// 球
		Cylinder,			// 円柱
	};
	// 方向生成モード
	enum class DirectionMode : uint8_t
	{
		Default = 0,   // EmitterMotionData::velocity に従う
		Axis,          // 指定軸方向
		Random,        // ランダム方向
		Outward,       // 中心から外へ
		Inward,        // 中心に向かう
		Normal,        // 形状法線方向
	};
	// エミット設定構造体
	struct EmitterEmitData
	{
		int maxParticles{ 1000 };						// 最大パーティクル数
		CurryEngine::Range<int> emitCount{ 10,10 };				// エミット数
		CurryEngine::Range<float> initialDelay{ 0,0 };				// 初期遅延時間
		CurryEngine::Range<float> emitInterval{ 0,0 };				// エミット間隔
		Vector3 positionOffset;							// 生成位置
		CurryEngine::Range<Vector3> rotationEuler;					// 回転
		CurryEngine::Range<Vector3> endRotationEuler;				// 終了回転
		CurryEngine::Range<float> rotationEasingTime{ 0.0f,0.0f };	// 回転イージング時間
		int rotationEasingType{ 0 };					// 回転イージングタイプ（ComputeParticleUpdateCS.hlslのEase関数参照）
		bool loop{ false };								// ループフラグ
		float duration{ 1.0f };							// エミット持続時間（ループする場合は1サイクルの時間）TODO: durationはループする場合の1サイクルの時間にするか、ループフラグと分けてエミット持続時間を別途設けるか要検討
	};
	// 形状エミッタ設定構造体
	struct EmitterShapeData
	{
		ShapeType shape = ShapeType::Point;						// 形状タイプ
		DirectionMode directionMode = DirectionMode::Default;	// 方向生成モード
		Vector3 directionAxis{ 0,1,0 };							// 方向軸（DirectionMode::Axisで使用）
		CurryEngine::Range<float> speed = { 1.0f,1.0f };					// 速度（DirectionModeで使用）
		CurryEngine::Range<float> endSpeed = { 1.0f,1.0f };				// 終了速度（DirectionModeで使用）
		CurryEngine::Range<float> speedEasingTime{ 0.0f, 0.0f };			// 速度イージング時間
		int speedEasingType{ 0 };								// 速度イージングタイプ（ComputeParticleUpdateCS.hlslのEase関数参照）
		float radius = 1.0f;									// 円/球で使用
		float height = 1.0f;									// Cylinderで使用
	};
	// 動作設定構造体
	struct EmitterMotionData
	{
		CurryEngine::Range<Vector3> velocity;					// 初速
		CurryEngine::Range<Vector3> acceleration;				// 加速度
		CurryEngine::Range<float> lifeTime{ 1.0f, 1.0f };		// 生存時間
		bool useGravity{ false };					// 重力使用フラグ
	};
	// ビジュアル設定構造体
	struct EmitterVisualData
	{
		RenderingMode renderingMode = RenderingMode::Billboard; // 描画モード
		std::string texturePath;								// テクスチャパス
		DirectX::XMUINT2 textureSplitCount{ 1, 1 };				// テクスチャ分割数
		BlendState blendState = BlendState::Transparency;		// ブレンドステート
		CurryEngine::Range<Vector2> startSize{ { 1,1 }, { 1,1 } };			// 開始サイズ
		CurryEngine::Range<Vector2> endSize{ { 1,1 }, { 1,1 } };			// 終了サイズ
		CurryEngine::Range<float> sizeEasingTime{ 0.0f,0.0f };				// サイズイージング時間
		int sizeEasingType{ 0 };								// サイズイージングタイプ（ComputeParticleUpdateCS.hlslのEase関数参照）
		bool useGradient{ false };								// グラデーション使用フラグ
		CurryEngine::Range<Color> startColor;								// 開始色
		CurryEngine::Range<Color> endColor;								// 終了色
		bool enableFadeIn{ false };								// フェードイン有効フラグ
		bool enableFadeOut{ false };							// フェードアウト有効フラグ
		CurryEngine::Range<float> fadeInTime{ 0.0f, 0.0f };				// フェードイン時間
		CurryEngine::Range<float> fadeOutTime{ 0.0f, 0.0f };				// フェードアウト時間

		ImGradientHDRState gradientState{};						// グラデーション状態
		ImGradientHDRTemporaryState gradientTempState{};		// グラデーション一時状態(エディタ用、保存しない)

		EmitterVisualData()
		{
			// デフォルトのグラデーション設定
			gradientState.AddColorMarker(0.0f, { 1.0f,1.0f,1.0f }, 1.0f);
			gradientState.AddColorMarker(1.0f, { 1.0f,1.0f,1.0f }, 1.0f);
			gradientState.AddAlphaMarker(0.0f, 1.0f);
			gradientState.AddAlphaMarker(1.0f, 1.0f);
		}
	};
	// エミッタデータ構造体
	struct ParticleEmitterData
	{
		std::string name;				// エミッタ名
		bool isEnabled{ true };			// 有効フラグ
		
		EmitterEmitData emitData;		// エミット設定
		EmitterShapeData shapeData;		// 形状エミッタ設定
		EmitterMotionData motionData;	// 動作設定
		EmitterVisualData visualData;	// ビジュアル設定
	};
	// エフェクトデータ構造体
	struct EffectData
	{
		std::string name; // エフェクト名
		std::vector<ParticleEmitterData> emitters; // エミッタデータリスト
	private:
		friend class EffectManager;
		EffectHandle handle = -1; // エフェクトハンドル
		std::string filePath; // エフェクトデータファイルパス
	};
	static inline std::unordered_map<EffectHandle, EffectData> effectData; // エフェクトデータリスト

private:
	friend class EffectEditor;
	//エディタが開いているか
	static inline bool isOpen = false;

	// エフェクト再生管理用のパーティクルシステムリスト
	using ParticleSystems = std::unordered_map<int/*emitterIndex*/, std::unique_ptr<ComputeParticleSystem>>;
	static inline std::unordered_map<EffectHandle,
		std::unordered_map<int/*playInstanceId*/, ParticleSystems>> particleSystems;

	// playInstanceIdカウンタ追加
	static inline int nextPlayInstanceId = 0;

private:

	struct EmitterPlayState
	{
		EffectHandle handle;			// エフェクトハンドル
		int emitterIndex;				// エミッタインデックス
		int playInstanceId;				// 再生インスタンスID（同一エミッタの複数再生を区別するため）
		ParticleEmitterData emitterData;	// エミッタデータ
		float elapsedTime;				// 経過時間
		float nextEmitTime;				// 次のエミット時間
		bool isPlaying;					// 再生中フラグ

		Vector3 position;				// エフェクト位置
		Vector3 rotationEuler;			// エフェクト回転（オイラー角）
	};

	static inline std::vector<EmitterPlayState> playingEmitters; // 再生中エミッタリスト
};
