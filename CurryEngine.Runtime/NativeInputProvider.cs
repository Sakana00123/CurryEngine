using CurryEngine.Interfaces;
using CurryEngine.Runtime.Native;
using Microsoft.Win32.SafeHandles;

namespace CurryEngine.Runtime
{
    internal class NativeInputProvider : IInputProvider
    {
        internal int KeyCodeToInt(KeyCode code)
        {
            return (int)code;
        }
        internal string GamepadButtonToString(GamepadButton button)
        {
            switch (button)
            {
                case GamepadButton.A: return "GamePad_A";
                case GamepadButton.B: return "GamePad_B";
                case GamepadButton.X: return "GamePad_X";
                case GamepadButton.Y: return "GamePad_Y";
                case GamepadButton.LB: return "GamePad_LB";
                case GamepadButton.RB: return "GamePad_RB";
                case GamepadButton.DPadUp: return "GamePad_DU";
                case GamepadButton.DPadDown: return "GamePad_DD";
                case GamepadButton.DPadLeft: return "GamePad_DL";
                case GamepadButton.DPadRight: return "GamePad_DR";
                case GamepadButton.LS: return "GamePad_LS";
                case GamepadButton.RS: return "GamePad_RS";
                case GamepadButton.LT: return "GamePad_LT";
                case GamepadButton.RT: return "GamePad_RT";
                case GamepadButton.Start: return "GamePad_START";
                case GamepadButton.Back: return "GamePad_BACK";
                default:
                    throw new ArgumentOutOfRangeException(nameof(button), button, null);
            }
        }



        public bool GetKey(KeyCode code)
        {
            // NativeMethods (P/Invoke) 経由で C++ 側のキー状態を問い合わせる
            return NativeMethods.Input_GetKey((int)code);
        }
        public bool GetKey(GamepadButton button)
        {
            // NativeMethods (P/Invoke) 経由で C++ 側のキー状態を問い合わせる
            return NativeMethods.Input_GetAction(GamepadButtonToString(button));
        }

        public bool GetKeyDown(KeyCode code)
        {
            // NativeMethods (P/Invoke) 経由で C++ 側のキー状態を問い合わせる
            return NativeMethods.Input_GetKeyDown((int)code);
        }
        public bool GetKeyDown(GamepadButton button)
        {
            // NativeMethods (P/Invoke) 経由で C++ 側のキー状態を問い合わせる
            return NativeMethods.Input_GetActionDown(GamepadButtonToString(button));
        }

        public bool GetKeyUp(KeyCode key)
        {
            // NativeMethods (P/Invoke) 経由で C++ 側のキー状態を問い合わせる
            return NativeMethods.Input_GetKeyUp((int)key);
        }
        public bool GetKeyUp(GamepadButton button)
        {
            // NativeMethods (P/Invoke) 経由で C++ 側のキー状態を問い合わせる
            return NativeMethods.Input_GetActionUp(GamepadButtonToString(button));
        }

        public bool GetActionDown(string actionName)
        {
            // NativeMethods (P/Invoke) 経由で C++ 側のアクション状態を問い合わせる
            return NativeMethods.Input_GetActionDown(actionName);
        }

        public bool GetActionUp(string actionName)
        {
            // NativeMethods (P/Invoke) 経由で C++ 側のアクション状態を問い合わせる
            return NativeMethods.Input_GetActionUp(actionName);
        }

        public bool GetAction(string actionName)
        {
            // NativeMethods (P/Invoke) 経由で C++ 側のアクション状態を問い合わせる
            return NativeMethods.Input_GetAction(actionName);
        }

        public Vector2 GetAxis(GamepadStick side)
        {
            // NativeMethods (P/Invoke) 経由で C++ 側の軸状態を問い合わせる
            NativeMethods.Input_GetAxis((int)side, out Vector2 axis);
            return axis;
        }

        public Vector2 GetAxisRaw(GamepadStick side)
        {
            // NativeMethods (P/Invoke) 経由で C++ 側の軸状態を問い合わせる
            NativeMethods.Input_GetAxisRaw((int)side, out Vector2 axis);
            return axis;
        }

        public Vector2 MousePosition
        {
            get
            {
                // NativeMethods (P/Invoke) 経由で C++ 側のマウス位置を問い合わせる
                float x = NativeMethods.Input_GetMousePositionX();
                float y = NativeMethods.Input_GetMousePositionY();
                return new Vector2(x, y);
            }
        }

        public Vector2 MouseDelta
        {
            get
            {
                // NativeMethods (P/Invoke) 経由で C++ 側のマウス移動量を問い合わせる
                float deltaX = NativeMethods.Input_GetMouseDeltaX();
                float deltaY = NativeMethods.Input_GetMouseDeltaY();
                return new Vector2(deltaX, deltaY);
            }
        }

    }
}
