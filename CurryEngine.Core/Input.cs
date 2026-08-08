

using CurryEngine.Interfaces;

namespace CurryEngine
{
    /// <summary>
    /// キーボードやマウスの入力を表す列挙型。各値は、Windows APIの仮想キーコードに対応しています。
    /// </summary>
    public enum KeyCode
    {
        // マウス
        Mouse0 = 0x01, // VK_LBUTTON
        Mouse1 = 0x02, // VK_RBUTTON
        Mouse2 = 0x04, // VK_MBUTTON

        // アルファベット
        A = 0x41, B = 0x42, C = 0x43, D = 0x44, E = 0x45,
        F = 0x46, G = 0x47, H = 0x48, I = 0x49, J = 0x4A,
        K = 0x4B, L = 0x4C, M = 0x4D, N = 0x4E, O = 0x4F,
        P = 0x50, Q = 0x51, R = 0x52, S = 0x53, T = 0x54,
        U = 0x55, V = 0x56, W = 0x57, X = 0x58, Y = 0x59,
        Z = 0x5A,

        // 数字
        Alpha0 = 0x30, Alpha1 = 0x31, Alpha2 = 0x32,
        Alpha3 = 0x33, Alpha4 = 0x34, Alpha5 = 0x35,
        Alpha6 = 0x36, Alpha7 = 0x37, Alpha8 = 0x38,
        Alpha9 = 0x39,

        // ファンクションキー
        F1 = 0x70, F2 = 0x71, F3 = 0x72, F4 = 0x73,
        F5 = 0x74, F6 = 0x75, F7 = 0x76, F8 = 0x77,
        F9 = 0x78, F10 = 0x79, F11 = 0x7A, F12 = 0x7B,

        // 特殊キー
        Space = 0x20,
        Return = 0x0D,
        Escape = 0x1B,
        Backspace = 0x08,
        Tab = 0x09,

        // 修飾キー
        LeftShift = 0xA0,
        RightShift = 0xA1,
        LeftControl = 0xA2,
        RightControl = 0xA3,
        LeftAlt = 0xA4,
        RightAlt = 0xA5,

        // 矢印キー
        UpArrow = 0x26,
        DownArrow = 0x28,
        LeftArrow = 0x25,
        RightArrow = 0x27,

        // ナビゲーション
        Insert = 0x2D,
        Delete = 0x2E,
        Home = 0x24,
        End = 0x23,
        PageUp = 0x21,
        PageDown = 0x22,
    }

    public enum GamepadButton
    {
        A = 0,
        B = 1,
        X = 2,
        Y = 3,
        LeftBumper = 4,
        RightBumper = 5,
        Back = 6,
        Start = 7,
        LeftStick = 8,
        RightStick = 9
    }

    /// <summary>
    /// 入力軸を表す列挙型です。水平軸と垂直軸の2種類があります。水平軸は左方向への入力が負の値、右方向への入力が正の値となります。垂直軸は上方向への入力が正の値、下方向への入力が負の値となります。
    /// </summary>
    public enum InputAxis
    {
        Horizontal = 0,
        Vertical = 1,
    }

    public enum GamepadStick
    {
        LeftStick = 0,
        RightStick = 1
    }


    /// <summary>
    /// 入力を管理する静的クラスです。キーボード、マウス、ゲームパッドなどの入力デバイスからの入力を取得するためのメソッドを提供します。
    /// </summary>
    public static class Input
    {
        /// <summary>
        /// Runtimeで使用される入力プロバイダーを取得または設定します。通常、エンジンの初期化時に自動的に設定されます。
        /// </summary>
        internal static IInputProvider? Provider { get; set; }

        /// <summary>
        /// 指定したキーが現在押されているかどうかを返します。Providerがnullの場合はfalseを返します。
        /// </summary>
        /// <param name="code"> 取得したいキーコードを指定します。KeyCode列挙型の値を使用します。</param>
        /// <returns> trueの場合、指定したキーが押されていることを示します。falseの場合、指定したキーが押されていないことを示します。</returns>
        public static bool GetKey(KeyCode code) => Provider?.GetKey(code) ?? false;

