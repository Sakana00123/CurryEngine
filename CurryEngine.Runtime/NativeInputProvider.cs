using CurryEngine.Interfaces;
using CurryEngine.Runtime.Native;
using Microsoft.Win32.SafeHandles;

namespace CurryEngine.Runtime
{
    internal class NativeInputProvider : IInputProvider
    {
        public bool GetKey(KeyCode code)
        {
            // NativeMethods (P/Invoke) 経由で C++ 側のキー状態を問い合わせる
            return NativeMethods.Input_GetKey((int)code);
        }

        public bool GetKeyDown(KeyCode code)
        {
            // NativeMethods (P/Invoke) 経由で C++ 側のキー状態を問い合わせる
            return NativeMethods.Input_GetKeyDown((int)code);
        }

        public bool GetKeyUp(KeyCode key)
        {
            // NativeMethods (P/Invoke) 経由で C++ 側のキー状態を問い合わせる
            return NativeMethods.Input_GetKeyUp((int)key);
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
            return NativeMethods.Input_GetAxis((int)side);
        }

        public Vector2 GetAxisRaw(GamepadStick side)
        {
            // NativeMethods (P/Invoke) 経由で C++ 側の軸状態を問い合わせる
            return NativeMethods.Input_GetAxisRaw((int)side);
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
