#include "pch.h"
#include "Renderer.h"

void Renderer::DrawProperty()
{
#ifdef USE_IMGUI

	Component::DrawProperty();

	if (material)
	{
		material->DrawProperty();
	}
#endif // USE_IMGUI
}

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