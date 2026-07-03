#include "pch.h"
#include "TypeSerializerRegistry.h"

C_REGISTER_TYPE(int);
C_REGISTER_TYPE(float);
C_REGISTER_TYPE(bool);
C_REGISTER_TYPE(std::string);


// 登録
void TypeSerializerRegistry::Register(
	const std::string& type,
	const TypeSerializerInfo& info)
{
	GetRegistry()[type] = info;
}

const TypeSerializerInfo* TypeSerializerRegistry::Find(const std::string& type) {
	auto& registry = GetRegistry();
	auto it = registry.find(type);
	if (it != registry.end()) {
		return &it->second;
	}
	return nullptr;
}