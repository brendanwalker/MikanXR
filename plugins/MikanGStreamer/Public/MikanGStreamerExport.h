#pragma once

#ifndef MIKAN_CALL
    #if defined _WIN32 || defined __CYGWIN__
        #define MIKAN_CALL __cdecl
        #define MIKAN_CALLBACK __stdcall
    #else
        #define MIKAN_CALL
        #define MIKAN_CALLBACK
    #endif
#endif

#ifndef MIKAN_EXTERN_C
    #ifdef __cplusplus
        #define MIKAN_EXTERN_C extern "C"
    #else
        #define MIKAN_EXTERN_C
    #endif
#endif

#ifndef MIKAN_GSTREAMER_API
    #if defined(MIKAN_GSTREAMER_EXPORTS)
        #if defined _WIN32 || defined __CYGWIN__
            #define MIKAN_GSTREAMER_CAPI(rval)       MIKAN_EXTERN_C  __declspec(dllexport)                   rval    MIKAN_CALL
            #define MIKAN_GSTREAMER_API                              __declspec(dllexport)
        #else  // Not Windows
            #if __GNUC__ >= 4
                #define MIKAN_GSTREAMER_CAPI(rval)   MIKAN_EXTERN_C  __attribute__((visibility("default")))  rval    MIKAN_CALL
                #define MIKAN_GSTREAMER_API                          __attribute__((visibility("default")))
            #else
                #define MIKAN_GSTREAMER_CAPI(rval)   MIKAN_EXTERN_C rval MIKAN_CALL
                #define MIKAN_GSTREAMER_API
            #endif
        #endif  //defined _WIN32 || defined __CYGWIN__
    #elif defined(MIKAN_GSTREAMER_NOEXPORT) || defined(MIKAN_GSTREAMER_CAPI_NOEXPORT)  // Building static lib
        #define MIKAN_GSTREAMER_CAPI(rval)           MIKAN_EXTERN_C                                          rval    MIKAN_CALL
        #define MIKAN_GSTREAMER_API
    #else //This DLL/so/dylib is being imported
        #if defined _WIN32 || defined __CYGWIN__
            #define MIKAN_GSTREAMER_CAPI(rval)       MIKAN_EXTERN_C  __declspec(dllimport)                   rval    MIKAN_CALL
            #define MIKAN_GSTREAMER_API                              __declspec(dllimport)
        #else  // Not Windows
            #define MIKAN_GSTREAMER_CAPI(rval)       MIKAN_EXTERN_C                                          rval    MIKAN_CALL
            #define MIKAN_GSTREAMER_API
        #endif  //defined _WIN32 || defined __CYGWIN__
    #endif //MIKAN_CORE_EXPORTS
#endif //!defined(MIKAN_GSTREAMER_CAPI)