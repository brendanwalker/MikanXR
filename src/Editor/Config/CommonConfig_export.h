#pragma once

#ifdef SERIALIZATION_EXPORTS
	#ifdef _MSC_VER
	#pragma warning (push)
	#pragma warning (disable: 4996) // This function or variable may be unsafe
	#pragma warning (disable: 4244) // 'return': conversion from 'const int64_t' to 'float', possible loss of data
	#pragma warning (disable: 4715) // configuru::Config::operator[]': not all control paths return a value
	#endif
	#include "visit_struct/visit_struct_intrusive.hpp"
	#include "configuru.hpp"
	#ifdef _MSC_VER
	#pragma warning (pop)
	#endif

	#define BEGIN_FIELDS(NAME)	BEGIN_VISITABLES(NAME)
	#define FIELD()				VISITABLE(TYPE, NAME)
	#define END_FIELDS			END_VISITABLES
#else
	#define BEGIN_FIELDS(NAME)
	#define FIELD(TYPE, NAME)	TYPE NAME
	#define END_FIELDS
#endif