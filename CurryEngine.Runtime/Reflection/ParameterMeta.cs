using System;
using System.Reflection;

namespace CurryEngine.Runtime.Reflection
{
    public class ParameterMeta
    {
        public string Name { get; }
        public string TypeName { get; }
        public bool IsOptional { get; }
        public object? DefaultValue { get; }
        public ParameterMeta(ParameterInfo parameterInfo)
        {
            Name = parameterInfo.Name ?? throw new ArgumentNullException(nameof(parameterInfo.Name));
            TypeName = ToTypeName(parameterInfo.ParameterType);
            IsOptional = parameterInfo.IsOptional;
            DefaultValue = parameterInfo.DefaultValue;
        }
        private static string ToTypeName(Type type)
        {
            if (type.IsGenericType)
            {
                var genericTypeName = type.GetGenericTypeDefinition().FullName ?? throw new InvalidOperationException("Generic type definition does not have a full name.");
                var genericArgs = string.Join(", ", type.GetGenericArguments().Select(ToTypeName));
                return $"{genericTypeName.Substring(0, genericTypeName.IndexOf('`'))}<{genericArgs}>";
            }
            return type.FullName ?? type.Name;
        }

        public override string ToString()
        {
                return $"{TypeName} {Name}" + (IsOptional ? $" = {DefaultValue}" : "");
        }

        public Dictionary<string, object?> ToJson()
        {
            return new Dictionary<string, object?>
            {
                { "Name", Name },
                { "TypeName", TypeName },
                { "IsOptional", IsOptional },
                { "DefaultValue", DefaultValue }
            };
        }

    }
}
