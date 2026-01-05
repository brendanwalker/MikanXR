#include "MikanVariantTypes.h"
#include <assert.h>

MikanVariant::MikanVariant()
	: value_type(MikanVariantType::INVALID)
	, value_ptr()
{
}

void MikanVariant::clear()
{
	value_type = MikanVariantType::INVALID;
	value_ptr.reset();
}

bool MikanVariant::getBoolValue() const
{
	assert(value_type == MikanVariantType::BOOL);
	return value_ptr.getTypedPointer<MikanBoolValue>()->value;
}

int MikanVariant::getIntValue() const
{
	assert(value_type == MikanVariantType::INT);
	return value_ptr.getTypedPointer<MikanIntValue>()->value;
}

float MikanVariant::getFloatValue() const
{
	assert(value_type == MikanVariantType::FLOAT);
	return value_ptr.getTypedPointer<MikanFloatValue>()->value;
}

double MikanVariant::getDoubleValue() const
{
	assert(value_type == MikanVariantType::DOUBLE);
	return value_ptr.getTypedPointer<MikanDoubleValue>()->value;
}

const std::string& MikanVariant::getStringValue() const
{
	assert(value_type == MikanVariantType::STRING);
	return value_ptr.getTypedPointer<MikanStringValue>()->value.getValue();
}

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

const float MikanVariant::getVectorComponentValue(size_t index) const
{
	const float* rawVectorPtr = nullptr;

	switch (value_type)
	{
		case MikanVariantType::VECTOR2F:
		{
			rawVectorPtr = (const float*)&value_ptr.getTypedPointer<MikanVector2fValue>()->value;
			assert(index < 2);
			return rawVectorPtr[index];
		}
		case MikanVariantType::VECTOR3F:
		{
			rawVectorPtr = (const float*)&value_ptr.getTypedPointer<MikanVector3fValue>()->value;
			assert(index < 3);
			return rawVectorPtr[index];
		}
		case MikanVariantType::VECTOR4F:
		{
			rawVectorPtr = (const float*)&value_ptr.getTypedPointer<MikanVector4fValue>()->value;
			assert(index < 4);
			return rawVectorPtr[index];
		}
		default:
			assert(false && "Invalid vector type");
			return 0.0f;
	}
}

const std::vector<int>& MikanVariant::getIntArrayValue() const
{
	assert(value_type == MikanVariantType::INT_ARRAY);
	return value_ptr.getTypedPointer<MikanIntArrayValue>()->value;
}

#if defined(MIKANAPI_REFLECTION_ENABLED) && defined(SERIALIZATION_REFLECTION_ENABLED)
void MikanVariant::setValue(const MikanVariant& other)
{
	switch (other.value_type)
	{
	case MikanVariantType::BOOL:
		setValue(other.getBoolValue());
		break;
	case MikanVariantType::INT:
		setValue(other.getIntValue());
		break;
	case MikanVariantType::FLOAT:
		setValue(other.getFloatValue());
		break;
	case MikanVariantType::DOUBLE:
		setValue(other.getDoubleValue());
		break;
	case MikanVariantType::STRING:
		setValue(other.getStringValue());
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
	case MikanVariantType::INT_ARRAY:
		setValue(other.getIntArrayValue());
		break;
	}
}

void MikanVariant::setValue(bool value)
{
	value_type = MikanVariantType::BOOL;
	value_ptr.allocatedByType<MikanBoolValue>()->value= value;
}

void MikanVariant::setValue(int value)
{
	value_type = MikanVariantType::INT;
	value_ptr.allocatedByType<MikanIntValue>()->value = value;
}

void MikanVariant::setValue(float value)
{
	value_type = MikanVariantType::FLOAT;
	value_ptr.allocatedByType<MikanFloatValue>()->value = value;
}

void MikanVariant::setValue(double value)
{
	value_type = MikanVariantType::DOUBLE;
	value_ptr.allocatedByType<MikanDoubleValue>()->value = value;
}

void MikanVariant::setValue(const char* value)
{
	std::string str_value(value);
	setValue(str_value);
}

void MikanVariant::setValue(const std::string& value)
{
	value_type = MikanVariantType::STRING;
	value_ptr.allocatedByType<MikanStringValue>()->value.setValue(value);
}

void MikanVariant::setValue(const std::vector<int>& value)
{
	std::vector<int>& value_array = value_ptr.allocatedByType<MikanIntArrayValue>()->value;

	value_type = MikanVariantType::INT_ARRAY;
	value_array = value;
}

void MikanVariant::setValue(const MikanVector2f& value)
{
	value_type = MikanVariantType::VECTOR2F;
	value_ptr.allocatedByType<MikanVector2fValue>()->value = value;
}

void MikanVariant::setValue(const MikanVector3f& value)
{
	value_type = MikanVariantType::VECTOR3F;
	value_ptr.allocatedByType<MikanVector3fValue>()->value = value;
}

void MikanVariant::setValue(const MikanVector4f& value)
{
	value_type = MikanVariantType::VECTOR4F;
	value_ptr.allocatedByType<MikanVector4fValue>()->value = value;
}

void MikanVariant::setVectorComponentValue(size_t index, float value)
{
	switch (value_type)
	{
	case MikanVariantType::VECTOR2F:
	{
		assert(index < 2);
		float* rawVectorPtr = (float*)&value_ptr.getTypedPointer<MikanVector2fValue>()->value;
		rawVectorPtr[index] = value;
	} break;
	case MikanVariantType::VECTOR3F:
	{
		assert(index < 3);
		float* rawVectorPtr = (float*)&value_ptr.getTypedPointer<MikanVector3fValue>()->value;
		rawVectorPtr[index] = value;
	} break;
	case MikanVariantType::VECTOR4F:
	{
		assert(index < 4);
		float* rawVectorPtr = (float*)&value_ptr.getTypedPointer<MikanVector4fValue>()->value;
		rawVectorPtr[index] = value;
	} break;
	default:
		assert(false && "Invalid vector type");
	}
}
#endif // MIKANAPI_REFLECTION_ENABLED && SERIALIZATION_REFLECTION_ENABLED