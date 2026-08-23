//-- includes -----
#include "CmdApp.h"
#include "ThreadUtils.h"

//-- entry point -----
int main(int argc, char* argv[])
{
	ThreadUtils::initMainThreadId();

	CmdApp app;

	return app.exec(argc, argv);
}
