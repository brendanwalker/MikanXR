//-- includes -----
#include "App.h"

//-- entry point -----
#ifdef WIN32
int __stdcall WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, 
	int nShowCmd)
{
	App app;

	return app.exec(__argc, __argv);
}
#else
extern "C" int main(int argc, char* argv[])
{
	App app;

	return app.exec(argc, argv);
}
#endif