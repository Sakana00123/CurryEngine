#include "pch.h"
#include <time.h>
#include <exception>

#include "Engine/Core/Framework.h"
#include "Engine/Core/EnginePaths.h"
#include "Engine/Core/Startup.h"

LRESULT CALLBACK window_procedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	Framework* p{ reinterpret_cast<Framework*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)) };
	return p ? p->HandleMessage(hwnd, msg, wparam, lparam) : DefWindowProc(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_  HINSTANCE prev_instance, _In_ LPSTR cmd_line, _In_ int cmd_show)
{
	if (const auto exitCode = CurryEngine::PrepareStartup(EnginePaths::StartupContext))
		return *exitCode;
	EnginePaths::ConfigureResourcePaths();
	CurryEngine::BeginStartupDiagnostics(EnginePaths::StartupContext);
	CurryEngine::RecordStartupStage("Creating window");

	srand(static_cast<unsigned int>(time(nullptr)));

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(####);
#endif

	WNDCLASSEXW wcex{};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = window_procedure;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = instance;
	wcex.hIcon = LoadIcon(instance, MAKEINTRESOURCE(333));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = APPLICATION_NAME;
	wcex.hIconSm = 0;
	RegisterClassExW(&wcex);

	RECT rc{ 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	HWND hwnd = CreateWindowExW(0, APPLICATION_NAME, L"", WS_OVERLAPPEDWINDOW/* ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME*/, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, instance, NULL);
	ShowWindow(hwnd, FULLSCREEN ? cmd_show : SW_SHOWMAXIMIZED);

	// フレームワークの実行
	try
	{
		Framework framework(hwnd);
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&framework));
		const auto result = framework.Run();
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
		return result;
	}
	catch (const std::exception& error)
	{
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
		CurryEngine::ReportRuntimeFailure(error.what());
	}
	catch (...)
	{
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
		CurryEngine::ReportRuntimeFailure("Unknown native exception.");
	}
	// Partial startup may have live watcher threads and uninitialized subsystems.
	// Do not run normal shutdown (which saves project state) or static thread destructors.
	ExitProcess(1);
}
