#include "pch.h"
#include "PropertyDrawContext.h"

PropertyDrawContext PropertyDrawContext::MakeSingle(Component* target)
{
	PropertyDrawContext context;
	context.targets = { target };
	context.isMultiSelect = false;
	return context;
}

PropertyDrawContext PropertyDrawContext::MakeMulti(const std::vector<Component*>& targets)
{
	PropertyDrawContext context;
	context.targets = targets;
	context.isMultiSelect = targets.size() > 1;
	return context;
}

Component* PropertyDrawContext::Primary() const
{
	return targets.empty() ? nullptr : targets[0];
}

bool PropertyDrawContext::IsEmpty() const
{
	return targets.empty();
}