#include "pch.h"
#include "Engine/Scripting/ScriptSystem.h"
// C#‚©‚çŒÄ‚Ño‚·ŠÖ”‚ÌÀ‘•

ENGINE_API void ScriptNames_Add(const char* name)
{
	ScriptSystem::AddTempScriptName(name);
}

ENGINE_API void ScriptNames_Clear()
{
	ScriptSystem::ClearScriptNames();
}