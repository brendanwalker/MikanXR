#include "MikanVariantTypes.h"
#include "SerializationProperty.h"
#include <Refureku/TypeInfo/Archetypes/EnumValue.h>

#include <assert.h>

const char* mikanVariantTypeToString(MikanVariantType variantType)
{
	int enumIntValue= (int)variantType;
	rfk::Enum const* enumType= rfk::getEnum<MikanVariantType>();
	rfk::EnumValue const* enumValue= enumType->getEnumValue(enumIntValue);

	return Serialization::getEnumStringValue(*enumValue);
}

MikanVariant::MikanVariant()
	: value_type(MikanVariantType::INVALID)
	, value_ptr()
{
}

void MikanVariant::clear()
{
	value_type= MikanVariantType::INVALID;
	value_ptr.reset();
}

bool MikanVariant::getBoolValue() const
{
	assert(value_type == MikanVariantType::BOOL);
	return value_ptr.getTypedPointer<MikanBoolValue>()->value;
}

uint8_t MikanVariant::getUByteValue() const
{
	assert(value_type == MikanVariantType::UBYTE);
	return value_ptr.getTypedPointer<MikanUByteValue>()->value;
}

uint16_t MikanVariant::getUShortValue() const
{
	assert(value_type == MikanVariantType::USHORT);
	return value_ptr.getTypedPointer<MikanUShortValue>()->value;
}

int MikanVariant::getIntValue() const
{
	assert(value_type == MikanVariantType::INT);
	return value_ptr.getTypedPointer<MikanIntValue>()->value;
}

long MikanVariant::getLongValue() const
{
	assert(value_type == MikanVariantType::LONG);
	return value_ptr.getTypedPointer<MikanLongValue>()->value;
}

float MikanVariant::getFloatValue() const
{
	if (value_type == MikanVariantType::FLOAT)
	{
		return value_ptr.getTypedPointer<MikanFloatValue>()->value;
	}
	else if (value_type == MikanVariantType::DOUBLE)
	{
		return (float)value_ptr.getTypedPointer<MikanDoubleValue>()->value;
	}
	else if (value_type == MikanVariantType::INT)
	{
		return (float)value_ptr.getTypedPointer<MikanIntValue>()->value;
	}
	else if (value_type == MikanVariantType::LONG)
	{
		return (float)value_ptr.getTypedPointer<MikanLongValue>()->value;
	}
	else if (value_type == MikanVariantType::USHORT)
	{
		return (float)value_ptr.getTypedPointer<MikanUShortValue>()->value;
	}
	else
	{
		assert(false && "Unsupported variant cast");
		return 0;
	}
}

double MikanVariant::getDoubleValue() const
{
	if (value_type == MikanVariantType::DOUBLE)
	{
		return value_ptr.getTypedPointer<MikanDoubleValue>()->value;
	}
	else if (value_type == MikanVariantType::FLOAT)
	{
		return (double)value_ptr.getTypedPointer<MikanFloatValue>()->value;
	}
	else if (value_type == MikanVariantType::INT)
	{
		return (double)value_ptr.getTypedPointer<MikanIntValue>()->value;
	}
	else if (value_type == MikanVariantType::LONG)
	{
		return (double)value_ptr.getTypedPointer<MikanLongValue>()->value;
	}
	else if (value_type == MikanVariantType::USHORT)
	{
		return (double)value_ptr.getTypedPointer<MikanUShortValue>()->value;
	}
	else
	{
		assert(false && "Unsupported variant cast");
		return 0;
	}
}

const Serialization::String& MikanVariant::getSerializationStringValue() const
{
	assert(value_type == MikanVariantType::STRING);
	return value_ptr.getTypedPointer<MikanStringValue>()->value;
}

const char* MikanVariant::getUtf8StringPointerValue() const { return getSerializationStringValue().getValue(); }

