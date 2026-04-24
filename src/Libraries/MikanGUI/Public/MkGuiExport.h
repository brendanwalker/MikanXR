#pragma once

#ifdef MIKAN_GUI_EXPORTS
	#define MIKAN_GUI_CLASS			__declspec(dllexport)
	#define MIKAN_GUI_FUNC(rtype)	__declspec(dllexport) rtype
#else
	#define MIKAN_GUI_CLASS			__declspec(dllimport)
	#define MIKAN_GUI_FUNC(rtype)	__declspec(dllimport) rtype
#endif
