
namespace CurryEngine.Runtime
{
    /// <summary>
    /// GameObject インスタンスをキャッシュするための静的クラスです。objectId をキーとして GameObject インスタンスを管理します。
    /// </summary>
    internal static class GameObjectCache
    {
        private static readonly Dictionary<ulong, GameObject> s_cache = new();

        /// <summary>
        /// 指定された objectId の GameObject インスタンスをキャッシュから取得します。存在しない場合は新しいインスタンスを作成してキャッシュに追加します。
        /// </summary>
        /// <param name="objectId">取得または作成する GameObject の objectId</param>
        /// <returns>指定された objectId の GameObject インスタンス</returns>
        internal static GameObject GetOrCreate(ulong objectId)
        {
            if (!s_cache.TryGetValue(objectId, out var gameObject))
            {
                gameObject = new GameObject(objectId);
                s_cache[objectId] = gameObject;
            }
            return gameObject;
        }

        /// <summary>
        /// 指定された objectId の GameObject インスタンスをキャッシュから削除します。
        /// </summary>
        /// <param name="objectId">削除する GameObject の objectId</param>
        internal static void Remove(ulong objectId)
        {
            s_cache.Remove(objectId);
        }

        /// <summary>
        /// キャッシュをクリアします。すべての GameObject インスタンスが破棄されます。
        /// </summary>
        internal static void ClearAll()
        {
            s_cache.Clear();
        }
    }
}