const MikanVector2f& MikanVariant::getVector2fValue() const
{
	assert(value_type == MikanVariantType::VECTOR2F);
	return value_ptr.getTypedPointer<MikanVector2fValue>()->value;
}

const MikanVector3f& MikanVariant::getVector3fValue() const
{
	assert(value_type == MikanVariantType::VECTOR3F);
	return value_ptr.getTypedPointer<MikanVector3fValue>()->value;
}

const MikanVector4f& MikanVariant::getVector4fValue() const
{
	assert(value_type == MikanVariantType::VECTOR4F);
	return value_ptr.getTypedPointer<MikanVector4fValue>()->value;
}

const MikanQuatf& MikanVariant::getQuaternionfValue() const
{
	assert(value_type == MikanVariantType::QUATERNIONF);
	return value_ptr.getTypedPointer<MikanQuatfValue>()->value;
}

const MikanMatrix4f& MikanVariant::getMatrix4fValue() const
{
	assert(value_type == MikanVariantType::MATRIX4F);
	return value_ptr.getTypedPointer<MikanMatrix4fValue>()->value;
}

const MikanVector2d& MikanVariant::getVector2dValue() const
{
	assert(value_type == MikanVariantType::VECTOR2D);
	return value_ptr.getTypedPointer<MikanVector2dValue>()->value;
}

const MikanVector3d& MikanVariant::getVector3dValue() const
{
	assert(value_type == MikanVariantType::VECTOR3D);
	return value_ptr.getTypedPointer<MikanVector3dValue>()->value;
}

const MikanVector4d& MikanVariant::getVector4dValue() const
{
	assert(value_type == MikanVariantType::VECTOR4D);
	return value_ptr.getTypedPointer<MikanVector4dValue>()->value;
}

const MikanQuatd& MikanVariant::getQuaterniondValue() const
{
	assert(value_type == MikanVariantType::QUATERNIOND);
	return value_ptr.getTypedPointer<MikanQuatdValue>()->value;
}

const float MikanVariant::getVectorComponentValue(size_t index) const
{
	const float* rawVectorPtr= nullptr;

	switch (value_type)
	{
	case MikanVariantType::VECTOR2F:
	{
		rawVectorPtr= (const float*)&value_ptr.getTypedPointer<MikanVector2fValue>()->value;
		assert(index < 2);
		return rawVectorPtr[index];
	}
	case MikanVariantType::VECTOR3F:
	{
		rawVectorPtr= (const float*)&value_ptr.getTypedPointer<MikanVector3fValue>()->value;
		assert(index < 3);
		return rawVectorPtr[index];
	}
	case MikanVariantType::VECTOR4F:
	{
		rawVectorPtr= (const float*)&value_ptr.getTypedPointer<MikanVector4fValue>()->value;
		assert(index < 4);
		return rawVectorPtr[index];
	}
	default:
		assert(false && "Invalid vector type");
		return 0.0f;
	}
}

const Serialization::List<bool>& MikanVariant::getBoolArrayValue() const
{
	assert(value_type == MikanVariantType::BOOL_ARRAY);
	return value_ptr.getTypedPointer<MikanBoolArrayValue>()->value;
}

const Serialization::List<uint8_t>& MikanVariant::getUByteArrayValue() const
{
	assert(value_type == MikanVariantType::UBYTE_ARRAY);
	return value_ptr.getTypedPointer<MikanUByteArrayValue>()->value;
}

const Serialization::List<int>& MikanVariant::getIntArrayValue() const
{
	assert(value_type == MikanVariantType::INT_ARRAY);
	return value_ptr.getTypedPointer<MikanIntArrayValue>()->value;
}

const Serialization::List<float>& MikanVariant::getFloatArrayValue() const
{
	assert(value_type == MikanVariantType::FLOAT_ARRAY);
	return value_ptr.getTypedPointer<MikanFloatArrayValue>()->value;
}

