using System;

namespace CurryEngine.Attributes;

/// <summary>
/// C++から呼ばれるコールバックメソッドに付ける属性。
/// </summary>
[AttributeUsage(AttributeTargets.Method)]
public class EngineCallbackAttribute : Attribute { }