        /// <summary>
        /// 指定したキーが前のフレームで押されていて、現在は離されているかどうかを返します。Providerがnullの場合はfalseを返します。
        /// </summary>
        /// <param name="code"> 取得したいキーコードを指定します。KeyCode列挙型の値を使用します。</param>
        /// <returns> trueの場合、指定したキーが離されていることを示します。falseの場合、指定したキーが押されていることを示します。</returns>
        public static bool GetKeyUp(KeyCode code) => Provider?.GetKeyUp(code) ?? false;

        /// <summary>
        /// 指定したキーが前のフレームで離されていて、現在は押されているかどうかを返します。Providerがnullの場合はfalseを返します。
        /// </summary>
        /// <param name="code"> 取得したいキーコードを指定します。KeyCode列挙型の値を使用します。</param>
        /// <returns> trueの場合、指定したキーが押されていることを示します。falseの場合、指定したキーが押されていないことを示します。</returns>
        public static bool GetKeyDown(KeyCode code) => Provider?.GetKeyDown(code) ?? false;

        /// <summary>
        /// 指定したアクションキーが現在押されているかどうかを返します。Providerがnullの場合はfalseを返します。
        /// </summary>
        /// <param name="action"> 取得したいアクションキーの名前を指定します。InputManagerで定義されたアクションキーを指定します。</param>
        /// <returns> trueの場合、指定したアクションキーが押されていることを示します。falseの場合、指定したアクションキーが押されていないことを示します。</returns>
        public static bool GetAction(string action) => Provider?.GetAction(action) ?? false;

        /// <summary>
        /// 指定したアクションキーが前のフレームで押されていて、現在は離されているかどうかを返します。
        /// </summary>
        /// <param name="action"> アクションキーの名前</param>
        /// <returns> trueの場合、アクションキーが離されていることを示します。falseの場合、アクションキーが押されていることを示します。</returns>
        public static bool GetActionUp(string action) => Provider?.GetActionUp(action) ?? false;

        /// <summary>
        /// 指定したアクションキーが前のフレームで離されていて、現在は押されているかどうかを返します。
        /// </summary>
        /// <param name="action"> アクションキーの名前。InputManagerで定義されたアクションキーを指定します
        /// <returns> trueの場合、アクションキーが押されていることを示します。falseの場合、アクションキーが押されていないことを示します。</returns>
        public static bool GetActionDown(string action) => Provider?.GetActionDown(action) ?? false;

        /// <summary>
        /// 指定した軸の値を返します。値は-1から1の範囲で、0がニュートラルな位置を表します。例えば、水平軸の場合、左方向への入力は負の値、右方向への入力は正の値となります。このメソッドは、入力のスムージングを行った値を返します。
        /// </summary>
        /// <param name="side"> 取得したい軸の種類を指定します。GamepadStick列挙型の値を使用します。</param>
        /// <returns> 指定した軸の値を返します。Providerがnullの場合はVector2.zeroを返します。</returns>
        public static Vector2 GetAxis(GamepadStick side) => Provider?.GetAxis(side) ?? Vector2.zero;
        /// <summary>
        /// 指定した軸の値を返します。値は-1から1の範囲で、0がニュートラルな位置を表します。例えば、水平軸の場合、左方向への入力は負の値、右方向への入力は正の値となります。このメソッドは、入力のスムージングを行わず、生の値を返します。
        /// </summary>
        /// <param name="side"> 取得したい軸の種類を指定します。GamepadStick列挙型の値を使用します。</param>
        /// <returns> 指定した軸の値を返します。Providerがnullの場合はVector2.zeroを返します。</returns>
        public static Vector2 GetAxisRaw(GamepadStick side) => Provider?.GetAxisRaw(side) ?? Vector2.zero;

        /// <summary>
        /// 現在のマウスの位置を取得します。スクリーン座標系で表され、左上が(0,0)で右下が(スクリーン幅, スクリーン高さ)となります。Providerがnullの場合はVector2.zeroを返します。
        /// </summary>
        public static Vector2 MousePosition => Provider?.MousePosition ?? Vector2.zero;

        /// <summary>
        /// 現在のマウスの移動量を取得します。前のフレームからの変化量を表し、スクリーン座標系で表されます。左方向への移動は負の値、右方向への移動は正の値、上方向への移動は負の値、下方向への移動は正の値となります。
        /// </summary>
        public static Vector2 MouseDelta => Provider?.MouseDelta ?? Vector2.zero;

    }
}
