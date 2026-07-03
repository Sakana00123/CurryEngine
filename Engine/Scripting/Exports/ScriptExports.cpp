#include "pch.h"
#include "Engine/Scripting/ScriptSystem.h"
// C#から呼び出す関数の実装

ENGINE_API void ScriptNames_Add(const char* name)
{
	ScriptSystem::AddTempScriptName(name);
}

ENGINE_API void ScriptNames_Clear()
{
	ScriptSystem::ClearScriptNames();
}