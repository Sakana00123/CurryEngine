using System.Reflection;
using System.Text.Json;

namespace CurryEngine.Runtime.Reflection;

public static class ScriptInspector
{
    // フィールドに特定の属性が付いているかどうかを判定するためのヘルパーメソッド
    private static bool HasAttribute(FieldInfo field, string attributeName)
        => field.GetCustomAttributes()
            .Any(a => a.GetType().Name == attributeName);

    // Type が Component の派生型かどうかを判定するためのヘルパーメソッド
    internal static bool IsComponentType(Type type)
    {
        for (Type? current = type; current != null; current = current.BaseType)
        {
            if (current.Name == nameof(Component))
            {
                return true;
            }
        }
        return false;
    }

    // インスペクタに表示するフィールドの情報を取得するためのヘルパーメソッド
    public static List<FieldMeta> GetFields(object instance)
    {
        var result = new List<FieldMeta>();
        var type = instance.GetType();

        // Behaviour 自体のフィールドは除外 (Transform などの基底クラスのフィールドは含める)
        var baseFields = typeof(Behaviour)
            .GetFields(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance)
            .Select(f => f.Name)
            .ToHashSet();

        var flags = BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance;
        foreach (var field in type.GetFields(flags))
        {
            // Behaviour クラスのフィールドはスキップ
            if (baseFields.Contains(field.Name))
                continue;

            // [NonSerialized] / [HideInInspector] 属性が付いているフィールドはスキップ
            if (HasAttribute(field, nameof(NonSerialized)))
            {
                //Debug.LogWarning($"[ScriptInspector] Skipping field '{field.Name}' in '{type.Name}' because it has [NonSerialized] attribute.");
                continue;
            }
            if (HasAttribute(field, nameof(HideInInspector)))
            {
                //Debug.LogWarning($"[ScriptInspector] Skipping field '{field.Name}' in '{type.Name}' because it has [HideInInspector] attribute.");
                continue;
            }

            // public か [SerializeField] 属性が付いているフィールドのみ対象
            bool isPublic = field.IsPublic;
            bool hasSerializeField = HasAttribute(field, nameof(SerializeField));
            if (!isPublic && !hasSerializeField)
            {
                //Debug.LogWarning($"[ScriptInspector] Skipping field '{field.Name}' in '{type.Name}' because it is not public and does not have [SerializeField] attribute.");
                continue;
            }

            // フィールドの値を取得
            var meta = new FieldMeta(field, instance);
            result.Add(meta);
        }

        return result;
    }

    internal static readonly JsonSerializerOptions JsonOptions = new JsonSerializerOptions
    {
        WriteIndented = true,
    };

    /// <summary>
    /// インスペクタに表示するフィールドの情報を JSON 形式で取得するためのヘルパーメソッド。(C++ からの編集用)
    /// </summary>
    /// <param name="instance">インスペクタに表示するフィールドの情報を取得したいオブジェクトのインスタンス</param>
    /// <returns>インスペクタに表示するフィールドの情報を JSON 形式で表現した文字列</returns>
    public static string GetFieldsJson(object instance)
    {
        var fields = GetFields(instance);
        var list = fields.Select(f => f.ToJson()).ToList();
        return JsonSerializer.Serialize(list);
    }

    public static void SetFieldValue(object instance, string fieldName, string valueJson)
    {
        var type = instance.GetType();
        var field = type.GetField(fieldName,
            BindingFlags.Public |
            BindingFlags.NonPublic |
            BindingFlags.Instance);
        if (field == null)
        {
            // インスタンスからフィールドが見つからない場合はスキップする。
            return;
        }

        // JSON から値をデシリアライズしてフィールドに設定
        try
        {
            var value = ParseValue(field, valueJson);

            if (value == null)
            {
                Debug.LogError($"[ScriptInspector] SetFieldValue failed: Unsupported field type '{field.FieldType.Name}' for field '{fieldName}' in type '{type.Name}'. Value JSON: {valueJson}");
                return;
            }

            field.SetValue(instance, value);
            // セット後の値を確認
            Debug.Log($"[SetFieldValue] {fieldName} = {field.GetValue(instance)}");
        }
        catch (Exception e)
        {
            Debug.LogError(e.ToString());
            throw;
        }
    }

    private static object? ParseValue(FieldInfo field, string valueJson)
    {
        string typeName = field.FieldType.Name;

