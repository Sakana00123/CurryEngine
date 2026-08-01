
namespace CurryEngine.Interfaces
{
    /// <summary>
    /// 入力情報を提供するインターフェースです。キー入力やマウス位置の取得など、ユーザーからの入力に関する機能を定義します。
    /// </summary>
    internal interface IInputProvider
    {
        /// <summary>
        /// 指定されたキーが押されたかどうかを取得します。
        /// </summary>
        /// <param name="key"> 取得するキーコード</param>
        /// <returns> 指定されたキーが押された場合は true、そうでない場合は false を返します。</returns>
        public bool GetKeyDown(KeyCode key);

        /// <summary>
        /// 指定されたキーが離されたかどうかを取得します。
        /// </summary>
        /// <param name="key"> 取得するキーコード</param>
        /// <returns> 指定されたキーが離された場合は true、そうでない場合は false を返します。</returns>
        public bool GetKeyUp(KeyCode key);

        /// <summary>
        /// 指定されたキーが押されているかどうかを取得します。
        /// </summary>
        /// <param name="key"> 取得するキーコード</param>
        /// <returns> 指定されたキーが押されている場合は true、そうでない場合は false を返します。</returns>
        public bool GetKey(KeyCode key);

        /// <summary>
        /// 指定されたアクションが押されたかどうかを取得します。
        /// </summary>
        /// <param name="actionName"> 取得するアクション名</param>
        /// <returns> 指定されたアクションが押された場合は true、そうでない場合は false を返します。</returns>
        public bool GetActionDown(string actionName);

        /// <summary>
        /// 指定されたアクションが離されたかどうかを取得します。
        /// </summary>
        /// <param name="actionName"> 取得するアクション名</param>
        /// <returns> 指定されたアクションが離された場合は true、そうでない場合は false を返します。</returns>
        public bool GetActionUp(string actionName);

        /// <summary>
        /// 指定されたアクションが押されているかどうかを取得します。
        /// </summary>
        /// <param name="actionName"> 取得するアクション名</param>
        /// <returns> 指定されたアクションが押されている場合は true、そうでない場合は false を返します。</returns>
        public bool GetAction(string actionName);

        /// <summary>
        /// 指定された軸の値を取得します。スムージングや補間が適用される場合があります。
        /// </summary>
        /// <param name="side"> 取得する軸</param>
        /// <returns> 指定された軸の値を返します。値は -1.0 から 1.0 の範囲で返されます。</returns>
        public Vector2 GetAxis(GamepadStick side);

        /// <summary>
        /// 指定された軸の値を取得します。生の値を返すため、スムージングや補間は行われません。
        /// </summary>
        /// <param name="side"> 取得する軸</param>
        /// <returns> 指定された軸の生の値を返します。値は -1.0 から 1.0 の範囲で返されます。</returns>
        public Vector2 GetAxisRaw(GamepadStick side);

        /// <summary>
        /// マウスの現在の位置を取得します。
        /// </summary>
        public Vector2 MousePosition { get; }

        /// <summary>
        /// マウスの前回フレームからの移動量を取得します。
        /// </summary>
        public Vector2 MouseDelta { get; }
    }
}
