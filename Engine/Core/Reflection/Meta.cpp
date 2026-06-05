#include "pch.h"
#include "Meta.h"
#include "Engine/Editor/Console.h"

std::any MethodInfo::Invoke(void* instance, std::vector<std::any> args) const
{
	if (invoker)
	{
		return invoker(instance, args);
	}
	Console::LogError("Method does not return a value or invoker is not set.");
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
		Console::LogError("Method does not return a value or invoker is not set.");
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
	GetRegistry()[meta.name] = meta;
	Console::Log("class: " + meta.name + ", fields: " + std::to_string(meta.properties.size()) + ", methods: " + std::to_string(meta.methods.size()));
}

const ClassMeta* ReflectionRegistry::FindClass(const std::string& name)
{
	auto it = GetRegistry().find(name);
	if (it != GetRegistry().end())
	{
		return &it->second;
	}
	return nullptr;
}

void ReflectionRegistry::UnregisterScriptClasses()
{
	auto& registry = GetRegistry();
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

std::unordered_map<std::string, ClassMeta>& ReflectionRegistry::GetRegistry()
{
	static std::unordered_map<std::string, ClassMeta> registry;
	return registry;
}