const Serialization::List<Serialization::String>& MikanVariant::getStringArrayValue() const
{
	assert(value_type == MikanVariantType::STRING_ARRAY);
	return value_ptr.getTypedPointer<MikanStringArrayValue>()->value;
}

const Serialization::Map<Serialization::String, Serialization::String>& MikanVariant::getStringMapValue() const
{
	assert(value_type == MikanVariantType::STRING_MAP);
	return value_ptr.getTypedPointer<MikanStringMapValue>()->value;
}

const Serialization::PolymorphicObjectPtr& MikanVariant::getPolymorphicObjectValue() const
{
	assert(value_type == MikanVariantType::POLYMORPHIC_OBJECT);
	return value_ptr;
}

#if MIKAN_SETTER_API_ENABLED
#include "PathUtils.h"

void MikanVariant::setValue(const MikanVariant& other)
{
	switch (other.value_type)
	{
	case MikanVariantType::BOOL:
		setValue(other.getBoolValue());
		break;
	case MikanVariantType::UBYTE:
		setValue(other.getUByteValue());
		break;
	case MikanVariantType::USHORT:
		setValue(other.getUShortValue());
		break;
	case MikanVariantType::INT:
		setValue(other.getIntValue());
		break;
	case MikanVariantType::LONG:
		setValue(other.getLongValue());
		break;
	case MikanVariantType::FLOAT:
		setValue(other.getFloatValue());
		break;
	case MikanVariantType::DOUBLE:
		setValue(other.getDoubleValue());
		break;
	case MikanVariantType::STRING:
		setValue(other.getUtf8StringPointerValue());
		break;
	case MikanVariantType::VECTOR2F:
		setValue(other.getVector2fValue());
		break;
	case MikanVariantType::VECTOR3F:
		setValue(other.getVector3fValue());
		break;
	case MikanVariantType::VECTOR4F:
		setValue(other.getVector4fValue());
		break;
	case MikanVariantType::QUATERNIONF:
		setValue(other.getQuaternionfValue());
		break;
	case MikanVariantType::MATRIX4F:
		setValue(other.getMatrix4fValue());
		break;
	case MikanVariantType::VECTOR2D:
		setValue(other.getVector2dValue());
		break;
	case MikanVariantType::VECTOR3D:
		setValue(other.getVector3dValue());
		break;
	case MikanVariantType::VECTOR4D:
		setValue(other.getVector4dValue());
		break;
	case MikanVariantType::QUATERNIOND:
		setValue(other.getQuaterniondValue());
		break;
	case MikanVariantType::BOOL_ARRAY:
		setValue(other.getBoolArrayValue());
		break;
	case MikanVariantType::UBYTE_ARRAY:
		setValue(other.getUByteArrayValue());
		break;
	case MikanVariantType::INT_ARRAY:
		setValue(other.getIntArrayValue());
		break;
	case MikanVariantType::FLOAT_ARRAY:
		setValue(other.getFloatArrayValue());
		break;
	case MikanVariantType::STRING_ARRAY:
		setValue(other.getStringArrayValue());
		break;
	case MikanVariantType::STRING_MAP:
		setValue(other.getStringMapValue());
		break;
	case MikanVariantType::POLYMORPHIC_OBJECT:
		setValue(other.getPolymorphicObjectValue());
		break;
	}
}

void MikanVariant::setValue(bool value)
{
	value_type= MikanVariantType::BOOL;
	value_ptr.allocatedByType<MikanBoolValue>()->value= value;
}

void MikanVariant::setValue(uint8_t value)
{
	value_type= MikanVariantType::UBYTE;
	value_ptr.allocatedByType<MikanUByteValue>()->value= value;
}

void MikanVariant::setValue(uint16_t value)
{
	value_type= MikanVariantType::USHORT;
	value_ptr.allocatedByType<MikanUShortValue>()->value= value;
}

