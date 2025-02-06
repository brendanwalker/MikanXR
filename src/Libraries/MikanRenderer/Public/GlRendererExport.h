#pragma once

#ifndef MIKAN_RENDERER_CLASS
    #if defined(MIKAN_RENDERER_EXPORTS)
        #if defined _WIN32 || defined __CYGWIN__
            #define MIKAN_RENDERER_FUNC(rval)       __declspec(dllexport)                   rval
            #define MIKAN_RENDERER_CLASS            __declspec(dllexport)
        #else  // Not Windows
            #if __GNUC__ >= 4
                #define MIKAN_RENDERER_FUNC(rval)   __attribute__((visibility("default")))  rval
                #define MIKAN_RENDERER_CLASS        __attribute__((visibility("default")))
            #else
                #define MIKAN_RENDERER_FUNC(rval)   rval
                #define MIKAN_RENDERER_CLASS
            #endif
        #endif  //defined _WIN32 || defined __CYGWIN__
    #else 
        #define MIKAN_RENDERER_FUNC(rval)                                                   rval
        #define MIKAN_RENDERER_CLASS
    #endif //MIKAN_RENDERER_EXPORTS
#endif //!defined(MIKAN_RENDERER_CLASS)