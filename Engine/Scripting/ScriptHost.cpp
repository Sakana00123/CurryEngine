#include "pch.h"
#include "ScriptHost.h"
#include <Windows.h>
#include <stdexcept>
#include <cassert>

// hostfxrの関数ポインタ
static hostfxr_initialize_for_runtime_config_fn s_initFunc;
static hostfxr_get_runtime_delegate_fn s_getDelegateFunc;
static hostfxr_close_fn s_closeFunc;

bool ScriptHost::LoadHostFxr()
{
	char_t path[MAX_PATH];
	size_t pathSize = MAX_PATH;

	// .NETインストール済み場所からhostfxr.dllを自動検索
	int rc = get_hostfxr_path(path, &pathSize, nullptr);
	if (rc != 0)
	{
		std::wstring message = L"get_hostfxr_path failed with error code: " + std::to_wstring(rc);
		MessageBoxW(nullptr, message.c_str(), L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// hostfxr.dllをロード
	HMODULE lib = LoadLibraryW(path);
	if (!lib)
	{
		MessageBoxW(nullptr, L"Failed to load hostfxr.dll", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// hostfxrの関数ポインタを取得
	s_initFunc = (hostfxr_initialize_for_runtime_config_fn)
		(GetProcAddress(lib, "hostfxr_initialize_for_runtime_config"));
	s_getDelegateFunc = (hostfxr_get_runtime_delegate_fn)
		(GetProcAddress(lib, "hostfxr_get_runtime_delegate"));
	s_closeFunc = (hostfxr_close_fn)
		(GetProcAddress(lib, "hostfxr_close"));

	// 関数ポインタがすべて取得できたか確認
	return s_initFunc && s_getDelegateFunc && s_closeFunc;
}

bool ScriptHost::InitRuntime(const std::wstring& runtimeConfigPath)
{
	// .NETランタイムを初期化
    int rc = s_initFunc(runtimeConfigPath.c_str(), nullptr, &m_hostContext);
	if (rc != 0 || !m_hostContext)
	{
		std::wstring message = L"Failed to initialize .NET runtime. Error code: " + std::to_wstring(rc);
		MessageBoxW(nullptr, message.c_str(), L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// 関数ポインタを取得
	rc = s_getDelegateFunc(
		m_hostContext,
		hdt_load_assembly_and_get_function_pointer,
		(void**)&m_loadFunc);

	// 関数ポインタが取得できたか確認
	return rc == 0 && m_loadFunc != nullptr;
}

bool ScriptHost::Initialize()
{
	if (!LoadHostFxr())
		return false;

	wchar_t exePath[MAX_PATH];
	GetModuleFileNameW(nullptr, exePath, MAX_PATH);
	std::wstring exeDir = std::wstring(exePath).substr(
		0, std::wstring(exePath).rfind(L'\\') + 1);

	std::wstring configPath = exeDir + L"CurryEngine.Runtime.runtimeconfig.json";
	std::wstring engineRuntimeDll = exeDir + L"CurryEngine.Runtime.dll";

	//// ここで実際のパスを表示して確認
	//std::wstring debug = L"configPath:\n" + configPath
	//	+ L"\n\nexists: "
	//	+ (GetFileAttributesW(configPath.c_str())
	//		!= INVALID_FILE_ATTRIBUTES ? L"YES" : L"NO");
	//MessageBoxW(nullptr, debug.c_str(), L"Debug", MB_OK);


	if (!InitRuntime(configPath))
		return false;

	// ここで関数ポインタをロードするためのラムダ関数を定義
	auto load = [&]<typename TFunc>(TFunc& funcPtr, const std::wstring& dllPath, const wchar_t* typeName, const wchar_t* methodName)
	{
		int rc = m_loadFunc(
			dllPath.c_str(),
			typeName,
			methodName,
			UNMANAGEDCALLERSONLY_METHOD, // delegate_type_name
			nullptr, // reserved
			(void**)&funcPtr);
		// 取得できたか確認
		assert(rc == 0 && funcPtr != nullptr);
	};

	
	// エンジンランタイムの関数をロード
	const wchar_t* engineRuntimeType = L"CurryEngine.Runtime.EngineRuntime, CurryEngine.Runtime";
	load(m_initFunc, engineRuntimeDll, engineRuntimeType,
		L"EngineInitialize");
	load(m_callbacks.ReloadScripts, engineRuntimeDll, engineRuntimeType,
		L"ReloadScripts");
	load(m_updateFunc, engineRuntimeDll, engineRuntimeType,
		L"EngineUpdate");
	load(m_shutdownFunc, engineRuntimeDll, engineRuntimeType,
		L"EngineShutdown");
	load(m_callbacks.RegisterAllScriptMeta, engineRuntimeDll, engineRuntimeType,
		L"RegisterAllScriptMeta");

	// スクリプトブリッジの関数をロード
	const wchar_t* scriptBridgeType = L"CurryEngine.Runtime.Native.ScriptBridge, CurryEngine.Runtime";
	load(m_callbacks.CreateScript, engineRuntimeDll, scriptBridgeType,
		L"CreateScript");
	load(m_callbacks.ReleaseScript, engineRuntimeDll, scriptBridgeType,
		L"ReleaseScript");
	load(m_callbacks.AwakeScript, engineRuntimeDll, scriptBridgeType,
		L"AwakeScript");
	load(m_callbacks.StartScript, engineRuntimeDll, scriptBridgeType,
		L"StartScript");
	load(m_callbacks.UpdateScript, engineRuntimeDll, scriptBridgeType,
		L"UpdateScript");
	load(m_callbacks.OnEnableScript, engineRuntimeDll, scriptBridgeType,
		L"OnEnableScript");
	load(m_callbacks.OnDisableScript, engineRuntimeDll, scriptBridgeType,
		L"OnDisableScript");

	load(m_callbacks.OnDestroyScript, engineRuntimeDll, scriptBridgeType,
		L"OnDestroyScript");

	load(m_callbacks.HotSwapScript, engineRuntimeDll, scriptBridgeType,
		L"HotSwapScript");

	load(m_callbacks.GetScriptFields, engineRuntimeDll, scriptBridgeType,
		L"GetScriptFields");
	//load(m_callbacks.GetScriptField, engineRuntimeDll, scriptBridgeType,
	//	L"GetScriptField");
	load(m_callbacks.SetScriptField, engineRuntimeDll, scriptBridgeType,
		L"SetScriptField");


	// Physicsイベント用コールバックをロード

	load(m_callbacks.OnCollisionEnter, engineRuntimeDll, scriptBridgeType,
		L"OnCollisionEnterScript");
	load(m_callbacks.OnCollisionStay, engineRuntimeDll, scriptBridgeType,
		L"OnCollisionStayScript");
	load(m_callbacks.OnCollisionExit, engineRuntimeDll, scriptBridgeType,
		L"OnCollisionExitScript");

	load(m_callbacks.OnTriggerEnter, engineRuntimeDll, scriptBridgeType,
		L"OnTriggerEnterScript");
	load(m_callbacks.OnTriggerStay, engineRuntimeDll, scriptBridgeType,
		L"OnTriggerStayScript");
	load(m_callbacks.OnTriggerExit, engineRuntimeDll, scriptBridgeType,
		L"OnTriggerExitScript");


	// 初期化関数を呼び出し
	m_initFunc();
	return true;
}

void ScriptHost::Update()
{
	// 更新関数を呼び出し
	if (m_updateFunc)
		m_updateFunc();
}

void ScriptHost::Shutdown()
{
	// 終了関数を呼び出し
	if (m_shutdownFunc)
		m_shutdownFunc();

	// hostfxrの終了関数を呼び出し
	if (m_hostContext)
	{
		s_closeFunc(m_hostContext);
		m_hostContext = nullptr;
	}
}
