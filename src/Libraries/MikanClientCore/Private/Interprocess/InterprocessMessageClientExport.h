#pragma once

#ifndef MIKAN_INTERPROCESS_CLIENT_CLASS
    #if defined(MIKAN_INTERPROCESS_CLIENT_EXPORTS)
        #if defined _WIN32 || defined __CYGWIN__
            #define MIKAN_INTERPROCESS_CLIENT_FUNC(rval)       __declspec(dllexport)                   rval
            #define MIKAN_INTERPROCESS_CLIENT_CLASS            __declspec(dllexport)
        #else  // Not Windows
            #if __GNUC__ >= 4
                #define MIKAN_INTERPROCESS_CLIENT_FUNC(rval)   __attribute__((visibility("default")))  rval
                #define MIKAN_INTERPROCESS_CLIENT_CLASS        __attribute__((visibility("default")))
            #else
                #define MIKAN_INTERPROCESS_CLIENT_FUNC(rval)   rval
                #define MIKAN_INTERPROCESS_CLIENT_CLASS
            #endif
        #endif  //defined _WIN32 || defined __CYGWIN__
    #else 
        #define MIKAN_INTERPROCESS_CLIENT_FUNC(rval)                                                   rval
        #define MIKAN_INTERPROCESS_CLIENT_CLASS
    #endif //MIKAN_INTERPROCESS_CLIENT_EXPORTS
#endif //!defined(MIKAN_INTERPROCESS_CLIENT_CLASS)