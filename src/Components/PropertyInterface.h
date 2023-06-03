#pragma once

#include "RmlFwd.h"

#include <string>
#include <vector>

enum class ePropertyDataType : int
{
	INVALID = -1,

	datatype_bool,
	datatype_int,
	datatype_float,
	datatype_float2,
	datatype_float3,
	datatype_float4,
	datatype_string
};

enum class ePropertySemantic : int
{
	INVALID = -1,

	checkbox,
	position,
	rotation,
	size3d,
	size2d,
	filename,
	name,
	anchor_id,

	COUNT
};
extern const std::string* k_PropertySemanticNames;

struct PropertyDescriptor
{
	std::string propertyName;
	ePropertyDataType dataType;
	ePropertySemantic semantic;
};

class IPropertyInterface
{
public:
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const = 0;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const = 0;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const = 0;
	virtual bool getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const = 0;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) = 0;
};