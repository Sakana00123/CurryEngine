#pragma once
#include <map>
#include "Engine/Core/Component.h"
#include <xaudio2.h>
#include <mmreg.h>

#include "Engine/Core/Misc.h"
#include <wrl.h>
#include <cstdint>

/**
 * @file
 * @brief オーディオ（BGM/SE）の再生・管理を行うユーティリティ。
 * @details XAudio2 を用いた初期化、マスター/サブミックスの音量制御、ワンショット再生、
 *          オーディオソースの寿命管理などを提供します。
 */

/**
 * @brief サウンド種別。
 */
enum SoundType
{
	BGM,       //!< 楽曲（BGM）
	SE,        //!< 効果音（SE）
	EnumCount  //!< 列挙数
};
class StandaloneAudioSource;

/**
 * @brief オーディオ再生の中核クラス（静的ユーティリティ）。
 */
class Audio
{
public:
	/**
	 * @brief 読み込み済みの音声データを保持するバッファ。
	 * @details WaveFormat と XAudio2 のバッファをまとめて管理します。
	 */
	class AudioBuffer
	{
	public:
		AudioBuffer() = default;
		~AudioBuffer() {
			delete[] buffer.pAudioData;
		}

		WAVEFORMATEXTENSIBLE wfx = { 0 }; //!< フォーマット情報
		XAUDIO2_BUFFER buffer = { 0 };    //!< XAudio2 再生用バッファ

		/**
		 * @brief オーディオの総再生時間を取得します。
		 * @return 再生時間（秒）。
		 */
		float GetDuration() const;
	public:
		/**
		 * @brief オーディオリソースを取得（共有）。
		 * @param filePath 読み込む音声ファイルパス。
		 * @return 読み込み済み/新規読み込みされた `AudioBuffer` の共有ポインタ。
		 */
		static std::shared_ptr<AudioBuffer> GetResource(const std::wstring& filePath);
	private:
		friend class Audio;
		/**
		 * @brief 参照済みリソースの弱参照キャッシュ。
		 * @note キーが `const wchar_t*` のため、同一パスでもポインタ値が異なると別キーとなります。
		 */
		static inline std::map<std::wstring, std::weak_ptr<AudioBuffer>> resources;
	};

	/** @brief 初期化処理（XAudio2 のセットアップ）。*/
	static void Initialize();

	/** @brief マスターボリューム設定。*/
	static void SetMasterVolume(float volume) { masterVoice->SetVolume(volume); }

	/** @brief マスターボリューム取得。*/
	static void GetMasterVolume(float& volume) { masterVoice->GetVolume(&volume); }

	/** @brief BGM 全体のボリューム設定。*/
	static void SetBgmVolume(float volume) { submixVoices[BGM]->SetVolume(volume); }

	/** @brief BGM 全体のボリューム取得。*/
	static void GetBgmVolume(float& volume) { submixVoices[BGM]->GetVolume(&volume); }

	/** @brief SE 全体のボリューム設定。*/
	static void SetSeVolume(float volume) { submixVoices[SE]->SetVolume(volume); }

	/** @brief SE 全体のボリューム取得。*/
	static void GetSeVolume(float& volume) { submixVoices[SE]->GetVolume(&volume); }

	/** @brief デストラクタ。リソースを解放します。*/
	virtual ~Audio();

public:

	/**
	 * @brief 単発の効果音（または BGM）を再生します。
	 * @param filePath 音声ファイルパス。
	 * @param volume 再生音量（0-1 目安）。
	 */
	static void PlayOneShot(const wchar_t* filePath, float volume = 1.0f);

	/**
	 * @brief フレーム更新。
	 * @param deltaTime 経過時間（秒）。
	 * @details 再生完了した一時ソースの破棄などを行います。
	 */
	static void Update(float deltaTime);
	/**
	 * @brief すべての一時ソースを破棄します。
	 */
	static void ClearAll() {
		audioSources.clear();
		erases.clear();
	}
private:
	/** @brief アクティブな一時ソース群。*/
	static inline std::vector<std::shared_ptr<StandaloneAudioSource>> audioSources;
	/** @brief 破棄予定の一時ソース群。*/
	static inline std::vector<std::shared_ptr<StandaloneAudioSource>> erases;

private:
	friend class AudioSourceComponent;
	friend class AudioSource;
	friend class StandaloneAudioSource;
	/**
	 * @brief XAudio2 のソースボイスを作成します。
	 * @param buffer 再生元のオーディオバッファ。
	 * @param sourceVoice 生成結果の出力先。
	 * @param type サウンド種別（BGM/SE）。
	 */
	static void CreateAudioSource(std::shared_ptr<AudioBuffer> buffer, IXAudio2SourceVoice** sourceVoice, SoundType type);

private:
//#ifdef X3DAUDIO
//	/** @brief X3DAudio のハンドル。*/
//	static inline BYTE x3dAudioHandle;
//#endif // X3DAUDIO
private:
	friend class AudioListener;
	friend class C3DAudio;
	/** @brief XAudio2 インスタンス。*/
	static inline Microsoft::WRL::ComPtr<IXAudio2> xaudio2;
	/** @brief マスターボイス。*/
	static inline IXAudio2MasteringVoice* masterVoice = nullptr;
	/** @brief サブミックスボイス（BGM/SE）。*/
	static inline IXAudio2SubmixVoice* submixVoices[SoundType::EnumCount];
};

/**
 * @brief 単体で動作する簡易オーディオソース。
 * @details ファイルを指定して再生・停止・音量操作を行います。
 */
class StandaloneAudioSource
{
public:
	/**
	 * @brief コンストラクタ。
	 * @param filePath 音声ファイルパス。
	 */
	StandaloneAudioSource(const wchar_t* filePath);
	/** @brief デストラクタ。*/
	~StandaloneAudioSource();

	/** @brief 再生を開始します。*/
	void Play(bool loop = false);
	/** @brief 再生を停止します。*/
	void Stop(bool playTails = true);
	/** @brief 音量を設定します。*/
	void SetVolume(float volume);
	/** @brief 現在の音量を取得します。*/
	float GetVolume();

	/** @brief 再生中かを返します。*/
	bool IsPlaying();

	/** @brief デバッグ GUI 描画。*/
	void DrawGUI();

private:
	/** @brief サウンド種別。*/
	SoundType type;

	/** @brief 再生対象のオーディオバッファ。*/
	std::shared_ptr<Audio::AudioBuffer> sptrBuffer;
	/** @brief XAudio2 のソースボイス。*/
	IXAudio2SourceVoice* sourceVoice;

	/** @brief マスター音量。*/
	static inline float masterVolume = 1.0f;
	/** @brief BGM 音量。*/
	static inline float bgmVolume = 1.0f;
	/** @brief SE 音量。*/
	static inline float seVolume = 1.0f;
};