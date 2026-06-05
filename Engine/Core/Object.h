#pragma once

#include "Engine/Core/Reflection/Meta.h"
#include "Engine/Core/ObjectId.h"

#include <string>
#include <typeindex>
#include <json.hpp>
using json = nlohmann::json;

/**
 * @file
 * @brief 全てのオブジェクトの共通基底クラス。
 * @details 一意識別子、名前、優先度、シリアライズ/デシリアライズ機能を提供します。
 */
class Object
{
	C_REFLECT(Object)
public:
	/** @brief 一意識別子。*/
	ObjectId id = ObjectId::Generate(); // オブジェクト生成時に一意なIDを割り当て
	/** @brief オブジェクト名。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::HideInInspector)
	std::string name;
	/** @brief 更新や描画の優先度。数値が小さいほど先に処理されることを想定。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::ReadOnly)
	int priority = 0;
	
public:
	//static void Destroy(Object* obj) {}

	// シリアライズ
	virtual json Serialize() const { return json::object(); }

	// デシリアライズ
	virtual void Deserialize(const json& j) {}

	/** @brief 名前を設定します。*/
	virtual void SetName(const std::string& newName) { name = newName; }

	/** @brief オブジェクトの文字列表現を取得します。デフォルトでは名前を返します。*/
	virtual std::string ToString() const { return name; }

	/** @brief ID を設定します。通常は使用しませんが、特定のIDを直接指定したい場合に便利です。*/
	void SetId(const ObjectId& newId) { id = newId; }

	/** @brief 自身の ID を取得します。*/
	ObjectId GetId() const { return id; }

	/** @brief 優先度を取得します。*/
	int GetPriority() const { return priority; }

	/** @brief 優先度を設定します。*/
	void SetPriority(int p) { priority = p; }

	/** @brief 名前を取得します。*/
	std::string GetName() const { return name; }

	/** @brief オブジェクトの型名を取得します。デフォルトではクラス名を返します。*/
	virtual std::string GetTypeName() const { return name; }

	/** @brief クラスメタデータを取得します。*/
	const ClassMeta* GetClassMeta() const
	{
		return ReflectionRegistry::FindClass(GetTypeName());
	}

	/** @brief プロパティのアドレスを取得します。プロパティが見つからない場合は nullptr を返します。*/
	char* GetPropertyAddress(const std::string& propertyName)
	{
		if (auto* meta = GetClassMeta())
		{
			for (const auto& prop : meta->properties)
			{
				if (prop.name == propertyName)
				{
					return reinterpret_cast<char*>(this) + prop.offset;
				}
			}
		}
		return nullptr; // プロパティが見つからない場合は nullptr を返す
	}
};