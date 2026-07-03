#pragma once
#include "ObjectId.h"
#include <memory>
#include <stdexcept>
#include <type_traits>
#include "ObjectManager.h"
#include "GameObject.h"
#include "Component.h"

/**
 * @brief オブジェクト参照を表すテンプレート構造体。
 * @tparam T 参照するオブジェクトの型。GameObject または Component を想定。
 */
template<typename T>
struct Reference
{
	ObjectId id = ObjectId::Invalid();

	// 参照先のオブジェクトが有効かどうかをチェックする
	bool IsValid() const
	{
		return id.IsValid();
	}
	bool operator==(const Reference& other) const
	{
		return id == other.id;
	}
	bool operator!=(const Reference& other) const
	{
		return !(*this == other);
	}

	T* operator->() const { return resolve(); }

	T& operator*() const { return *resolve(); }

	operator T* () const { return resolve(); }
	explicit operator bool() const { return resolve() != nullptr; }

	// ポインタから直接代入するためのオーバーロード。これにより、オブジェクトのポインタを直接参照に代入できるようになる。
	Reference& operator=(T* ptr)
	{
		id = ptr ? ptr->GetId() : ObjectId::Invalid();
		return *this;
	}
	// ObjectIdから直接代入するためのオーバーロード。これにより、ObjectIdを直接参照に代入できるようになる。
	Reference& operator=(ObjectId newId)
	{
		id = newId;
		return *this;
	}
private:

	T* resolve() const
	{
		if (!id.IsValid()) return nullptr;

		if constexpr (std::is_same_v<T, Component>)
		{
			return dynamic_cast<T*>(ObjectManager::FindComponent(id).get());
		}
		else if constexpr (std::is_same_v<T, GameObject>)
		{
			return dynamic_cast<T*>(ObjectManager::Find(id));
		}
		else
		{
			static_assert(false, "Reference<T>: T must be GameObject or Component-derived");
		}

		return nullptr;
	}
};

Reference<GameObject> FindGameObject(const std::string& name) { return Reference<GameObject>(ObjectManager::Find(name)->GetId()); }
Reference<GameObject> FindGameObject(const ObjectId& id) { return Reference<GameObject>(ObjectManager::Find(id)->GetId()); }
Reference<Component> FindComponent(const ObjectId& id) { return Reference<Component>(ObjectManager::FindComponent(id)->GetId()); }

// 型トレイトメタプログラミング：Reference<T>が参照型かどうかを判定するためのis_reference_type構造体
// デフォルトではfalse_typeを継承し、Reference<T>に対してはtrue_typeを継承する特殊化を定義する。

template<typename T>
struct is_reference_type : std::false_type {};

template<typename T>
struct is_reference_type<Reference<T>> : std::true_type {};

// inner_type取得
template<typename T>
struct reference_inner_type {};

template<typename T>
struct reference_inner_type<Reference<T>> {
	using type = T;
};