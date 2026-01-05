#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableList.h"
#include "SerializableString.h"
#include "SerializableObjectPtr.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanVariantTypes.rfkh.h"
#endif

#include <string>
#include <type_traits>
#include <vector>

enum class ENUM(Serialization::CodeGenModule("MikanVariantTypes")) MikanVariantType
{
	INVALID ENUMVALUE_STRING("INVALID_TYPE"),
	
	// Primitive Types
	BOOL ENUMVALUE_STRING("BOOL_TYPE"),
	INT ENUMVALUE_STRING("INT_TYPE"),
	FLOAT ENUMVALUE_STRING("FLOAT_TYPE"),
	DOUBLE ENUMVALUE_STRING("DOUBLE_TYPE"),
	STRING ENUMVALUE_STRING("MK_STRING_TYPE"),
	VECTOR2F ENUMVALUE_STRING("VECTOR2F_TYPE"),
	VECTOR3F ENUMVALUE_STRING("VECTOR3F_TYPE"),
	VECTOR4F ENUMVALUE_STRING("VECTOR4F_TYPE"),
	
	// Array Types
	INT_ARRAY ENUMVALUE_STRING("INT_ARRAY_TYPE"),
};

/// Bundle containing all intrinsic video source properties
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVariantTypes")) MikanVariant
{
	FIELD()
	MikanVariantType value_type;

	// MikanVariantBase derived type
	FIELD()
	Serialization::PolymorphicObjectPtr value_ptr;

	MikanVariant();

	// Construct by variant type
	template< typename T >
	explicit MikanVariant(T&& t) 
		: value_type(MikanVariantType::INVALID)
		, value_ptr()
	{
		setValue(std::forward<T>(t));
	}

	// Assign by variant type
	template<typename T>
	MikanVariant& operator=(T&& t)
	{
		setValue(std::forward<T>(t));
		return *this;
	}

	void clear();

	bool getBoolValue() const;
	int getIntValue() const;
	float getFloatValue() const;
	double getDoubleValue() const;
	const std::string& getStringValue() const;
	const MikanVector2f& getVector2fValue() const;
	const MikanVector3f& getVector3fValue() const;
	const MikanVector4f& getVector4fValue() const;
	const float getVectorComponentValue(size_t index) const;
	const std::vector<int>& getIntArrayValue() const;

#if defined(MIKANAPI_REFLECTION_ENABLED) && defined(SERIALIZATION_REFLECTION_ENABLED)
	void setValue(const MikanVariant& other);
	void setValue(bool value);
	void setValue(int value);
	void setValue(float value);
	void setValue(double value);
	void setValue(const char* value);
	void setValue(const std::string& value);
	void setValue(const MikanVector2f& value);
	void setValue(const MikanVector3f& value);
	void setValue(const MikanVector4f& value);
	void setVectorComponentValue(size_t index, float value);
	void setValue(const std::vector<int>& value);
#endif // MIKANAPI_REFLECTION_ENABLED && SERIALIZATION_REFLECTION_ENABLED

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVariant_GENERATED
#endif // MIKANAPI_REFLECTION_ENABLED
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanVariantBase
	: public Serialization::PolymorphicStruct
{
#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVariantBase_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVariantTypes")) MikanBoolValue
	: public MikanVariantBase
{
	FIELD()
	bool value;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanBoolValue_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVariantTypes")) MikanIntValue
	: public MikanVariantBase
{
	FIELD()
	int value;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanIntValue_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVariantTypes")) MikanFloatValue
	: public MikanVariantBase
{
	FIELD()
	float value;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanFloatValue_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVariantTypes")) MikanDoubleValue
	: public MikanVariantBase
{
	FIELD()
	double value;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanDoubleValue_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVariantTypes")) MikanStringValue
	: public MikanVariantBase
{
	FIELD()
	Serialization::String value;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStringValue_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVariantTypes")) MikanVector2fValue
	: public MikanVariantBase
{
	FIELD()
	MikanVector2f value;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVector2fValue_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVariantTypes")) MikanVector3fValue
	: public MikanVariantBase
{
	FIELD()
	MikanVector3f value;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVector3fValue_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVariantTypes")) MikanVector4fValue
	: public MikanVariantBase
{
	FIELD()
	MikanVector4f value;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVector4fValue_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVariantTypes")) MikanIntArrayValue
	: public MikanVariantBase
{
	FIELD()
	Serialization::List<int> value;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanIntArrayValue_GENERATED
#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanVariantTypes_GENERATED
#endif // MIKANAPI_REFLECTION_ENABLED