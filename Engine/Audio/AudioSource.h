#pragma once
#include "Audio.h"
#include "Engine/Core/Component.h"
#include "AudioAnalyzer.h"

/**
 * @file
 * @brief オーディオソース（コンポーネント）。
 * @details XAudio2 を用いて音声の再生/停止/音量/ループ設定などを行います。
 *          ファイルパスから `Audio::AudioBuffer` を取得して再生します。
 */

/**
 * @brief シーン上に配置して音声を再生するコンポーネント。
 */
class AudioSource : public Component
{
	C_REFLECT(AudioSource)
public:
	/** @brief 既定コンストラクタ。*/
	AudioSource() = default;
	/** @brief デストラクタ。*/
	virtual ~AudioSource() override;

	/**
	 * @brief 開始処理。
	 */
	void Start() override;

	/**
	 * @brief フレーム更新。
	 * @param deltaTime 経過時間（秒）。
	 */
	void Update(float deltaTime) override;

	/**
	 * @brief ソース（音声ファイル）を設定します。
	 * @param filePath ファイルパス。
	 */
	void SetSource(const std::wstring& filePath);

	/**
	 * @brief 再生を開始します。
	 */
	C_FUNCTION()
	void Play();
	/**
	 * @brief 再生を停止します。
	 * @param playTails テイル（残響等）を再生してから停止するか。
	 */
	C_FUNCTION()
	void Stop(bool playTails = true);

	/**
	 * @brief 再生を一時停止します。
	 */
	C_FUNCTION()
	void Pause();

	/**
	 * @brief 再生を再開します。
	 */
	C_FUNCTION()
	void Resume();

	/**
	 * @brief 音量を設定します。
	 * @param volume 音量（0-1 目安）。
	 */
	C_FUNCTION()
	void SetVolume(float volume);

	/**
	 * @brief 現在の音量を取得します。
	 * @return 音量（0-1 目安）。
	 */
	C_FUNCTION()
	float GetVolume();

	/**
	 * @brief 再生中かを返します。
	 * @return `true` で再生中、`false` で停止中。
	 */
	C_FUNCTION()
	bool IsPlaying();

	/**
	 * @brief 3D音源として扱うかを設定します。
	 * @param use3D `true` で 3D音源、`false` で通常の2D音源。
	 * @details 3D音源として扱う場合は `AudioListener` コンポーネントが必要です。
	 */
	void SetUse3DAudio(bool use3D);

	/**
	 * @brief 3D音源として扱うかを取得します。
	 * @return `true` で 3D音源、`false` で通常の2D音源。
	 */
	bool GetUse3DAudio() const;

	/**
	 * @brief パンを設定します。
	 * @param pan パン（-1.0 左、0 中央、1.0 右）。
	 */
	C_FUNCTION()
	void SetPan(float pan);

	/**
	 * @brief 現在のパンを取得します。
	 * @return パン（-1.0 左、0 中央、1.0 右）。
	 */
	C_FUNCTION()
	float GetPan() const { return m_Pan; }

	/**
	 * @brief ループ再生の有無を設定します。
	 * @param loop `true` でループ再生、`false` で一回再生。
	 */
	C_FUNCTION()
	void SetLoop(bool loop) { this->loop = loop; }

	/**
	 * @brief ループ再生の有無を取得します。
	 * @return `true` でループ再生、`false` で一回再生。
	 */
	C_FUNCTION()
	bool IsLoop() const { return loop; }

	/**
	 * @brief ループ再生の範囲を設定します。
	 * @param begin ループ開始位置（秒）。
	 * @param length ループ長（begin からの長さ）。
	 */
	C_FUNCTION()
	void SetLoopOption(float begin, float length);

	/**
	 * @brief 再生時間を取得します。
	 * @return 再生時間（秒）。
	 */
	float GetPlaybackTime() const;

	/**
	 * @brief 再生のデルタ時間を取得します。
	 * @return 再生デルタ時間（秒）。
	 */
	float GetPlaybackDeltaTime();

	/**
	 * @brief キューに残っているバッファ数を取得します。
	 * @return バッファキュー数（再生中なら 0 より大きい）。
	 */
	uint32_t GetBufferQueueCount();

	/**
	 * @brief オーディオの総再生時間を取得します。
	 * @return 再生時間（秒）。
	 */
	float GetTotalDuration() const;

	/**
	 * @brief オーディオアナライザの解析結果を取得します。
	 * @return 解析結果。
	 */
	[[nodiscard]] const AudioAnalyzerResult& GetAudioAnalyzerResult() const;
	
	/**
	 * @brief オーディオアナライザの基本解析結果を取得します。
	 * @return 基本解析結果。
	 */
	[[nodiscard]] const AudioAnalyzer::Result& GetAudioAnalyzerBasicResult() const;

#ifdef USE_IMGUI
	/**
	 * @brief インスペクタ用のプロパティ表示。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

	/**
	 * @brief コンポーネントのシリアライズ。
	 * @return シリアライズ結果の JSON オブジェクト。
	 */
	json Serialize() const override;

	/**
	 * @brief コンポーネントのデシリアライズ。
	 * @param j デシリアライズ元の JSON オブジェクト。
	 */
	void Deserialize(const json& j) override;

private:
	/** @brief 設定中のファイルパス。*/
	std::wstring filePath;
	/** @brief サウンド種別（BGM/SE）。*/
	SoundType type;
	/** @brief 自動再生設定。*/
	bool playOnStart = false;
	/** @brief ループ設定。*/
	bool loop = false;
	/** @brief 3D 音源として扱うか。*/
	bool use3DAudio = false;
	/** @brief 音量（0-1 目安）。*/
	float volume = 1.0f;
	/** @brief パン（-1.0 左、0 中央、1.0 右）。*/
	float m_Pan = 0.0f;
	
	/** @brief 再生対象のオーディオバッファ。*/
	std::shared_ptr<Audio::AudioBuffer> m_SptrBuffer;
	friend class C3DAudio;
	/** @brief XAudio2 のソースボイス。*/
	IXAudio2SourceVoice* sourceVoice;

	/** @brief オーディオアナライザ。*/
	std::unique_ptr<AudioAnalyzer> m_AudioAnalyzer;

	/** @brief マスター音量。*/
	static inline float masterVolume = 1.0f;
	/** @brief BGM 音量。*/
	static inline float bgmVolume = 1.0f;
	/** @brief SE 音量。*/
	static inline float seVolume = 1.0f;

private:
	float m_LastSamplesPlayed = 0.0f;
};