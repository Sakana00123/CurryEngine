#pragma once
#include "ImSequencer.h"
#include <vector>
#include <string>

struct TimelineEvent
{
	int startFrame;
	int endFrame;
	int type; // For future use if multiple event types are needed
	std::string label;
	unsigned int color; // RGBA format
};

class Timeline : public ImSequencer::SequenceInterface
{
public:
	Timeline() = default;
	~Timeline() = default;
	// Inherited via SequenceInterface
	int GetFrameMin() const override { return frameMin; }
	int GetFrameMax() const override { return frameMax; }
	int GetItemCount() const override { return static_cast<int>(events.size()); }
	void Get(int index, int** start, int** end, int* type, unsigned int* color) override
	{
		if (index < 0 || index >= static_cast<int>(events.size()))
			return;
		*start = &events[index].startFrame;
		*end = &events[index].endFrame;
		*type = 0; // Single type for now
		*color = events[index].color;
	}
	void Add(int type) override
	{
		// Add a new event with default values
		TimelineEvent newEvent{ 
			.startFrame = currentFrame, 
			.endFrame = currentFrame + 10,
			.type = type, 
			.label = "New Event", 
			.color = 0xFF00FFFF // Cyan color
		}; 
		events.push_back(newEvent);
	}
	void Del(int index) override
	{
		if (index < 0 || index >= static_cast<int>(events.size()))
			return;
		events.erase(events.begin() + index);
	}
	void Duplicate(int index) override
	{
		if (index < 0 || index >= static_cast<int>(events.size()))
			return;
		TimelineEvent newEvent = events[index];
		newEvent.startFrame += 10; // Offset the duplicated event
		newEvent.endFrame += 10;
		newEvent.label += " Copy";
		events.push_back(newEvent);
	}
	void Copy() override
	{
		copiedEvents = events;
	}
	void Paste() override
	{
		for (auto& event : copiedEvents)
		{
			event.startFrame += 10; // Offset pasted events
			event.endFrame += 10;
			event.label += " Pasted";
			events.push_back(event);
		}
	}
	size_t GetCustomHeight(int /*index*/) override { return 0; }
	void DoubleClick(int /*index*/) override {}
	void CustomDraw(int /*index*/, ImDrawList* /*draw_list*/, const ImRect& /*rc*/, const ImRect& /*legendRect*/, const ImRect& /*clippingRect*/, const ImRect& /*legendClippingRect*/) override
		{
		// Custom drawing can be implemented here if needed
	}
	void CustomDrawCompact(int /*index*/, ImDrawList* /*draw_list*/, const ImRect& /*rc*/, const ImRect& /*clippingRect*/) override
		{
		// Custom compact drawing can be implemented here if needed
	}
	void SetFrameRange(int min, int max) { frameMin = min; frameMax = max; }
	void SetCurrentFrame(int frame) { currentFrame = frame; }
	int GetCurrentFrame() const { return currentFrame; }
	void Clear() { events.clear(); }

	// GUI rendering function
	void DrawGUI();

private:
	int frameMin{ 0 };
	int frameMax{ 100 };
	int currentFrame{ 0 };
	std::vector<TimelineEvent> events;
	std::vector<TimelineEvent> copiedEvents; // For copy-paste functionality
};