#include "pch.h"
#include "Renderer.h"

#ifdef USE_IMGUI
void Renderer::DrawProperty(const PropertyDrawContext& context)
{

	Component::DrawProperty(context);

	if (material)
	{
		material->DrawProperty();
	}
}
#endif // USE_IMGUI

json Renderer::Serialize() const
{
	json j;
	if (material)
	{
		j["material"] = material->Serialize();
	}
	return j;
}

void Renderer::Deserialize(const json& j)
{
	if (j.contains("material"))
	{
		material = std::make_shared<Material>();
		material->Deserialize(j["material"]);
	}
}