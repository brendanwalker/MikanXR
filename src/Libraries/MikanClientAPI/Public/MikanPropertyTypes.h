#pragma once

#include "MikanAPIExport.h"
#include "MikanTypeFwd.h"
#include "MikanCoreTypes.h"
#include "MikanVariantTypes.h"
#include "SerializableString.h"
#include "SerializableObjectPtr.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanPropertyTypes.rfkh.h"
#endif // MIKANAPI_REFLECTION_ENABLED

#include <string>

enum class ENUM(Serialization::CodeGenModule("MikanPropertyRequests")) MikanPropertyNotifyMode
{
	NONE ENUMVALUE_STRING("NONE"),
	NAME ENUMVALUE_STRING("NAME"),
	NAME_AND_VALUE ENUMVALUE_STRING("NAME_AND_VALUE")
};

/// Structure representing a component property value
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanPropertyTypes")) MikanPropertyDescriptor
{
	FIELD()
	Serialization::String ownerSystemClass;
	FIELD()
	Serialization::String ownerComponentClass;
	FIELD()
	Serialization::String fieldName;
	FIELD()
	MikanVariantType fieldType;
	FIELD()
	bool isReadOnly;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanPropertyDescriptor_GENERATED
#endif // MIKANAPI_REFLECTION_ENABLED
};

/// Structure representing a component property value
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanPropertyTypes")) MikanPropertyValue
{
	FIELD()
	Serialization::String ownerSystem;
	FIELD()
	int componentId;
	FIELD()
	Serialization::String fieldName;
	FIELD()
	MikanVariant fieldValue;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanPropertyValue_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanPropertyTypes_GENERATED
#endif // MIKANAPI_REFLECTION_ENABLED
