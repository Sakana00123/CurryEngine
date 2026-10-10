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

	/**
	 * @brief エフェクトデータを指定ファイルから読み込みます。
	 */
	C_FUNCTION()
	void Load(const std::string& filePath);

	/**
	 * @brief エフェクトデータを再読み込みします。
	 */
	C_FUNCTION()
	void ReloadAsset();

	/**
	 * @brief エフェクトを再生します。
	 */
	C_FUNCTION()
	void Play();

	/**
	 * @brief エフェクトを停止します。
	 */
	C_FUNCTION()
	void Stop();

	/**
	 * @brief 再生中かを返します。
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
	C_PROPERTY(CurryEngine::PropertyAttributes::CustomDrawer("String_AssetReference"), CurryEngine::PropertyAttributes::DialogFilter("Particle Effect Files(*.effect)|*.effect|All Files(*.*)|*.*|"), CurryEngine::PropertyAttributes::Setter("Load"))
	std::string filePath; // エフェクトファイルパス
	C_PROPERTY(CurryEngine::PropertyAttributes::CustomDrawer("AssetId"), CurryEngine::PropertyAttributes::AssetTypeExtension(".effect"), CurryEngine::PropertyAttributes::OnPropertyChanged("ReloadAsset"))
	CurryEngine::Resources::AssetId assetId; // エフェクトアセットID

	C_PROPERTY()
	bool playOnAwake = false;			// 自動再生フラグ
private:
	EffectHandle effectHandle = -1; 	// エフェクトハンドル
	std::vector<int> instanceIDs;		// 再生インスタンスIDリスト（複数再生に対応するためリストにする）
	bool isPlaying = false;				// 再生中フラグ
	AddSettings settings; 				// 追加設定
};
