#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Effects/EffectManager.h"

/** @brief ParticleComponent を表すクラスです。 */
class ParticleComponent : public Component
{
	C_REFLECT(ParticleComponent)
public:
	/**
	 * @brief ParticleComponent を構築します。
	 */
	ParticleComponent() = default;
	/**
	 * @brief ParticleComponent を破棄します。
	 */
	~ParticleComponent() override = default;
public:
	/** @brief LineData を表す構造体です。 */
	struct LineData
	{
		bool useLine = false;	// 線を使うかどうか

		// 線分構造体
		/** @brief Segment を表す構造体です。 */
		struct Segment
		{
			Transform* start = nullptr; // 線の開始Transform
			Transform* end = nullptr;   // 線の終了Transform
			int segmentCount = 5;    // 線分の分割数
		};
		std::vector<Segment> segments; 	// 線分リスト
	};

	// 追加設定構造体
	/** @brief AddSettings を表す構造体です。 */
	struct AddSettings
	{
		LineData lineData;					//線情報
		std::function<void()> onPreEmit;		//エフェクト発生前コールバック
	};
	// 追加設定取得
	const AddSettings& GetAddSettings() const { return settings; }

	// 追加設定設定
	void SetAddSettings(const AddSettings& settings) { this->settings = settings; }

	// 初期化
	/**
	 * @brief Awake の処理を行います。
	 */
	void Awake() override;

	// 終了処理
	/**
	 * @brief Destroy イベントを処理します。
	 */
	void OnDestroy() override;

	// エフェクトデータ読み込み
	/**
	 * @brief Load に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	C_FUNCTION()
	void Load(const std::string& filePath);

	// エフェクト再生
	/**
	 * @brief Play の処理を行います。
	 */
	C_FUNCTION()
	void Play();

	// エフェクト停止
	/**
	 * @brief Stop の処理を行います。
	 */
	C_FUNCTION()
	void Stop();

	// 再生中かを返す
	/**
	 * @brief IsPlaying の条件を満たすか判定します。
	 * @return 処理結果を返します。
	 */
	C_FUNCTION()
	bool IsPlaying() const;

	// エフェクトハンドル取得
	EffectHandle GetEffectHandle() const { return effectHandle; }

	// エフェクトデータ取得
	EffectManager::EffectData& GetEffectData() const { return EffectManager::GetEffectData(effectHandle); }

	// エフェクトデータ設定
	/**
	 * @brief EffectData を設定します。
	 */
	void SetEffectData(const EffectManager::EffectData& data);

	// フレーム更新
	/**
	 * @brief 状態を更新します。
	 */
	void Update(float elapsedTime) override;

#ifdef USE_IMGUI
	// デバッグGUI描画
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI


	// シリアライズ
	/**
	 * @brief Serialize の処理を行います。
	 */
	json Serialize() const override;

	// デシリアライズ
	/**
	 * @brief Deserialize の処理を行います。
	 */
	void Deserialize(const json& j) override;

private:
	C_PROPERTY(CurryEngine::PropertyAttributes::CustomDrawer("String_AssetReference"), CurryEngine::PropertyAttributes::DialogFilter("Particle Effect Files|*.json|All Files|*.*|"), CurryEngine::PropertyAttributes::Setter("Load"))
	std::string filePath; // エフェクトファイルパス
	C_PROPERTY()
	bool playOnAwake = false;			// 自動再生フラグ
private:
	EffectHandle effectHandle = -1; 	// エフェクトハンドル
	std::vector<int> instanceIDs;		// 再生インスタンスIDリスト（複数再生に対応するためリストにする）
	bool isPlaying = false;				// 再生中フラグ
	AddSettings settings; 				// 追加設定
};