void MikanVariant::setValue(int value)
{
	value_type= MikanVariantType::INT;
	value_ptr.allocatedByType<MikanIntValue>()->value= value;
}

void MikanVariant::setValue(long value)
{
	value_type= MikanVariantType::LONG;
	value_ptr.allocatedByType<MikanLongValue>()->value= value;
}

void MikanVariant::setValue(float value)
{
	value_type= MikanVariantType::FLOAT;
	value_ptr.allocatedByType<MikanFloatValue>()->value= value;
}

void MikanVariant::setValue(double value)
{
	value_type= MikanVariantType::DOUBLE;
	value_ptr.allocatedByType<MikanDoubleValue>()->value= value;
}

void MikanVariant::setValue(const char* value)
{
	value_type= MikanVariantType::STRING;
	value_ptr.allocatedByType<MikanStringValue>()->value.setValue(value);
}

void MikanVariant::setValue(const std::string& value)
{
	const char* szStringValue= value.c_str();

	setValue(szStringValue);
}

void MikanVariant::setValue(const std::filesystem::path& value)
{
	// path::string() is platform specific string encoding (using active string code page on Windows)
	// so we need to explicitly convert this to a UTF-8 string to safely serialize this.
	setValue(PathUtils::pathToUtf8(value));
}

void MikanVariant::setValue(const std::vector<bool>& value)
{
	Serialization::List<bool>& value_array= value_ptr.allocatedByType<MikanBoolArrayValue>()->value;
	value_type= MikanVariantType::BOOL_ARRAY;
	value_array.resize(value.size());
	for (size_t i= 0; i < value.size(); ++i)
		value_array[i]= value[i];
}

void MikanVariant::setValue(const Serialization::List<bool>& value)
{
	value_ptr.allocatedByType<MikanBoolArrayValue>()->value= value;
	value_type= MikanVariantType::BOOL_ARRAY;
}

void MikanVariant::setValue(const std::vector<uint8_t>& value)
{
	Serialization::List<uint8_t>& value_array= value_ptr.allocatedByType<MikanUByteArrayValue>()->value;
	value_type= MikanVariantType::UBYTE_ARRAY;
	value_array.assign(value.data(), value.data() + value.size());
}

void MikanVariant::setValue(const std::vector<int>& value)
{
	Serialization::List<int>& value_array= value_ptr.allocatedByType<MikanIntArrayValue>()->value;

	value_type= MikanVariantType::INT_ARRAY;
	value_array.assign(value.data(), value.data() + value.size());
}

void MikanVariant::setValue(const std::vector<float>& value)
{
	Serialization::List<float>& value_array= value_ptr.allocatedByType<MikanFloatArrayValue>()->value;

	value_type= MikanVariantType::FLOAT_ARRAY;
	value_array.assign(value.data(), value.data() + value.size());
}

void MikanVariant::setValue(const std::vector<std::string>& value)
{
	Serialization::List<Serialization::String>& value_array= value_ptr.allocatedByType<MikanStringArrayValue>()->value;

	value_type= MikanVariantType::STRING_ARRAY;
	value_array.clear();
	for (const std::string& str : value)
	{
		value_array.push_back(Serialization::String(str.c_str()));
	}
}

void MikanVariant::setValue(const std::vector<Serialization::String>& value)
{
	Serialization::List<Serialization::String>& value_array= value_ptr.allocatedByType<MikanStringArrayValue>()->value;

	value_type= MikanVariantType::STRING_ARRAY;
	value_array.assign(value.data(), value.data() + value.size());
}

void MikanVariant::setValue(const Serialization::List<uint8_t>& value)
{
	value_ptr.allocatedByType<MikanUByteArrayValue>()->value= value;
	value_type= MikanVariantType::UBYTE_ARRAY;
}

void MikanVariant::setValue(const Serialization::List<int>& value)
{
	value_ptr.allocatedByType<MikanIntArrayValue>()->value= value;
	value_type= MikanVariantType::INT_ARRAY;
}

