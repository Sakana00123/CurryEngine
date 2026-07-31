#include "pch.h"
#include "ComponentFactory.h"
#include "Engine/Core/Component.h"
#include "Engine/Editor/Console.h"

void ComponentFactory::Register(
	const std::string& name,
	const std::string& category,
	std::function<std::shared_ptr<Component>()> func,
	unsigned int attributes,
	std::vector<std::string> requireComponents
)
{
	Registry()[name] = { category, std::move(func), attributes, std::move(requireComponents) };
}

std::shared_ptr<Component> ComponentFactory::Create(const std::string& name)
{
	if (Registry().find(name) == Registry().end()) {
		LOG_ERROR("ComponentFactory: Unknown component type: " + name);
		return nullptr;
	}
	auto& entry = Registry().at(name);
	auto component = entry.createFunc();
	component->SetAttributeFlags(entry.attributes);
	return component;
}

bool ComponentFactory::Exists(const std::string& name)
{
	return Registry().find(name) != Registry().end();
}

std::unordered_map<std::string, ComponentFactory::Entry>& ComponentFactory::GetAll()
{
	return Registry();
}

std::unordered_map<std::string, ComponentFactory::Entry>& ComponentFactory::Registry()
{
	static std::unordered_map<std::string, Entry> registry;
	return registry;
}