//-- includes -----
#include "App.h"

//-- entry point -----
extern "C" int main(int argc, char* argv[])
{
	App app;

	return app.exec(argc, argv);
}