// Test-only native logging/registration endpoints used by the real managed runtime.
extern "C" __declspec(dllexport) void ScriptNames_Clear() {}
extern "C" __declspec(dllexport) void ScriptNames_Add(const char*) {}
extern "C" __declspec(dllexport) void Console_CustomLog(int, const char*, const char*, int) {}
