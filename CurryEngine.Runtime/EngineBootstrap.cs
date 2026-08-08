using CurryEngine.Scripting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CurryEngine.Runtime
{
    public static class EngineBootstrap
    {

        /// <summary>
        /// CurryEngine の初期化処理を行います。SharedAssemblyRegistry に必要なアセンブリを登録します。
        /// </summary>
        public static void Initialize()
        {
            // CurryEngine.Core.dll を SharedAssemblyRegistry に登録
            CurryEngine.Scripting.SharedAssemblyRegistry.RegisterFromType(typeof(CurryEngine.Scripting.SharedAssemblyRegistry));
            // CurryEngine.Runtime.dll を SharedAssemblyRegistry に登録
            CurryEngine.Scripting.SharedAssemblyRegistry.RegisterFromType(typeof(CurryEngine.Runtime.HotReload.ScriptLoadContext));
            // CurryEngine.API.dll を SharedAssemblyRegistry に登録
            var executableDir = EngineRuntime.GetExecutableDirectory();
            if (executableDir == null)
            {
                File.AppendAllText("CurryEngine_Runtime_Error.log", $"実行可能ファイルのディレクトリが取得できませんでした。{DateTime.Now}\n");
                return;
            }
            var apiDllPath = Path.Combine(executableDir, "CurryEngine.API.dll");
            if (File.Exists(apiDllPath))
            {
                CurryEngine.Scripting.SharedAssemblyRegistry.RegisterFromPath(apiDllPath);
            }
            else
            {
                File.AppendAllText("CurryEngine_Runtime_Error.log", $"CurryEngine.API.dll が見つかりませんでした。{DateTime.Now}\n");
            }
        }
    }
}