        switch (typeName)
        {
            case "Single": // float
            case "float":
                return JsonSerializer.Deserialize<float>(valueJson);
            case "Int32": // int
            case "int":
                return JsonSerializer.Deserialize<int>(valueJson);
            case "Boolean": // bool
            case "bool":
                return JsonSerializer.Deserialize<bool>(valueJson);
            case "String": // string
            case "string":
                return JsonSerializer.Deserialize<string>(valueJson);
            case "Vector2":
            {
                var doc = JsonDocument.Parse(valueJson).RootElement;
                float x = doc.GetProperty("x").GetSingle();
                float y = doc.GetProperty("y").GetSingle();
                // リフレクションで生成(ALC 型同一性問題を回避するため)
                var ctor = field.FieldType.GetConstructor(
                    [typeof(float), typeof(float)]);
                return ctor?.Invoke([x, y]);
            }
            case "Vector3":
            {
                var doc = JsonDocument.Parse(valueJson).RootElement;
                float x = doc.GetProperty("x").GetSingle();
                float y = doc.GetProperty("y").GetSingle();
                float z = doc.GetProperty("z").GetSingle();

                // リフレクションで生成(ALC 型同一性問題を回避するため)
                var ctor = field.FieldType.GetConstructor(
                    [typeof(float), typeof(float), typeof(float)]);
                return ctor?.Invoke([x, y, z]);
            }
            case "Quaternion":
            {
                var doc = JsonDocument.Parse(valueJson).RootElement;
                float x = doc.GetProperty("x").GetSingle();
                float y = doc.GetProperty("y").GetSingle();
                float z = doc.GetProperty("z").GetSingle();
                float w = doc.GetProperty("w").GetSingle();
                // リフレクションで生成(ALC 型同一性問題を回避するため)
                var ctor = field.FieldType.GetConstructor(
                    [typeof(float), typeof(float), typeof(float), typeof(float)]);
                return ctor?.Invoke([x, y, z, w]);
            }
            case "Color":
            {
                var doc = JsonDocument.Parse(valueJson).RootElement;
                float r = doc.GetProperty("r").GetSingle();
                float g = doc.GetProperty("g").GetSingle();
                float b = doc.GetProperty("b").GetSingle();
                float a = doc.GetProperty("a").GetSingle();
                // リフレクションで生成(ALC 型同一性問題を回避するため)
                var ctor = field.FieldType.GetConstructor(
                    [typeof(float), typeof(float), typeof(float), typeof(float)]);
                return ctor?.Invoke([r, g, b, a]);
            }
            case "PrefabReference":
            {
                string? AssetId = JsonSerializer.Deserialize<string>(valueJson);
                if (AssetId == null)
                {
                    Debug.LogError($"[ScriptInspector] Failed to parse PrefabReference from JSON. Value: {valueJson}");
                    return null;
                }
                // リフレクションで生成(ALC 型同一性問題を回避するため)
                var ctor = field.FieldType.GetConstructor([typeof(string)]);
                return ctor?.Invoke([AssetId]);
            }
            case "GameObject":
                {
                    try
                    {
                        var ctor = field.FieldType.GetConstructor([typeof(ulong)]);
                        if (valueJson == null)
                        {
                            // valueJson が null の場合はデフォルト値(例: null)を返す。GameObject のフィールドは null 許容型であるべきだが、念のため。
                            return ctor?.Invoke([0]);
                        }
                        // $"GameObject(objectId: {objectId})" の形式でシリアライズされているため、objectId の部分を抜き取る
                        var idStart = valueJson?.IndexOf("objectId: ") + "objectId: ".Length ?? 0;
                        var idEnd = valueJson?.IndexOf(")") ?? 0;
                        if (idStart < 0 || idEnd < 0 || idEnd <= idStart) // objectId を正しく抜き取れない場合(例: フォーマットが変わった、valueJson が null など)
                        {
                            Debug.LogError($"[ScriptInspector] Failed to parse GameObject id from JSON. Value: {valueJson}");
                            return null;
                        }
                        var id = ulong.Parse(valueJson![idStart..idEnd]);
                        Debug.Log($"[ScriptInspector] Parsed GameObject id: {id}");
                        // リフレクションで生成(ALC 型同一性問題を回避するため)
                        return ctor?.Invoke([id]);
                    }
                    catch (Exception e)
                    {
                        Debug.LogError($"[ScriptInspector] Exception while parsing GameObject from JSON: {e.Message}. Value: {valueJson}");
                        return null;
                    }
                }
            default:
                {
                    // 最後に、コンポーネントが継承されている可能性を考慮して、field.FieldType.IsSubclassOf(typeof(Component)) をチェックする
                    //if (field.FieldType.IsSubclassOf(typeof(Component)))

                    //if (field.FieldType?.BaseType?.Name == "Component") // ALC 型同一性問題を回避するため、BaseType の Name を比較する
                    if (IsComponentType(field.FieldType))
                    {
                        try
                        {
                            if (valueJson == null)
                            {
                                // valueJson が null の場合はデフォルト値(例: null)を返す。Component のフィールドは null 許容型であるべきだが、念のため。
                                return ComponentCache.CreateInstance(field.FieldType, 0, 0);
                            }
                            // $"Component(objectId: {objectId}, ownerId: {ownerId})" の形式でシリアライズされているため、objectId と ownerId の部分を抜き取る
                            var objectIdStart = valueJson.IndexOf("objectId: ") + "objectId: ".Length;
                            var objectIdEnd = valueJson.IndexOf(",", objectIdStart);
                            if (objectIdStart < 0 || objectIdEnd < 0 || objectIdEnd <= objectIdStart) // objectId を正しく抜き取れない場合(例: フォーマットが変わった、valueJson が null など)
                            {
                                Debug.LogError($"[ScriptInspector] Failed to parse Component objectId from JSON. Value: {valueJson}");
                                return null;
                            }
                            var ownerIdStart = valueJson.IndexOf("ownerId: ") + "ownerId: ".Length;
                            var ownerIdEnd = valueJson.IndexOf(")", ownerIdStart);
                            if (ownerIdStart < 0 || ownerIdEnd < 0 || ownerIdEnd <= ownerIdStart) // ownerId を正しく抜き取れない場合(例: フォーマットが変わった、valueJson が null など)
                            {
                                Debug.LogError($"[ScriptInspector] Failed to parse Component ownerId from JSON. Value: {valueJson}");
                                return null;
                            }
                            var objectId = ulong.Parse(valueJson[objectIdStart..objectIdEnd]);
                            var ownerId = ulong.Parse(valueJson[ownerIdStart..ownerIdEnd]);
                            return ComponentCache.GetOrCreate(ownerId, objectId, field.FieldType);
                        }
                        catch (Exception e)
                        {
                            Debug.LogError($"[ScriptInspector] Exception while parsing Component from JSON: {e.Message}. Value: {valueJson}");
                            return null;
                        }
                    }

                    // サポートされていない型の場合はエラーをログに出力して null を返す
                    Debug.LogWarning($"[ScriptInspector] Unsupported field type '{typeName}' for JSON deserialization. Value: {valueJson}");
                    return null;
                }
        }

    }

