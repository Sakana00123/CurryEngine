#pragma once

/**
 * @file
 * @brief 入力システムの公開インターフェース。
 * @details キーボード・マウス・ゲームパッドの入力状態取得、更新、
 *          マウス座標やホイール量、アクティブデバイス切り替えなどを提供します。
 */

#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

#include <map>
#include <vector>
#include <memory>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <DirectXMath.h>
#include "Engine/Core/Math/Vector2.h"

#define DIRECTION_KEY_NUM 4
#define DIRECTION_SET_MAX 2

/**
 * @brief 入力デバイスの種別。
 */
enum class InputDevice { Keybord, Mouse, GamePad };

/**
 * @brief 単一の入力（キー/マウスボタン/ゲームパッドボタン）を表す基底クラス。
 * @details 押下時間の蓄積、トリガー/リリース判定を提供します。
 */
class InputKey
{
protected:
	int vKey;                 //!< バーチャルキーコード（またはボタン/入力の識別子）
	float pressTime;          //!< 現在フレームまでの押下時間（秒）。0なら未押下
	float oldPressTime;       //!< 前フレームまでの押下時間（秒）
	InputDevice deviceType;   //!< この入力のデバイス種別
public:
	/**
	 * @brief コンストラクタ。
	 * @param vKey バーチャルキーコード
	 * @param deviceType デバイス種別
	 */
	InputKey(int vKey, InputDevice deviceType) : vKey(vKey), pressTime(0), oldPressTime(0), deviceType(deviceType) {}
	virtual ~InputKey() = default;

	/**
	 * @brief 入力状態を更新します。
	 * @param elapsedTime 経過時間（秒）
	 */
	virtual void Update(float elapsedTime);

	/**
	 * @brief バーチャルキーコードを取得します。
	 * @return バーチャルキーコード
	 */
	int GetVKey() const { return vKey; }

	/**
	 * @brief 押されているか。
	 * @return 押下中なら true
	 */
	bool IsPressed() const { return pressTime > 0; }

	/**
	 * @brief このフレームで新たに押されたか。
	 * @return 今フレームで押下開始なら true
	 */
	bool IsTrigger() const { return (oldPressTime == 0 && pressTime > 0); }

	/**
	 * @brief このフレームで離されたか。
	 * @return 今フレームでリリースなら true
	 */
	bool IsRelease() const { return (pressTime == 0 && oldPressTime > 0); }

	/**
	 * @brief 入力のデバイス種別を取得します。
	 * @return デバイス種別
	 */
	InputDevice GetDeviceType() const { return deviceType; }
};

/**
 * @brief キーボード入力を表すクラス。
 */
class Keybord : public InputKey
{
public:
	/**
	 * @brief コンストラクタ。
	 * @param vKey バーチャルキーコード（VK_XXX）
	 */
	Keybord(int vKey) : InputKey(vKey, InputDevice::Keybord) {}
	~Keybord() override = default;
	Keybord(Keybord&) = delete;
	Keybord& operator=(Keybord&) = delete;
};

/**
 * @brief マウス入力（ボタン）を表すクラス。
 */
class Mouse : public InputKey
{
public:
	/**
	 * @brief コンストラクタ。
	 * @param vKey ボタンの仮想キー（VK_LBUTTON/VK_RBUTTON など）
	 */
	Mouse(int vKey) : InputKey(vKey, InputDevice::Mouse) {}
	~Mouse() override = default;
	Mouse(Mouse&) = delete;
	Mouse& operator=(Mouse&) = delete;
};

/**
 * @brief ゲームパッドの入力タイプ。
 */
enum class KeyType { Key, LeftTrigger, RightTrigger };

/**
 * @brief スティックの左右。
 */
enum class Side { Left, Right };

/**
 * @brief 軸の種類。
 */
enum class Axis { X, Y };

/**
 * @brief ゲームパッド入力を表すクラス。
 * @details ボタンに加え、トリガー（アナログ）の押下判定にも対応します。
 */
class GamePad : public InputKey
{
	KeyType keyType; //!< ボタンかトリガーかの種別
public:
	/**
	 * @brief コンストラクタ。
	 * @param vKey バーチャルキー
	 * @param type 判定する入力の種類（ボタン/トリガー）
	 */
	GamePad(int vKey, KeyType type = KeyType::Key) : InputKey(vKey, InputDevice::GamePad), keyType(type) {}
	~GamePad() override = default;
	GamePad(GamePad&) = delete;
	GamePad& operator=(GamePad&) = delete;

