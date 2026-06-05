#include "pch.h"
#include "ScriptFactory.h"


void ScriptFactory::Register(const char* name)
{
	registry.push_back(name);
}

std::vector<std::string> ScriptFactory::GetRegisteredScriptNames() const
{
	return registry;
}

ScriptFactory& GetScriptFactory()
{
	static ScriptFactory g_Factory;
	return g_Factory;
}