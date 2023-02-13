%module MikanClient
%include "typemaps.i"
%include "stdint.i"
%include "swigtype_inout.i"

%begin %{
  #ifdef _MSC_VER
  #define SWIG_PYTHON_INTERPRETER_NO_DEBUG
  #endif
%}

%{
  #include "MikanConstants.h"
  #include "MikanClientTypes.h"
  #include "MikanMathTypes.h"
  #include "MikanClient_CAPI.h"
%}

#ifdef _MSC_VER
#define MIKAN_CALL __cdecl
#define MIKAN_CALLBACK __stdcall
#else
#define MIKAN_CALL
#define MIKAN_CALLBACK
#endif
#define MIKAN_PUBLIC_FUNCTION(rval) rval
#define MIKAN_PUBLIC_CLASS

#if defined(SWIGPYTHON)
#endif  /* SWIGPYTHON */

#if defined(SWIGCSHARP)
%include "MikanClientCSharpTypeMaps.i"
%include "MikanClientCSharpCallbacks.i"
#endif  /* SWIGCSHARP */

// TODO: Figure out how to properly support void* out parameters
%ignore Mikan_GetGraphicsDeviceInterface;

%include "../src/Client/MikanConstants.h"
%include "../src/Client/MikanMathTypes.h"

// Make all variables read-only in exposed structs in MikanClient_CAPI.h
%immutable;
%include "../src/Client/MikanClientTypes.h"
%include "../src/Client/MikanClient_CAPI.h"