	/**
	 * @brief 入力状態を更新します。
	 * @param elapsedTime 経過時間（秒）
	 */
	void Update(float elapsedTime) override;
};

/**
 * @brief デッドゾーンのモード。
 */
enum class DeadZoneMode { IndependentAxes, Circular, None };

/**
 * @brief 入力デバイスのフィルタ。
 */
enum class DeviceFlags { All, KeyboardOnly, MouseOnly, GamePadOnly, KeyboardAndMouse, KeyboardAndGamePad, MouseAndGamePad };

/**
 * @brief 入力状態の絞り込みマスク。
 */
enum class InputStateMask { None, Trigger, Release };

/**
 * @brief 方向入力の離散値。
 */
enum class Direction { Up, Left, Down, Right, None };

/**
 * @brief 入力を一元管理するシステム（全メンバー静的）。
 * @details アクション名での問い合わせ、スティック・マウス座標の取得、
 *          デバイスの接続状態およびアクティブデバイスの判定を提供します。
 */
class InputSystem
{
private:
	static inline std::map<std::string, std::vector<std::unique_ptr<InputKey>>> inputKeys; //!< アクション名→入力キー集合
	static inline std::unique_ptr<InputKey> directionKeys[DIRECTION_SET_MAX][DIRECTION_KEY_NUM]; //!< 方向入力（上下左右）のキーセット
	static inline std::unordered_map<int, InputKey*> vKeyMap; //!< バーチャルキーコード→InputKey のマッピング（スクリプト用）
	static inline std::vector<std::unique_ptr<InputKey>> rawKeys; //!< 仮想キーコードで登録された全キーの集合（スクリプト用）
#if 0
	static inline std::vector<std::unique_ptr<InputKey>> navigationKeys[DIRECTION_KEY_NUM];
#endif // 0
private:
	InputSystem();
	~InputSystem() {}

public:
	/**
	 * @brief 初期化処理。
	 */
	static void Initialize();

	/**
	 * @brief 終了処理。
	 */
	static void Finalize();

	/**
	 * @brief 毎フレームの更新処理。
	 * @param elapsedTime 経過時間（秒）
	 */
	static void Update(float elapsedTime);

	/**
	 * @brief フレーム終了時の後処理。
	 * @details トリガー・リリース判定の更新など。
	 */
	static void EndFrame();

	/**
	 * @brief ウィンドウメッセージの処理。
	 * @param hwnd ウィンドウハンドル
	 * @param msg メッセージID
	 * @param wParam パラメータ1
	 * @param lParam パラメータ2
	 * @return 既定処理に委ねる値
	 */
	static LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	/**
	 * @brief アクションに対する入力状態を取得します。
	 * @param action アクション名（`inputKeys` のキー）
	 * @param state  トリガー/リリース/指定なしのフィルタ
	 * @param flag   参照するデバイスの種類
	 * @return 条件に合致する入力があれば true
	 */
	static bool GetInputState(const std::string& action, InputStateMask state = InputStateMask::None, DeviceFlags flag = DeviceFlags::All);

	/**
	 * @brief アクションに入力キーを登録します。
	 * @param action アクション名（任意の文字列）
	 * @param key 登録する入力キー（Keybord/Mouse/GamePad のいずれか）
	 */
	static void RegisterKey(const std::string& action, std::unique_ptr<InputKey> key);

	/**
	 * @brief 仮想キーコードに対する入力状態を取得します（主にスクリプト用）。
	 * @param vKey バーチャルキーコード
	 * @return 押されていれば true
	 */
	static bool GetKeyTrigger(int vKey);

	/**
	 * @brief 仮想キーコードに対するリリース状態を取得します（主にスクリプト用）。
	 * @param vKey バーチャルキーコード
	 * @return 離されたばかりなら true
	 */
	static bool GetKeyRelease(int vKey);

	/**
	 * @brief スティックの軸値を取得します（-1〜1）。
	 * @param side 左右
	 * @param axis X/Y
	 * @return 正規化された軸値
	 */
	static float GetAxis(Side side, Axis axis);

	/**
	 * @brief スティックの軸の生値を取得します（整数）。
	 * @param side 左右
	 * @param axis X/Y
	 * @return 軸の生値
	 */
	static int GetAxisRaw(Side side, Axis axis);

	/**
	 * @brief スティックの離散方向を取得します。
	 * @return 上下左右または None
	 */
	static Direction GetAxisDirection();