    internal static object?[]? ParseParameters(string parametersJsonStr)
    {
        if (string.IsNullOrEmpty(parametersJsonStr))
        {
            return Array.Empty<object?>();
        }
        try
        {
            var parametersJson = JsonDocument.Parse(parametersJsonStr).RootElement;
            if (parametersJson.ValueKind != JsonValueKind.Array)
            {
                return null;
            }
            var parameters = new List<object?>();
            foreach (var param in parametersJson.EnumerateArray())
            {
                // パラメータの型を判定して適切にデシリアライズする
                switch (param.ValueKind)
                {
                    case JsonValueKind.Number:
                        if (param.TryGetInt32(out int intValue))
                        {
                            parameters.Add(intValue);
                        }
                        else if (param.TryGetDouble(out double doubleValue))
                        {
                            parameters.Add(doubleValue);
                        }
                        else
                        {
                            Debug.LogError($"[ScriptInspector] Unsupported number type in parameters JSON: {param}");
                            return null;
                        }
                        break;
                    case JsonValueKind.String:
                        parameters.Add(param.GetString());
                        break;
                    case JsonValueKind.True:
                    case JsonValueKind.False:
                        parameters.Add(param.GetBoolean());
                        break;
                    default:
                        Debug.LogError($"[ScriptInspector] Unsupported parameter type in JSON: {param}");
                        return null;
                }
            }
            return parameters.ToArray();
        }
        catch (Exception e)
        {
            Debug.LogError($"[ScriptInspector] Exception while parsing parameters JSON: {e.Message}. Value: {parametersJsonStr}");
            return null;
        }
    }

    internal static string? GetMethodsJson(object instance)
    {
        var type = instance.GetType();
        var methods = type.GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance)
            .Where(m => !m.IsSpecialName) // プロパティの getter/setter などの特殊メソッドを除外
            .Select(m => new MethodMeta(m))
            .ToList();
        var list = methods.Select(m => m.ToJson()).ToList();
        return JsonSerializer.Serialize(list);
    }

    internal static string? GetScriptMetaJson(object instance)
    {
        var type = instance.GetType();
        var fields = GetFields(instance);
        var methods = type.GetMethods(BindingFlags.Public | BindingFlags.Instance)
            .Where(m => !m.IsSpecialName) // プロパティの getter/setter などの特殊メソッドを除外
            .Select(m => new MethodMeta(m))
            .ToList();
        var scriptMeta = new Dictionary<string, object?>
        {
            { "TypeName", type.FullName },
            { "Fields", fields.Select(f => f.ToJson()).ToArray() },
            { "Methods", methods.Select(m => m.ToJson()).ToArray() }
        };
        return JsonSerializer.Serialize(scriptMeta, JsonOptions);
    }
}
