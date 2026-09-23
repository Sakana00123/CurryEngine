using System.Reflection;

namespace CurryEngine.Runtime.Reflection
{
    public class MethodMeta
    {
        public string Name { get; }
        public string ReturnTypeName { get; }
        public ParameterMeta[] Parameters { get; }
        public MethodMeta(MethodInfo methodInfo)
        {
            Name = methodInfo.Name;
            ReturnTypeName = ToTypeName(methodInfo.ReturnType);
            Parameters = methodInfo.GetParameters().Select(p => new ParameterMeta(p)).ToArray();
        }
        private static string ToTypeName(Type type)
        {
            if (type.IsGenericType)
            {
                var genericTypeName = type.GetGenericTypeDefinition().FullName;
                var genericArgs = string.Join(", ", type.GetGenericArguments().Select(ToTypeName));
                return $"{genericTypeName.Substring(0, genericTypeName.IndexOf('`'))}<{genericArgs}>";
            }
            return type.FullName ?? type.Name;
        }

        public override string ToString()
        {
            var parameters = string.Join(", ", Parameters.Select(p => $"{p.TypeName} {p.Name}"));
            return $"{ReturnTypeName} {Name}({parameters})";
        }

        public Dictionary<string, object?> ToJson()
        {
            var json = new Dictionary<string, object?>
            {
                { "Name", Name },
                { "ReturnTypeName", ReturnTypeName },
                { "Parameters", Parameters.Select(p => p.ToJson()).ToArray() }
            };
            return json;
        }
    }
}
