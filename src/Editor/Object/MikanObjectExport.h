#pragma once

#ifndef MIKAN_OBJECT_CLASS
    #if defined(MIKAN_OBJECT_EXPORTS)
        #if defined _WIN32 || defined __CYGWIN__
            #define MIKAN_OBJECT_FUNC(rval)       __declspec(dllexport)                   rval
            #define MIKAN_OBJECT_CLASS            __declspec(dllexport)
        #else  // Not Windows
            #if __GNUC__ >= 4
                #define MIKAN_OBJECT_FUNC(rval)   __attribute__((visibility("default")))  rval
                #define MIKAN_OBJECT_CLASS        __attribute__((visibility("default")))
            #else
                #define MIKAN_OBJECT_FUNC(rval)   rval
                #define MIKAN_OBJECT_CLASS
            #endif
        #endif  //defined _WIN32 || defined __CYGWIN__
    #else 
        #define MIKAN_OBJECT_FUNC(rval)                                                   rval
        #define MIKAN_OBJECT_CLASS
    #endif //MIKAN_OBJECT_EXPORTS
#endif //!defined(MIKAN_OBJECT_CLASS)