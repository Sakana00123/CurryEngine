#include "pch.h"
#include "SceneViewConfig.h"

void SceneViewConfig::ResetToDefault()
{
	guizmoPivotMode = 1;
	translationSnap = { .enabled{true}, .snapAllAxes{false}, .snapValue{1,1,1} };
	rotationSnap = { .enabled{true}, .snapAllAxes{false}, .snapValue{15,15,15} };
	scaleSnap = { .enabled{true}, .snapAllAxes{true}, .snapValue{0.1f,0.1f,0.1f} };
}

nlohmann::json SceneViewConfig::Serialize() const
{
	nlohmann::json j;
	j["guizmoPivotMode"] = guizmoPivotMode;
	j["translationSnap"]["enabled"] = translationSnap.enabled;
	j["translationSnap"]["snapAllAxes"] = translationSnap.snapAllAxes;
	j["translationSnap"]["snapValue"] = { translationSnap.snapValue.x, translationSnap.snapValue.y, translationSnap.snapValue.z };
	j["rotationSnap"]["enabled"] = rotationSnap.enabled;
	j["rotationSnap"]["snapAllAxes"] = rotationSnap.snapAllAxes;
	j["rotationSnap"]["snapValue"] = { rotationSnap.snapValue.x, rotationSnap.snapValue.y, rotationSnap.snapValue.z };
	j["scaleSnap"]["enabled"] = scaleSnap.enabled;
	j["scaleSnap"]["snapAllAxes"] = scaleSnap.snapAllAxes;
	j["scaleSnap"]["snapValue"] = { scaleSnap.snapValue.x, scaleSnap.snapValue.y, scaleSnap.snapValue.z };
	return j;
}

void SceneViewConfig::Deserialize(const nlohmann::json& j)
{
	if (j.contains("guizmoPivotMode")) {
		guizmoPivotMode = j["guizmoPivotMode"].get<int>();
	}
	if (j.contains("translationSnap")) {
		const auto& ts = j["translationSnap"];
		if (ts.contains("enabled")) {
			translationSnap.enabled = ts["enabled"].get<bool>();
		}
		if (ts.contains("snapAllAxes")) {
			translationSnap.snapAllAxes = ts["snapAllAxes"].get<bool>();
		}
		if (ts.contains("snapValue") && ts["snapValue"].is_array() && ts["snapValue"].size() == 3) {
			translationSnap.snapValue.x = ts["snapValue"][0].get<float>();
			translationSnap.snapValue.y = ts["snapValue"][1].get<float>();
			translationSnap.snapValue.z = ts["snapValue"][2].get<float>();
		}
	}
	if (j.contains("rotationSnap")) {
		const auto& rs = j["rotationSnap"];
		if (rs.contains("enabled")) {
			rotationSnap.enabled = rs["enabled"].get<bool>();
		}
		if (rs.contains("snapAllAxes")) {
			rotationSnap.snapAllAxes = rs["snapAllAxes"].get<bool>();
		}
		if (rs.contains("snapValue") && rs["snapValue"].is_array() && rs["snapValue"].size() == 3) {
			rotationSnap.snapValue.x = rs["snapValue"][0].get<float>();
			rotationSnap.snapValue.y = rs["snapValue"][1].get<float>();
			rotationSnap.snapValue.z = rs["snapValue"][2].get<float>();
		}
	}
	if (j.contains("scaleSnap")) {
		const auto& ss = j["scaleSnap"];
		if (ss.contains("enabled")) {
			scaleSnap.enabled = ss["enabled"].get<bool>();
		}
		if (ss.contains("snapAllAxes")) {
			scaleSnap.snapAllAxes = ss["snapAllAxes"].get<bool>();
		}
		if (ss.contains("snapValue") && ss["snapValue"].is_array() && ss["snapValue"].size() == 3) {
			scaleSnap.snapValue.x = ss["snapValue"][0].get<float>();
			scaleSnap.snapValue.y = ss["snapValue"][1].get<float>();
			scaleSnap.snapValue.z = ss["snapValue"][2].get<float>();
		}
	}
}