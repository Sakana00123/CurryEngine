
using System.Reflection;

namespace CurryEngine.Runtime.Reflection;

public class FieldMeta
{
    public string Name { get; }
    public string TypeName { get; }
    public object? Value { get; }
    
    public bool IsComponentReference { get; set; } = false; // フィールドがComponentへの参照かどうかを示すプロパティ

    public bool IsReadOnly { get; set; } = false; // フィールドが読み取り専用かどうかを示すプロパティ

    public bool IsEditable { get; set; } = true; // フィールドが編集可能かどうかを示すプロパティ

    public bool IsVisible { get; set; } = true; // フィールドがインスペクタに表示されるかどうかを示すプロパティ

    // フィールドの属性を表すプロパティ
    public string? Header { get; }
    public string? Tooltip { get; }
    public float? RangeMin { get; }
    public float? RangeMax { get; }
    public float? Min { get; }
    public float? Max { get; }

    public FieldMeta(FieldInfo fieldInfo, object? instance)
    {
        Name = fieldInfo.Name;
        TypeName = ToTypeName(fieldInfo.FieldType);
        Value = fieldInfo.GetValue(instance);

        // フィールドがComponentへの参照かどうかを判定
        //IsComponentReference = fieldInfo.FieldType.IsSubclassOf(typeof(Component));
        IsComponentReference = fieldInfo.FieldType?.BaseType?.Name == "Component"; // ALCの型同一性の問題を回避するため、BaseTypeの名前で判定

        // フィールドの属性を取得
        Header = fieldInfo.GetCustomAttribute<Header>()?.Text;
        Tooltip = fieldInfo.GetCustomAttribute<Tooltip>()?.Text;

        var rangeAttr = fieldInfo.GetCustomAttribute<Range>();
        if (rangeAttr != null)
        {
            RangeMin = rangeAttr.Min;
            RangeMax = rangeAttr.Max;
        }

        Min = fieldInfo.GetCustomAttribute<Min>()?.Value;
        Max = fieldInfo.GetCustomAttribute<Max>()?.Value;
    }

    /// <summary>
    /// 型をわかりやすい形式で表現するためのヘルパーメソッド。(ジェネリック型を含む)
    /// </summary>
    /// <param name="type">変換する型</param>
    /// <returns>型のわかりやすい名前</returns>
    /// <remarks> ジェネリック型の場合、例えば List<int> は System.Collections.Generic.List<System.Int32> として表示されるため、わかりやすい形式に変換する。</remarks>
    private static string ToTypeName(Type type)
    {
        if (type == typeof(float)) return "float";
        if (type == typeof(float)) return "float"; // System.Single も float として表示
        if (type == typeof(double)) return "double";
        if (type == typeof(int)) return "int";
        if (type == typeof(bool)) return "bool";
        if (type == typeof(string)) return "string";
        if (type == typeof(Vector3)) return "Vector3";
        if (type == typeof(Quaternion)) return "Quaternion";
        if (type == typeof(GameObject)) return "GameObject";

        return type.Name;
    }

    public Dictionary<string, object?> ToJson()
    {
        var dict = new Dictionary<string, object?>
        {
            { "name", Name },
            { "type", TypeName },
            { "value", SerializeValue(Value) }
        };

        // フィールドの状態を追加
        dict["isComponentReference"] = IsComponentReference;

        // フィールドの属性を追加
        if (Header != null) dict["header"] = Header;
        if (Tooltip != null) dict["tooltip"] = Tooltip;
        if (RangeMin.HasValue) dict["rangeMin"] = RangeMin.Value;
        if (RangeMax.HasValue) dict["rangeMax"] = RangeMax.Value;
        if (Min.HasValue) dict["min"] = Min.Value;
        if (Max.HasValue) dict["max"] = Max.Value;

        return dict;
    }

    private static object? SerializeValue(object? value)
    {
        if (value == null)
            return null;
        if (value is float f)             return f; // JSONではfloatはそのまま数値として表現される
        if (value is double d)            return d; // JSONではdoubleもそのまま数値として表現される
        if (value is int i)               return i; // JSONではintもそのまま数値として表現される
        if (value is string s)            return s; // JSONではstringもそのまま文字列として表現される
        if (value is bool b)              return b; // JSONではboolもそのままtrue/falseとして表現される

        // 型名で判定 (ALC 型同一性の問題を回避するため)
        var typeName = value.GetType().Name;
        if (typeName == "Vector3")
        {
            var t = value.GetType();
            var x = (float)t.GetField("x")!.GetValue(value)!;
            var y = (float)t.GetField("y")!.GetValue(value)!;
            var z = (float)t.GetField("z")!.GetValue(value)!;
            return new { x, y, z };
        }
        if (typeName == "Quaternion")
        {
            var t = value.GetType();
            var x = (float)t.GetField("x")!.GetValue(value)!;
            var y = (float)t.GetField("y")!.GetValue(value)!;
            var z = (float)t.GetField("z")!.GetValue(value)!;
            var w = (float)t.GetField("w")!.GetValue(value)!;
            return new { x, y, z, w };
        }
        if (value is IEnumerable<object> enumerable) // 配列やリストなどのコレクション型をシリアライズ
        {
            var list = new List<object?>();
            foreach (var item in enumerable)
            {
                list.Add(SerializeValue(item)); // 再帰的にシリアライズ
            }
            return list;
        }
        if (value is Enum e) // 列挙型は名前でシリアライズ
        {
            return e.ToString();
        }
        bool isComponent = false;
        var baseType = value.GetType().BaseType;
        while (baseType != null)
        {
            if (baseType.Name == "Component")
            {
                isComponent = true;
                break;
            }
            baseType = baseType.BaseType;
        }
        if (isComponent) // Componentはidを文字列でシリアライズ
        {
            var component = (Component)value;
            return new string($"Component(objectId: {component.objectId}, ownerId: {component.ownerId})");
        }
        if (value.GetType().Name == "GameObject") // GameObjectはidを文字列でシリアライズ
        {
            var gameObject = (GameObject)value;
            return new string($"GameObject(objectId: {gameObject.objectId})");
        }


        // その他の型はnullを返すか、必要に応じてさらにシリアライズ方法を追加する


        return null;
    }

}
