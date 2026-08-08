using System.Reflection;
using System.Runtime.Loader;

namespace CurryEngine.Scripting
{
    /// <summary>
    /// プロセス生存期間中ずっと不変な「常駐アセンブリ」を一元管理する。
    /// Core/Runtime/API/Physics/UIなど、型IDを固定したいものはすべてここに登録する。
    /// </summary>
    internal static class SharedAssemblyRegistry
    {
        private static readonly Dictionary<string, Assembly> s_assemblies = new();

        /// <summary>
        /// 指定したアセンブリを、SharedAssemblyRegistry に登録する。
        /// </summary>
        /// <param name="typeInAssembly"> 登録したいアセンブリに含まれる型</param>
        public static void RegisterFromType(Type typeInAssembly)
        {
            var asm = typeInAssembly.Assembly;
            s_assemblies[asm.GetName().Name!] = asm;
        }

        /// <summary>
        /// 指定したパスのアセンブリを、SharedAssemblyRegistry に登録する。
        /// </summary>
        /// <param name="dllPath"> 登録したいアセンブリのDLLパス</param>
        /// <returns> 登録されたアセンブリ</returns>
        public static Assembly RegisterFromPath(string dllPath)
        {
            var name = Path.GetFileNameWithoutExtension(dllPath);
            if (s_assemblies.TryGetValue(name, out var cached))
                return cached;

            var asm = AssemblyLoadContext.Default.LoadFromAssemblyPath(dllPath);
            s_assemblies[name] = asm;
            return asm;
        }

        /// <summary>
        /// 指定したアセンブリ名のアセンブリを、SharedAssemblyRegistry から取得する。
        /// </summary>
        /// <param name="assemblyName"> 取得したいアセンブリの名前</param>
        /// <returns> 取得されたアセンブリ、存在しない場合は null</returns>
        public static Assembly? TryGet(string assemblyName)
            => s_assemblies.TryGetValue(assemblyName, out var asm) ? asm : null;
    }

    public static class ReloadDiagnostics
    {
        public static void AssertSharedTypeIdentity()
        {
            var core = SharedAssemblyRegistry.TryGet("CurryEngine.Core");
            Debug.Log($"Core Assembly identity: {core?.GetHashCode()}");

            var runtime = SharedAssemblyRegistry.TryGet("CurryEngine.Runtime");
            Debug.Log($"Runtime Assembly identity: {runtime?.GetHashCode()}");

            var api = SharedAssemblyRegistry.TryGet("CurryEngine.API");
            Debug.Log($"API Assembly identity: {api?.GetHashCode()}");
        }
    }
}
