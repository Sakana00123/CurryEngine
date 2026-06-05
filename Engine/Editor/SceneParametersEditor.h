#pragma once

class SceneParametersEditor
{
public:
	static void Show();
	static void DrawGUI();

private:
	static inline bool isOpen = true;
};