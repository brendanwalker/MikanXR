#pragma once

#ifdef MIKAN_WINDOW_EXPORTS
#define MIKAN_WINDOW_CLASS __declspec(dllexport)
#define MIKAN_WINDOW_FUNC(rtype) __declspec(dllexport) rtype
#else
#define MIKAN_WINDOW_CLASS __declspec(dllimport)
#define MIKAN_WINDOW_FUNC(rtype) __declspec(dllimport) rtype
#endif
