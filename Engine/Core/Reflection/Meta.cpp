#include "pch.h"
#include "Meta.h"
#include "Engine/Editor/Console.h"

std::any MethodInfo::Invoke(void* instance, std::vector<std::any> args) const
{
	if (invoker)
	{
		return invoker(instance, args);
	}
	LOG_ERROR("Method does not return a value or invoker is not set.");
	return std::any(); // 戻り値なし（void）や invoker が未設定の場合は空の any を返す
}

void MethodInfo::InvokeVoid(void* instance, std::vector<std::any> args) const
{
	if (invoker)
	{
		invoker(instance, args);
	}
	else
	{
		LOG_ERROR("Method does not return a value or invoker is not set.");
	}
}

const PropertyInfo* ClassMeta::FindProperty(const std::string& propName) const
{
	// クラス自身のプロパティを検索
	for (const auto& prop : properties)
	{
		if (prop.name == propName)
		{
			return &prop;
		}
	}
	// 基底クラスを再帰的に検索
	for (const auto& baseName : bases)
	{
		const ClassMeta* baseMeta = ReflectionRegistry::FindClass(baseName);
		if (baseMeta)
		{
			const PropertyInfo* prop = baseMeta->FindProperty(propName);
			if (prop)
			{
				return prop;
			}
		}
	}
	return nullptr; // 見つからなかった場合
}

const MethodInfo* ClassMeta::FindMethod(const std::string& methodName) const
{
	// クラス自身のメソッドを検索
	for (const auto& method : methods)
	{
		if (method.name == methodName)
		{
			return &method;
		}
	}
	// 基底クラスを再帰的に検索
	for (const auto& baseName : bases)
	{
		const ClassMeta* baseMeta = ReflectionRegistry::FindClass(baseName);
		if (baseMeta)
		{
			const MethodInfo* method = baseMeta->FindMethod(methodName);
			if (method)
			{
				return method;
			}
		}
	}
	return nullptr; // 見つからなかった場合
}

void ReflectionRegistry::Register(const ClassMeta& meta)
{
	GetClassRegistry()[meta.name] = meta;
	LOG_INFO("class: " + meta.name + ", fields: " + std::to_string(meta.properties.size()) + ", methods: " + std::to_string(meta.methods.size()));
}

void ReflectionRegistry::RegisterEnum(const EnumInfo& meta)
{
	GetEnumRegistry()[meta.name] = meta;
	LOG_INFO("enum: " + meta.name + ", values: " + std::to_string(meta.values.size()));
}

void ReflectionRegistry::RegisterStruct(const StructInfo& meta)
{
	GetStructRegistry()[meta.name] = meta;
	LOG_INFO("struct: " + meta.name + ", fields: " + std::to_string(meta.properties.size()));
}

const ClassMeta* ReflectionRegistry::FindClass(const std::string& name)
{
	auto it = GetClassRegistry().find(name);
	if (it != GetClassRegistry().end())
	{
		return &it->second;
	}
	return nullptr;
}

const EnumInfo* ReflectionRegistry::FindEnum(const std::string& name)
{
	auto it = GetEnumRegistry().find(name);
	if (it != GetEnumRegistry().end())
	{
		return &it->second;
	}
	return nullptr;
}

const StructInfo* ReflectionRegistry::FindStruct(const std::string& name)
{
	auto it = GetStructRegistry().find(name);
	if (it != GetStructRegistry().end())
	{
		return &it->second;
	}
	return nullptr;
}

void ReflectionRegistry::UnregisterScriptClasses()
{
	auto& registry = GetClassRegistry();
	for (auto it = registry.begin(); it != registry.end(); )
	{
		if (it->second.isScript)
		{
			it = registry.erase(it);
		}
		else
		{
			++it;
		}
	}
}

std::unordered_map<std::string, ClassMeta>& ReflectionRegistry::GetClassRegistry()
{
	static std::unordered_map<std::string, ClassMeta> registry;
	return registry;
}

std::unordered_map<std::string, EnumInfo>& ReflectionRegistry::GetEnumRegistry()
{
	static std::unordered_map<std::string, EnumInfo> registry;
	return registry;
}

std::unordered_map<std::string, StructInfo>& ReflectionRegistry::GetStructRegistry()
{
	static std::unordered_map<std::string, StructInfo> registry;
	return registry;
}