void MikanVariant::setValue(const Serialization::List<float>& value)
{
	value_ptr.allocatedByType<MikanFloatArrayValue>()->value= value;
	value_type= MikanVariantType::FLOAT_ARRAY;
}

void MikanVariant::setValue(const Serialization::List<Serialization::String>& value)
{
	value_ptr.allocatedByType<MikanStringArrayValue>()->value= value;
	value_type= MikanVariantType::STRING_ARRAY;
}

void MikanVariant::setValue(const Serialization::Map<Serialization::String, Serialization::String>& value)
{
	Serialization::Map<Serialization::String, Serialization::String>& value_map=
		value_ptr.allocatedByType<MikanStringMapValue>()->value;

	value_type= MikanVariantType::STRING_MAP;
	value_map= value;
}

void MikanVariant::setValue(const MikanVector2f& value)
{
	value_type= MikanVariantType::VECTOR2F;
	value_ptr.allocatedByType<MikanVector2fValue>()->value= value;
}

void MikanVariant::setValue(const MikanVector3f& value)
{
	value_type= MikanVariantType::VECTOR3F;
	value_ptr.allocatedByType<MikanVector3fValue>()->value= value;
}

void MikanVariant::setValue(const MikanVector4f& value)
{
	value_type= MikanVariantType::VECTOR4F;
	value_ptr.allocatedByType<MikanVector4fValue>()->value= value;
}

void MikanVariant::setValue(const MikanQuatf& value)
{
	value_type= MikanVariantType::QUATERNIONF;
	value_ptr.allocatedByType<MikanQuatfValue>()->value= value;
}

void MikanVariant::setValue(const MikanMatrix4f& value)
{
	value_type= MikanVariantType::MATRIX4F;
	value_ptr.allocatedByType<MikanMatrix4fValue>()->value= value;
}

void MikanVariant::setValue(const MikanVector2d& value)
{
	value_type= MikanVariantType::VECTOR2D;
	value_ptr.allocatedByType<MikanVector2dValue>()->value= value;
}

void MikanVariant::setValue(const MikanVector3d& value)
{
	value_type= MikanVariantType::VECTOR3D;
	value_ptr.allocatedByType<MikanVector3dValue>()->value= value;
}

void MikanVariant::setValue(const MikanVector4d& value)
{
	value_type= MikanVariantType::VECTOR4D;
	value_ptr.allocatedByType<MikanVector4dValue>()->value= value;
}

void MikanVariant::setValue(const MikanQuatd& value)
{
	value_type= MikanVariantType::QUATERNIOND;
	value_ptr.allocatedByType<MikanQuatdValue>()->value= value;
}

void MikanVariant::setVectorComponentValue(size_t index, float value)
{
	switch (value_type)
	{
	case MikanVariantType::VECTOR2F:
	{
		assert(index < 2);
		float* rawVectorPtr= (float*)&value_ptr.getTypedPointer<MikanVector2fValue>()->value;
		rawVectorPtr[index]= value;
	}
	break;
	case MikanVariantType::VECTOR3F:
	{
		assert(index < 3);
		float* rawVectorPtr= (float*)&value_ptr.getTypedPointer<MikanVector3fValue>()->value;
		rawVectorPtr[index]= value;
	}
	break;
	case MikanVariantType::VECTOR4F:
	{
		assert(index < 4);
		float* rawVectorPtr= (float*)&value_ptr.getTypedPointer<MikanVector4fValue>()->value;
		rawVectorPtr[index]= value;
	}
	break;
	default:
		assert(false && "Invalid vector type");
	}
}

void MikanVariant::setValue(const Serialization::PolymorphicObjectPtr& value)
{
	value_type= MikanVariantType::POLYMORPHIC_OBJECT;
	value_ptr= value;
}
#endif // MIKAN_SETTER_API_ENABLED