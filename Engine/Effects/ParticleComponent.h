#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Effects/EffectManager.h"

class ParticleComponent : public Component
{
	C_REFLECT(ParticleComponent)
public:
	ParticleComponent() = default;
	~ParticleComponent() override = default;
public:
	struct LineData
	{
		bool useLine = false;	// 線を使うかどうか

		// 線分構造体
		struct Segment
		{
			Transform* start = nullptr; // 線の開始Transform
			Transform* end = nullptr;   // 線の終了Transform
			int segmentCount = 5;    // 線分の分割数
		};
		std::vector<Segment> segments; 	// 線分リスト
	};

	// 追加設定構造体
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
	void Awake() override;

	// 終了処理
	void OnDestroy() override;

	// エフェクトデータ読み込み
	C_FUNCTION()
	void Load(const std::string& filePath);

	// エフェクト再生
	C_FUNCTION()
	void Play();

	// エフェクト停止
	C_FUNCTION()
	void Stop();

	// 再生中かを返す
	C_FUNCTION()
	bool IsPlaying() const;

	// エフェクトハンドル取得
	EffectHandle GetEffectHandle() const { return effectHandle; }

	// エフェクトデータ取得
	EffectManager::EffectData& GetEffectData() const { return EffectManager::GetEffectData(effectHandle); }

	// エフェクトデータ設定
	void SetEffectData(const EffectManager::EffectData& data);

	// フレーム更新
	void Update(float elapsedTime) override;

#ifdef USE_IMGUI
	// デバッグGUI描画
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI


	// シリアライズ
	json Serialize() const override;

	// デシリアライズ
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
