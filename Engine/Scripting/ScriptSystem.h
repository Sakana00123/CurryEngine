#pragma once
#include <string>
#include "Engine/Physics/CollisionEvent.h"

class ScriptHost;
class ScriptWatcher;

/**
 * @file
 * @brief スクリプトシステムの管理クラス。
 * @details スクリプトシステムの初期化、リロード、終了処理を行います。
 */
class ScriptSystem
{
public:
	/** @brief スクリプトシステムの初期化。*/
	static void Initialize();

	/** @brief スクリプトシステムの終了処理。*/
	static void Shutdown();

	/** @brief スクリプトシステムのリロード。*/
	static void Reload();

	/* ユーザースクリプトのビルドを要求するための関数。成功するとリロードもされる。*/
	static void RequestScriptBuildAndReload();

	// ----- ScriptComponent から呼ぶAPI -----


	static void* CreateScript(const std::string& typeName, uint64_t ownerId, uint64_t componentId);
	static void ReleaseScript(void* gcHandle);
	static void AwakeScript(void* gcHandle);
	static void StartScript(void* gcHandle);
	static void UpdateScript(void* gcHandle);
	static void OnDestroyScript(void* gcHandle);

	static void OnEnableScript(void* gcHandle);
	static void OnDisableScript(void* gcHandle);

	static void HotSwapScript(void* gcHandle, uint64_t ownerId, uint64_t componentId);
	static void* GetScriptFields(void* gcHandle);
	static void SetScriptField(void* gcHandle, const std::string& fieldName, const std::string& value);


	static void OnCollisionEnterScript(void* gcHandle, const CollisionInfo& info);
	static void OnCollisionStayScript(void* gcHandle, const CollisionInfo& info);
	static void OnCollisionExitScript(void* gcHandle, const CollisionInfo& info);

	static void OnTriggerEnterScript(void* gcHandle, const TriggerInfo& info);
	static void OnTriggerStayScript(void* gcHandle, const TriggerInfo& info);
	static void OnTriggerExitScript(void* gcHandle, const TriggerInfo& info);


	// ----- その他のAPI -----

	static std::vector<std::string> GetRegisteredScriptNames();

	static void ClearScriptNames();

	static void AddTempScriptName(const std::string& name);

private:
	// スクリプトホストのインスタンスへのポインタ
	static inline ScriptHost* s_scriptHost = nullptr;

	static inline ScriptWatcher* s_scriptWatcher = nullptr; // スクリプトファイルの監視とリロードを担当するインスタンス

	// 登録されているスクリプトの名前のキャッシュ（GetRegisteredScriptNamesの呼び出しごとにホストから取得して更新する）
	static inline std::vector<std::string> s_tempNames;
};
