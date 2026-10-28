#include "pch.h"
#include "Timeline.h"

void Timeline::DrawGUI()
{
#ifdef USE_IMGUI
	static bool expanded = true;
	static int selectedEntry = -1;
	static int firstFrame = 0;
	ImGui::Begin("Timeline");
	if (ImSequencer::Sequencer(this, &currentFrame, &expanded, &selectedEntry, &firstFrame,
		ImSequencer::SEQUENCER_EDIT_ALL | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE))
	{
		// Handle selection change if needed
	}
	// Display details of the selected event
	if (selectedEntry >= 0 && selectedEntry < static_cast<int>(events.size()))
	{
		ImGui::Separator();
		ImGui::Text("Selected Event Details:");
		ImGui::InputInt("Start Frame", &events[selectedEntry].startFrame);
		ImGui::InputInt("End Frame", &events[selectedEntry].endFrame);
		char buffer[256];
		strncpy_s(buffer, events[selectedEntry].label.c_str(), sizeof(buffer));
		if (ImGui::InputText("Label", buffer, sizeof(buffer)))
		{
			events[selectedEntry].label = std::string(buffer);
		}
		ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&events[selectedEntry].color), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
	}
	ImGui::End();
#endif // USE_IMGUI
}