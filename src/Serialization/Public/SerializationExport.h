#pragma once

#ifndef MIKAN_SERIALIZATION_API
    #if defined(MIKAN_SERIALIZATION_EXPORTS) // CMake-defined when creating shared library
        #if defined _WIN32 || defined __CYGWIN__
            #define MIKAN_SERIALIZATION_API                                __declspec(dllexport)
        #else  // Not Windows
            #if __GNUC__ >= 4
                #define MIKAN_SERIALIZATION_API                            __attribute__((visibility("default")))
            #else
                #define MIKAN_SERIALIZATION_API
            #endif
        #endif  //defined _WIN32 || defined __CYGWIN__
    #else //This DLL/so/dylib is being imported
        #if defined _WIN32 || defined __CYGWIN__
            #define MIKAN_SERIALIZATION_API                                __declspec(dllimport)
        #else  // Not Windows
            #define MIKAN_SERIALIZATION_API
        #endif  //defined _WIN32 || defined __CYGWIN__
    #endif //MIKAN_SERIALIZATION_EXPORTS
#endif //!defined(MIKAN_SERIALIZATION_API)