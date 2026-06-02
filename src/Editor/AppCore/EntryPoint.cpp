//-- includes -----
#include "App.h"
#include "ThreadUtils.h"
#include "include/cef_app.h"

#ifdef WIN32
#include <windows.h>
#endif

//-- entry point -----
#ifdef WIN32
int __stdcall WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	// Allow Mikan.exe to act as its own CEF subprocess handler.
	// Subprocess invocations return >= 0 and must exit immediately.
	CefMainArgs cef_args(hInstance);
	int cef_exit = CefExecuteProcess(cef_args, nullptr, nullptr);
	if (cef_exit >= 0)
		return cef_exit;

	ThreadUtils::initMainThreadId();
	App app;

	return app.exec(__argc, __argv);
}
#else
extern "C" int main(int argc, char* argv[])
{
	// On Linux/macOS, CefMainArgs wraps (argc, argv) instead of an HINSTANCE.
	CefMainArgs cef_args(argc, argv);
	int cef_exit = CefExecuteProcess(cef_args, nullptr, nullptr);
	if (cef_exit >= 0)
		return cef_exit;

	ThreadUtils::initMainThreadId();
	App app;

	return app.exec(argc, argv);
}
#endif