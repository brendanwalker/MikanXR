#ifndef VERSION_H
#define VERSION_H

/// Conventional string-ification macro.
// From: http://stackoverflow.com/questions/5256313/c-c-macro-string-concatenation
#if !defined(MIKAN_STRINGIZE)
    #define MIKAN_STRINGIZEIMPL(x) #x
    #define MIKAN_STRINGIZE(x)     MIKAN_STRINGIZEIMPL(x)
#endif

// Current version of this release
#define MIKAN_RELEASE_VERSION_PRODUCT 1
#define MIKAN_RELEASE_VERSION_MAJOR   0
#define MIKAN_RELEASE_VERSION_MINOR   0
#define MIKAN_RELEASE_VERSION_RELEASE 0

/// "Product.Major.Minor.Release"
#if !defined(MIKAN_RELEASE_VERSION_STRING)
    #define MIKAN_RELEASE_VERSION_STRING MIKAN_STRINGIZE(MIKAN_RELEASE_VERSION_PRODUCT.MIKAN_RELEASE_VERSION_MAJOR.MIKAN_RELEASE_VERSION_MINOR.MIKAN_RELEASE_VERSION_RELEASE)
#endif

#endif // VERSION_H
