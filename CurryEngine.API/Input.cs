

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


    public enum GamepadSide
    {
        Left = 0,
        Right = 1,
    }
    public enum GamepadAxis
    {
        X = 0,
        Y = 1,
    }


    public static class Input
    {
        /// <summary>
        /// 指定したキーが現在押されているかどうかを返します。
        /// </summary>
        public static bool GetKey(KeyCode key) => Interop.NativeMethods.Input_GetKey((int)key);
        /// <summary>
        /// 指定したキーが前のフレームで押されていて、現在は離されているかどうかを返します。
        /// </summary>
        public static bool GetKeyUp(KeyCode key) => Interop.NativeMethods.Input_GetKeyUp((int)key);
        /// <summary>
        /// 指定したキーが前のフレームで離されていて、現在は押されているかどうかを返します。
        /// </summary>
        public static bool GetKeyDown(KeyCode key) => Interop.NativeMethods.Input_GetKeyDown((int)key);

        /// <summary>
        /// 指定したアクションキーが現在押されているかどうかを返します。
        /// </summary>
        /// <param name="action">アクションキーの名前。InputManagerで定義されたアクションキーを指定します。</param>
        public static bool GetAction(string action) => Interop.NativeMethods.Input_GetAction(action);

        /// <summary>
        /// 指定したアクションキーが前のフレームで押されていて、現在は離されているかどうかを返します。
        /// </summary>
        /// <param name="action">アクションキーの名前。InputManagerで定義されたアクションキーを指定します。</param>
        public static bool GetActionUp(string action) => Interop.NativeMethods.Input_GetActionUp(action);

        /// <summary>
        /// 指定したアクションキーが前のフレームで離されていて、現在は押されているかどうかを返します。
        /// </summary>
        /// <param name="action">アクションキーの名前。InputManagerで定義されたアクションキーを指定します。</param>
        public static bool GetActionDown(string action) => Interop.NativeMethods.Input_GetActionDown(action);

        /// <summary>
        /// 指定したゲームパッドの軸の値を返します。値は-1から1の範囲で、0がニュートラルな位置を表します。例えば、左スティックのX軸を取得する場合は、sideにGamepadSide.Left、axisにGamepadAxis.Xを指定します。
        /// </summary>
        /// <param name="side">ゲームパッドの左右を指定します。GamepadSide.Leftは左スティック、GamepadSide.Rightは右スティックを表します。</param>
        /// <param name="axis">取得したい軸を指定します。GamepadAxis.Xは水平軸、GamepadAxis.Yは垂直軸を表します。</param>
        /// <returns>float値で、-1から1の範囲。0がニュートラルな位置を表します。</returns>
        public static float GetAxis(GamepadSide side, GamepadAxis axis) => Interop.NativeMethods.Input_GetAxis((int)side, (int)axis);

        /// <summary>
        /// 指定したゲームパッドの軸の生の値を返します。値は-1から1の範囲で、0がニュートラルな位置を表します。GetAxisと異なり、GetAxisRawは入力のスムージングやデッドゾーンの処理を行わないため、より直接的な入力値を取得できます。例えば、左スティックのX軸を取得する場合は、sideにGamepadSide.Left、axisにGamepadAxis.Xを指定します。
        /// </summary>
        /// <param name="side">ゲームパッドの左右を指定します。GamepadSide.Leftは左スティック、GamepadSide.Rightは右スティックを表します。</param>
        /// <param name="axis">取得したい軸を指定します。GamepadAxis.Xは水平軸、GamepadAxis.Yは垂直軸を表します。</param>
        /// <returns>int値で、-1から1の範囲。0がニュートラルな位置を表します。</returns>
        public static int GetAxisRaw(GamepadSide side, GamepadAxis axis) => Interop.NativeMethods.Input_GetAxisRaw((int)side, (int)axis);


        public static Vector2 mousePosition
        {
            get
            {
                Vector2 pos = new Vector2();
                pos.x = Interop.NativeMethods.Input_GetMousePositionX();
                pos.y = Interop.NativeMethods.Input_GetMousePositionY();
                return pos;
            }
        }

        public static Vector2 mouseDelta
        {
            get
            {
                Vector2 delta = new Vector2();
                delta.x = Interop.NativeMethods.Input_GetMouseDeltaX();
                delta.y = Interop.NativeMethods.Input_GetMouseDeltaY();
                return delta;
            }
        }

    }
}
