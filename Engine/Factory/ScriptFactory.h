#pragma once
#include <unordered_map>
#include <string>
/**
 * @file
 * @brief スクリプトの管理を行うファクトリクラス。
 */
class ScriptFactory
{
public:
	/** @brief 登録されているスクリプトをすべてクリアします。*/
	void Clear() { registry.clear(); }
	/** @brief スクリプトを登録します。*/
	void Register(const char* name);

	/** @brief 登録されているすべてのスクリプト名を取得します。*/
	std::vector<std::string> GetRegisteredScriptNames() const;
	
private:
	std::vector<std::string> registry;
};

// グローバルなスクリプトファクトリインスタンス
ScriptFactory& GetScriptFactory();