#include "pch.h"
#include "Timeline.h"

void Timeline::DrawGUI()
{
#ifdef USE_IMGUI
	static bool expanded = true;
	static int selectedEntry = -1;
	static int firstFrame = 0;
	ImGui::Begin(reinterpret_cast<const char*>(u8"タイムライン###Timeline"));
	if (ImSequencer::Sequencer(this, &currentFrame, &expanded, &selectedEntry, &firstFrame,
		ImSequencer::SEQUENCER_EDIT_ALL | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE))
	{
		// Handle selection change if needed
	}
	// Display details of the selected event
	if (selectedEntry >= 0 && selectedEntry < static_cast<int>(events.size()))
	{
		ImGui::Separator();
		ImGui::Text(reinterpret_cast<const char*>(u8"選択中のイベントの詳細:"));
		ImGui::InputInt(reinterpret_cast<const char*>(u8"開始フレーム"), &events[selectedEntry].startFrame);
		ImGui::InputInt(reinterpret_cast<const char*>(u8"終了フレーム"), &events[selectedEntry].endFrame);
		char buffer[256];
		strncpy_s(buffer, events[selectedEntry].label.c_str(), sizeof(buffer));
		if (ImGui::InputText(reinterpret_cast<const char*>(u8"ラベル"), buffer, sizeof(buffer)))
		{
			events[selectedEntry].label = std::string(buffer);
		}
		ImGui::ColorEdit4(reinterpret_cast<const char*>(u8"色"), reinterpret_cast<float*>(&events[selectedEntry].color), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
	}
	ImGui::End();
#endif // USE_IMGUI
}