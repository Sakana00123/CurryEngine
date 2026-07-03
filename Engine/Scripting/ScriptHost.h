#pragma once
#include <string>
#include <functional>
#include <nethost.h>
#include <hostfxr.h>
#include <coreclr_delegates.h>
#include "Engine/Scripting/Exports/ScriptBridgeTypes.h"

// C++から呼ぶC#メソッドの関数ポインタ型
using ManagedUpdateFunc = void(__stdcall*)(void* gcHandle);
using ManagedAwakeFunc = void(__stdcall*)(void* gcHandle);
using ManagedStartFunc = void(__stdcall*)(void* gcHandle);
using ManagedDestroyFunc = void(__stdcall*)(void* gcHandle);
using ManagedCreateFunc = void* (__stdcall*)(const char* typeName, uint64_t ownerId, uint64_t componentId);
using ManagedReleaseFunc = void(__stdcall*)(void* gcHandle);
using ManagedReloadFunc = void(__stdcall*)(void* gcHandle, uint64_t ownerId, uint64_t componentIds);
using ManagedScriptCallbackFunc = void(__stdcall*)(void* gcHandle);
using ManagedCollisionCallbackFunc = void(__stdcall*)(void* gcHandle, CollisionInfoDto* info);
using ManagedTriggerCallbackFunc = void(__stdcall*)(void* gcHandle, TriggerInfoDto* info);
using ManagedGetFieldsJsonFunc = void* (__stdcall*)(void* gcHandle);
using ManagedGetFieldFunc = void* (__stdcall*)(void* gcHandle, const char* fieldName);
using ManagedSetFieldFunc = void(__stdcall*)(void* gcHandle, const char* fieldName, const char* value);
using ManagedRegisterAllScriptMetaFunc = void(__stdcall*)(RegisterScriptClassFunc callback);
//using ManagedGetComponentFunc = void* (__stdcall*)(void* gcHandle, const char* typeName);
using VoidFunc = void(__stdcall*)();
using ReloadScriptsFunc = void(__stdcall*)(const char* assemblyPath);

struct ScriptCallbacks
{
	ManagedUpdateFunc UpdateScript = nullptr;
	ManagedAwakeFunc AwakeScript = nullptr;
	ManagedStartFunc StartScript = nullptr;
	ManagedDestroyFunc OnDestroyScript = nullptr;
	ManagedCreateFunc CreateScript = nullptr;

	ManagedScriptCallbackFunc OnEnableScript = nullptr;
	ManagedScriptCallbackFunc OnDisableScript = nullptr;

	ManagedReleaseFunc ReleaseScript = nullptr;
	ManagedReloadFunc HotSwapScript = nullptr;
	ManagedGetFieldsJsonFunc GetScriptFields = nullptr;
	//ManagedGetFieldFunc GetScriptField = nullptr;
	ManagedSetFieldFunc SetScriptField = nullptr;
	ManagedRegisterAllScriptMetaFunc RegisterAllScriptMeta = nullptr;

	ReloadScriptsFunc ReloadScripts = nullptr;


	// ------------ Physicsイベント用コールバック ------------
	ManagedCollisionCallbackFunc OnCollisionEnter = nullptr;
	ManagedCollisionCallbackFunc OnCollisionStay = nullptr;
	ManagedCollisionCallbackFunc OnCollisionExit = nullptr;
	ManagedTriggerCallbackFunc OnTriggerEnter = nullptr;
	ManagedTriggerCallbackFunc OnTriggerStay = nullptr;
	ManagedTriggerCallbackFunc OnTriggerExit = nullptr;




};

// .NETホストの初期化に必要な情報を表す構造体
struct ScriptHostDesc
{
	std::wstring runtimeConfigPath; // .NETランタイムのパス
	std::wstring engineApiPath; // エンジンのAPIを定義したDLLのパス
	get_hostfxr_parameters parameters; // hostfxrの初期化に必要なパラメータ(内部で使用)
};

struct ScriptHostConfig
{
	ScriptCallbacks callbacks; // C#から呼び出す関数のコールバック
};

// C#アセンブリ内の関数を表す構造体
struct FunctionDesc
{
	std::wstring assemblyPath; // C#アセンブリのパス
	std::wstring typeName;     // C#クラスの完全修飾名（例: "MyNamespace.MyClass"）
	std::wstring methodName;   // C#クラス内のメソッド名（例: "MyMethod"）
};

// .NET Core ホストクラス
class ScriptHost
{
public:

	bool Initialize();
	void Shutdown();

	const ScriptCallbacks& GetCallbacks() { return m_callbacks; }

	// C#アセンブリから関数ポインタを取得するための関数
	template<typename TFunc>
	TFunc GetFunction(const FunctionDesc& desc)
	{
		if (!m_loadFunc)
			return nullptr;
		TFunc funcPtr = nullptr;
		int result = m_loadFunc(
			desc.assemblyPath.c_str(),
			desc.typeName.c_str(),
			desc.methodName.c_str(),
			nullptr, // delegate_type_name
			nullptr, // reserved
			(void**)&funcPtr);
		return (result == 0) ? funcPtr : nullptr;
	}

private:

	bool LoadHostFxr();

	bool InitRuntime(const std::wstring& runtimeConfigPath);

private:
	VoidFunc m_initFunc = nullptr;
	VoidFunc m_shutdownFunc = nullptr;
	ScriptCallbacks m_callbacks;
	// hostfxrの関数ポインタ
	hostfxr_handle m_hostContext = nullptr;
	load_assembly_and_get_function_pointer_fn m_loadFunc = nullptr;
};

