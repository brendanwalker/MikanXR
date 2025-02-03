//-- includes -----
#include "App.h"
#include "ThreadUtils.h"

//-- entry point -----
#ifdef WIN32
#include <windows.h>
int __stdcall WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, 
	int nShowCmd)
{
	ThreadUtils::initMainThreadId();
	App app;

	return app.exec(__argc, __argv);
}
#else
extern "C" int main(int argc, char* argv[])
{
	ThreadUtils::initMainThreadId();
	App app;

	return app.exec(argc, argv);
}
#endif