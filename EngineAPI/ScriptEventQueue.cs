using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CurryEngine;

internal static class ScriptEventQueue
{
    readonly record struct ScriptEvent(object Instance, Action<object> Invoke);
    private static readonly Queue<ScriptEvent> s_queue = new();

    internal static void Enqueue(object instance, Action<object> invoke)
    {
        s_queue.Enqueue(new ScriptEvent(instance, invoke));
    }

    // C++のメインループから呼ばれる: キューに溜まったイベントを順次処理する
    internal static void Process()
    {
        while (s_queue.Count > 0)
        {
            var evt = s_queue.Dequeue();
            try { evt.Invoke(evt.Instance); }
            catch (Exception ex)
            {
                Debug.LogError($"Error processing script event: {ex}");
            }
        }
    }
}