	/**
	 * @brief マウスカーソルの移動量を取得します。
	 * @param[out] x X 方向のデルタ
	 * @param[out] y Y 方向のデルタ
	 */
	static void GetMouseDelta(int& x, int& y);

	/**
	 * @brief マウスカーソルの X 座標を取得します。
	 * @return 現在の X 座標（ピクセル）
	 */
	static int GetMousePositionX();

	/**
	 * @brief マウスカーソルの Y 座標を取得します。
	 * @return 現在の Y 座標（ピクセル）
	 */
	static int GetMousePositionY();

	/**
	 * @brief マウスカーソルの座標を取得します。
	 * @param[out] position {x,y} の配列
	 */
	static void GetMousePosition(float position[2]);

	/**
	 * @brief マウスカーソルの座標を取得します。
	 * @return マウス座標のベクトル
	 */
	static Vector2 GetMousePosition();

	/**
	 * @brief 前フレームのマウス X 座標を取得します。
	 * @return 前回の X 座標
	 */
	static int GetOldMousePositionX();

	/**
	 * @brief 前フレームのマウス Y 座標を取得します。
	 * @return 前回の Y 座標
	 */
	static int GetOldMousePositionY();

	/**
	 * @brief マウスホイールの回転量を取得します。
	 * @return ホイールのデルタ量
	 */
	static float GetWheelDelta();

	/**
	 * @brief カーソルのロック状態を設定します。
	 * @param lock ロックするか
	 * @param changeVisible true の場合、表示状態も合わせて変更
	 */
	static void SetCursorLock(bool lock, bool changeVisible = true);

	/**
	 * @brief カーソルがロックされているか。
	 * @return ロック中なら true
	 */
	static bool IsCursorLock();

	/**
	 * @brief カーソルが表示されているか。
	 * @return 表示中なら true
	 */
	static bool IsCursorVisible();

private:
	/**
	 * @brief カーソルの表示/非表示を切り替えます。
	 * @param visible 表示するなら true
	 */
	static void SetCursorVisible(bool visible) {
		cursolVisible = visible;
		int count = 0;
		do {
			count = ShowCursor(visible);
		} while ((visible && count < 0) || (!visible && count >= 0));
	}

private:
	static inline InputDevice activeDevice = InputDevice::Keybord; //!< 最後に操作されたデバイス
public:
	/**
	 * @brief ゲームパッドが接続されているか。
	 * @return 接続されていれば true
	 */
	static bool IsGamepadConnected() { return isGamePadConnected; }
	/**
	 * @brief 現在アクティブなデバイスを取得します。
	 * @return デバイス種別
	 */
	static InputDevice GetActiveDevice() { return activeDevice; }

	/**
	 * @brief 入力の有効/無効を設定します。
	 * @param enable 有効にするなら true
	 */
	static void SetInputEnabled(bool enable) { inputEnabled = enable; }

	/**
	 * @brief 入力が有効かどうかを取得します。
	 * @return 有効なら true
	 */
	static bool IsInputEnabled() { return inputEnabled; }
private:
	friend class GamePad;
	/**
	 * @brief XInput の状態を取得します（内部用）。
	 */
	static XINPUT_STATE GetXInputState() { return state; }
private:
	static inline float m_axis[2][2]; //!< スティック軸値 [Side][Axis]
	static inline XINPUT_STATE state; //!< XInput の生状態
	static inline DeadZoneMode deadZoneMode = DeadZoneMode::Circular; //!< デッドゾーン処理モード
	static inline int					slot = 0; //!< 使用するコントローラのスロット番号
private:
	static inline int				mousePositionX[2]; //!< [0]=現在, [1]=前回
	static inline int				mousePositionY[2]; //!< [0]=現在, [1]=前回
	static inline float wheelDelta;                //!< マウスホイールのデルタ
public:
	static inline WPARAM inputChar{};     //!< 最新の文字入力（WM_CHAR）
	static inline WPARAM inputKeyDown{};  //!< 最新のキー押下（WM_KEYDOWN）
	static inline std::wstring inputString{}; //!< フレーム中に入力された文字列
	static inline std::string keyString{};   //!< フレーム中に押下されたキーの名称
private:
	static inline bool isGamePadConnected = false; //!< ゲームパッド接続状態
	
private:
	static inline bool cursolLock = false;   //!< カーソルロック状態
	static inline bool cursolVisible = true; //!< カーソル表示状態
	static inline bool inputEnabled = true;   //!< 入力有効状態
};
