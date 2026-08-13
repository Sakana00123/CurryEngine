#pragma once
#include <functional>
#include <json.hpp>
using namespace nlohmann;

using SerializeFunc = std::function<void(const void*, json&)>;
using DeserializeFunc = std::function<void(void*, const json&)>;

// 型のシリアライザ情報
struct TypeSerializerInfo
{
	SerializeFunc serialize;
	DeserializeFunc deserialize;
};

// 型シリアライザのレジストリ
class TypeSerializerRegistry
{
public:
	// 型とシリアライザ情報を登録
	static void Register(
		const std::string& type,
		const TypeSerializerInfo& info);

	// 型に対応するシリアライザ情報を取得
	static const TypeSerializerInfo* Find(const std::string& type);

private:
	// シリアライザ情報のレジストリを取得
	static std::unordered_map<std::string, TypeSerializerInfo>& GetRegistry() {
		static std::unordered_map<std::string, TypeSerializerInfo> registry;
		return registry;
	}
};

#define C_CONCAT_IMPL(a, b) a##b
#define C_CONCAT(a, b) C_CONCAT_IMPL(a, b)

#define C_REGISTER_TYPE_IMPL(Type, N) \
	namespace { \
	struct C_CONCAT(TypeAutoRegister_, N) { \
		C_CONCAT(TypeAutoRegister_, N)() { \
			TypeSerializerInfo info; \
			info.serialize = [](const void* obj, json& j) { \
				j = *static_cast<const Type*>(obj); \
			}; \
			info.deserialize = [](void* obj, const json& j) { \
				*static_cast<Type*>(obj) = j.get<Type>(); \
			}; \
			TypeSerializerRegistry::Register(#Type, info); \
		} \
	}; \
	static C_CONCAT(TypeAutoRegister_, N) C_CONCAT(_instance_, N); \
	}


// 型のシリアライザを自動登録するマクロ
#define C_REGISTER_TYPE(Type) \
	C_REGISTER_TYPE_IMPL(Type, __COUNTER__)

// プリミティブ型のシリアライザを登録
#ifndef C_REGISTER_PRIMITIVE_TYPES
#define C_REGISTER_PRIMITIVE_TYPES
C_REGISTER_TYPE(int)
C_REGISTER_TYPE(float)
C_REGISTER_TYPE(bool)
C_REGISTER_TYPE(std::string)
#endif // C_REGISTER_PRIMITIVE_TYPES
