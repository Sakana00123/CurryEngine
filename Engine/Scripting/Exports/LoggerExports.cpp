#include "pch.h"
#include "Engine/Editor/Console.h"

ENGINE_API void Console_CustomLog(int level, const char* message, const char* file, int line)
{
	Console::CustomLog(static_cast<Console::LogLevel>(level), message, file, line);
}