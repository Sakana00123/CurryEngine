#pragma once
#include <json.hpp>
using json = nlohmann::json;

/**
 * @file
 * @brief エディタの設定インターフェース。
 * @details エディタの各種設定を管理するためのインターフェースクラスです。
 *          デフォルト設定へのリセット、シリアライズ/デシリアライズ機能を提供します。
 */
struct IEditorConfig
{
	/* @brief デフォルト設定にリセットします。*/
	virtual void ResetToDefault() = 0;

	/** @brief 設定を保存します。*/
	virtual json Serialize() const = 0;

	/** @brief 設定を読み込みます。*/
	virtual void Deserialize(const json& j) = 0;
};