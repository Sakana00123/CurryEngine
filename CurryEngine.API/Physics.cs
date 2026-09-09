
using CurryEngine.Runtime.Native;

namespace CurryEngine
{
    public static class Physics
    {
        /// <summary>
        /// シーン内のオブジェクトに対してレイキャストを行い、最初にヒットしたオブジェクトの情報を取得します。
        /// </summary>
        /// <param name="origin"> レイの発射元の位置</param>
        /// <param name="direction"> レイの方向</param>
        /// <param name="hitInfo"> ヒットしたオブジェクトの情報を格納するRaycastHit構造体</param>
        /// <param name="maxDistance"> レイの最大距離</param>
        /// <param name="layerMask"> レイヤーマスク。デフォルトはすべてのレイヤーを対象とする</param>
        /// <returns> ヒットした場合はtrue、ヒットしなかった場合はfalse</returns>
        public static bool Raycast(Vector3 origin, Vector3 direction, out RaycastHit hitInfo, float maxDistance, int layerMask = ~0)
        {
            // CurryEngineの内部でレイキャストを実行するためのネイティブ関数を呼び出す
            return NativeMethods.Physics_Raycast(origin, direction, out hitInfo, maxDistance, layerMask);
        }
    }
}
