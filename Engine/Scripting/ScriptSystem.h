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

	/** @brief スクリプトシステムの更新処理。*/
	static void Update();

	/** @brief スクリプトシステムの終了処理。*/
	static void Shutdown();

	/** @brief スクリプトシステムのリロード。*/
	static void Reload();

	/* ユーザースクリプトのビルドを要求するための関数。成功するとリロードもされる。*/
	static void RequestScriptBuildAndReload();

	// ----- ScriptComponent から呼ぶAPI -----


	/**
	 * @brief 新しい要素を生成します。
	 */
	static void* CreateScript(const std::string& typeName, uint64_t ownerId, uint64_t componentId);
	/**
	 * @brief ReleaseScript の処理を行います。
	 */
	static void ReleaseScript(void* gcHandle);
	/**
	 * @brief AwakeScript の処理を行います。
	 */
	static void AwakeScript(void* gcHandle);
	/**
	 * @brief StartScript の処理を行います。
	 */
	static void StartScript(void* gcHandle);
	/**
	 * @brief 状態を更新します。
	 */
	static void UpdateScript(void* gcHandle);
	/**
	 * @brief DestroyScript イベントを処理します。
	 */
	static void OnDestroyScript(void* gcHandle);

	/**
	 * @brief EnableScript イベントを処理します。
	 */
	static void OnEnableScript(void* gcHandle);
	/**
	 * @brief DisableScript イベントを処理します。
	 */
	static void OnDisableScript(void* gcHandle);

	/**
	 * @brief HotSwapScript の処理を行います。
	 */
	static void* HotSwapScript(void* gcHandle, uint64_t ownerId, uint64_t componentId);
	/**
	 * @brief GetScriptFields に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	static void* GetScriptFields(void* gcHandle);
	/**
	 * @brief ScriptField を設定します。
	 */
	static void SetScriptField(void* gcHandle, const std::string& fieldName, const std::string& value);
	static void CallScriptMethod(void* gcHandle, const MethodInfo* info, std::vector<std::any> args = {});
	/**
	 * @brief GetScriptMethods に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	static std::vector<MethodInfo> GetScriptMethods(void* gcHandle);


	/**
	 * @brief CollisionEnterScript イベントを処理します。
	 */
	static void OnCollisionEnterScript(void* gcHandle, const CollisionInfo& info);
	/**
	 * @brief CollisionStayScript イベントを処理します。
	 */
	static void OnCollisionStayScript(void* gcHandle, const CollisionInfo& info);
	/**
	 * @brief CollisionExitScript イベントを処理します。
	 */
	static void OnCollisionExitScript(void* gcHandle, const CollisionInfo& info);

	/**
	 * @brief TriggerEnterScript イベントを処理します。
	 */
	static void OnTriggerEnterScript(void* gcHandle, const TriggerInfo& info);
	/**
	 * @brief TriggerStayScript イベントを処理します。
	 */
	static void OnTriggerStayScript(void* gcHandle, const TriggerInfo& info);
	/**
	 * @brief TriggerExitScript イベントを処理します。
	 */
	static void OnTriggerExitScript(void* gcHandle, const TriggerInfo& info);


	// ----- その他のAPI -----

	/**
	 * @brief GetRegisteredScriptNames に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	static std::vector<std::string> GetRegisteredScriptNames();

	/**
	 * @brief 保持している内容を消去します。
	 */
	static void ClearScriptNames();

	/**
	 * @brief AddTempScriptName の処理を行います。
	 */
	static void AddTempScriptName(const std::string& name);

	/**
	 * @brief GetScriptMeta に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	static void* GetScriptMeta(const std::string& scriptName);

private:
	// スクリプトホストのインスタンスへのポインタ
	static inline ScriptHost* s_scriptHost = nullptr;

	static inline ScriptWatcher* s_scriptWatcher = nullptr; // スクリプトファイルの監視とリロードを担当するインスタンス

	// 登録されているスクリプトの名前のキャッシュ（GetRegisteredScriptNamesの呼び出しごとにホストから取得して更新する）
	static inline std::vector<std::string> s_tempNames;
};
