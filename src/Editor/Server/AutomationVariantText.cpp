#include "AutomationVariantText.h"

#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace
{
// %g with enough digits to round-trip the value through text
std::string formatFloat(float value)
{
	char buffer[64];
	snprintf(buffer, sizeof(buffer), "%.9g", value);
	return buffer;
}

std::string formatDouble(double value)
{
	char buffer[64];
	snprintf(buffer, sizeof(buffer), "%.17g", value);
	return buffer;
}

bool parseLongLong(const std::string& token, long long& outValue)
{
	if (token.empty())
		return false;

	errno= 0;
	char* end= nullptr;
	outValue= strtoll(token.c_str(), &end, 10);
	return errno == 0 && end != nullptr && *end == '\0';
}

bool parseDouble(const std::string& token, double& outValue)
{
	if (token.empty())
		return false;

	errno= 0;
	char* end= nullptr;
	outValue= strtod(token.c_str(), &end);
	return errno == 0 && end != nullptr && *end == '\0';
}

bool parseComponents(const std::vector<std::string>& tokens, size_t expectedCount, double* outComponents,
					 std::string& outError)
{
	if (tokens.size() != expectedCount)
	{
		outError=
			"expected " + std::to_string(expectedCount) + " numeric components, got " + std::to_string(tokens.size());
		return false;
	}

	for (size_t i= 0; i < expectedCount; ++i)
	{
		if (!parseDouble(tokens[i], outComponents[i]))
		{
			outError= "invalid number '" + tokens[i] + "'";
			return false;
		}
	}

	return true;
}

bool parseIntegerInRange(const std::vector<std::string>& tokens, long long minValue, long long maxValue,
						 long long& outValue, std::string& outError)
{
	if (tokens.size() != 1)
	{
		outError= "expected one integer value";
		return false;
	}

	if (!parseLongLong(tokens[0], outValue) || outValue < minValue || outValue > maxValue)
	{
		outError= "invalid integer '" + tokens[0] + "'";
		return false;
	}

	return true;
}

std::string joinTokens(const std::vector<std::string>& tokens, const char* separator)
{
	std::string result;
	for (const std::string& token : tokens)
	{
		if (!result.empty())
			result+= separator;
		result+= token;
	}
	return result;
}
} // namespace

namespace AutomationVariantText
{
std::string variantToText(const MikanVariant& value)
{
	switch (value.value_type)
	{
	case MikanVariantType::BOOL:
		return value.getBoolValue() ? "true" : "false";
	case MikanVariantType::UBYTE:
		return std::to_string(value.getUByteValue());
	case MikanVariantType::USHORT:
		return std::to_string(value.getUShortValue());
	case MikanVariantType::INT:
		return std::to_string(value.getIntValue());
	case MikanVariantType::LONG:
		return std::to_string(value.getLongValue());
	case MikanVariantType::FLOAT:
		return formatFloat(value.getFloatValue());
	case MikanVariantType::DOUBLE:
		return formatDouble(value.getDoubleValue());
	case MikanVariantType::STRING:
		return value.getUtf8Value();
	case MikanVariantType::VECTOR2F:
	{
		const MikanVector2f& v= value.getVector2fValue();
		return formatFloat(v.x) + " " + formatFloat(v.y);
	}
	case MikanVariantType::VECTOR3F:
	{
		const MikanVector3f& v= value.getVector3fValue();
		return formatFloat(v.x) + " " + formatFloat(v.y) + " " + formatFloat(v.z);
	}
	case MikanVariantType::VECTOR4F:
	{
		const MikanVector4f& v= value.getVector4fValue();
		return formatFloat(v.x) + " " + formatFloat(v.y) + " " + formatFloat(v.z) + " " + formatFloat(v.w);
	}
	case MikanVariantType::QUATERNIONF:
	{
		const MikanQuatf& q= value.getQuaternionfValue();
		return formatFloat(q.w) + " " + formatFloat(q.x) + " " + formatFloat(q.y) + " " + formatFloat(q.z);
	}
	case MikanVariantType::MATRIX4F:
	{
		const MikanMatrix4f& m= value.getMatrix4fValue();
		const float components[16]= {m.x0, m.x1, m.x2, m.x3, m.y0, m.y1, m.y2, m.y3,
									 m.z0, m.z1, m.z2, m.z3, m.w0, m.w1, m.w2, m.w3};
		std::string result;
		for (float component : components)
		{
			if (!result.empty())
				result+= " ";
			result+= formatFloat(component);
		}
		return result;
	}
	case MikanVariantType::VECTOR2D:
	{
		const MikanVector2d& v= value.getVector2dValue();
		return formatDouble(v.x) + " " + formatDouble(v.y);
	}
	case MikanVariantType::VECTOR3D:
	{
		const MikanVector3d& v= value.getVector3dValue();
		return formatDouble(v.x) + " " + formatDouble(v.y) + " " + formatDouble(v.z);
	}
	case MikanVariantType::VECTOR4D:
	{
		const MikanVector4d& v= value.getVector4dValue();
		return formatDouble(v.x) + " " + formatDouble(v.y) + " " + formatDouble(v.z) + " " + formatDouble(v.w);
	}
	case MikanVariantType::QUATERNIOND:
	{
		const MikanQuatd& q= value.getQuaterniondValue();
		return formatDouble(q.w) + " " + formatDouble(q.x) + " " + formatDouble(q.y) + " " + formatDouble(q.z);
	}
	case MikanVariantType::BOOL_ARRAY:
	{
		const auto& list= value.getBoolArrayValue();
		std::vector<std::string> entries;
		for (bool entry : list)
			entries.push_back(entry ? "true" : "false");
		return joinTokens(entries, ", ");
	}
	case MikanVariantType::UBYTE_ARRAY:
	{
		const auto& list= value.getUByteArrayValue();
		std::vector<std::string> entries;
		for (uint8_t entry : list)
			entries.push_back(std::to_string(entry));
		return joinTokens(entries, ", ");
	}
	case MikanVariantType::INT_ARRAY:
	{
		const auto& list= value.getIntArrayValue();
		std::vector<std::string> entries;
		for (int entry : list)
			entries.push_back(std::to_string(entry));
		return joinTokens(entries, ", ");
	}
	case MikanVariantType::FLOAT_ARRAY:
	{
		const auto& list= value.getFloatArrayValue();
		std::vector<std::string> entries;
		for (float entry : list)
			entries.push_back(formatFloat(entry));
		return joinTokens(entries, ", ");
	}
	case MikanVariantType::STRING_ARRAY:
	{
		const auto& list= value.getStringArrayValue();
		std::vector<std::string> entries;
		for (const Serialization::String& entry : list)
			entries.push_back(entry.getUtf8Value());
		return joinTokens(entries, ", ");
	}
	case MikanVariantType::STRING_MAP:
	{
		const auto& map= value.getStringMapValue();
		std::vector<std::string> entries;
		for (const auto& [key, entry] : map)
			entries.push_back(std::string(key.getUtf8Value()) + "=" + entry.getUtf8Value());
		return joinTokens(entries, "; ");
	}
	case MikanVariantType::POLYMORPHIC_OBJECT:
		return "<polymorphic object>";
	default:
		return "<invalid>";
	}
}

bool textToVariant(MikanVariantType dataType, const std::vector<std::string>& valueTokens, MikanVariant& outValue,
				   std::string& outError)
{
	switch (dataType)
	{
	case MikanVariantType::BOOL:
	{
		if (valueTokens.size() != 1)
		{
			outError= "expected one boolean value";
			return false;
		}

		const std::string& token= valueTokens[0];
		if (token == "true" || token == "1")
			outValue.setValue(true);
		else if (token == "false" || token == "0")
			outValue.setValue(false);
		else
		{
			outError= "invalid boolean '" + token + "' (use true/false)";
			return false;
		}
		return true;
	}
	case MikanVariantType::UBYTE:
	{
		long long parsed= 0;
		if (!parseIntegerInRange(valueTokens, 0, UINT8_MAX, parsed, outError))
			return false;
		outValue.setValue((uint8_t)parsed);
		return true;
	}
	case MikanVariantType::USHORT:
	{
		long long parsed= 0;
		if (!parseIntegerInRange(valueTokens, 0, UINT16_MAX, parsed, outError))
			return false;
		outValue.setValue((uint16_t)parsed);
		return true;
	}
	case MikanVariantType::INT:
	{
		long long parsed= 0;
		if (!parseIntegerInRange(valueTokens, INT32_MIN, INT32_MAX, parsed, outError))
			return false;
		outValue.setValue((int)parsed);
		return true;
	}
	case MikanVariantType::LONG:
	{
		long long parsed= 0;
		if (!parseIntegerInRange(valueTokens, LONG_MIN, LONG_MAX, parsed, outError))
			return false;
		outValue.setValue((long)parsed);
		return true;
	}
	case MikanVariantType::FLOAT:
	{
		double components[1];
		if (!parseComponents(valueTokens, 1, components, outError))
			return false;
		outValue.setValue((float)components[0]);
		return true;
	}
	case MikanVariantType::DOUBLE:
	{
		double components[1];
		if (!parseComponents(valueTokens, 1, components, outError))
			return false;
		outValue.setValue(components[0]);
		return true;
	}
	case MikanVariantType::STRING:
	{
		outValue.setValue(joinTokens(valueTokens, " "));
		return true;
	}
	case MikanVariantType::VECTOR2F:
	{
		double c[2];
		if (!parseComponents(valueTokens, 2, c, outError))
			return false;
		outValue.setValue(MikanVector2f{(float)c[0], (float)c[1]});
		return true;
	}
	case MikanVariantType::VECTOR3F:
	{
		double c[3];
		if (!parseComponents(valueTokens, 3, c, outError))
			return false;
		outValue.setValue(MikanVector3f{(float)c[0], (float)c[1], (float)c[2]});
		return true;
	}
	case MikanVariantType::VECTOR4F:
	{
		double c[4];
		if (!parseComponents(valueTokens, 4, c, outError))
			return false;
		outValue.setValue(MikanVector4f{(float)c[0], (float)c[1], (float)c[2], (float)c[3]});
		return true;
	}
	case MikanVariantType::QUATERNIONF:
	{
		double c[4];
		if (!parseComponents(valueTokens, 4, c, outError))
			return false;
		outValue.setValue(MikanQuatf{(float)c[0], (float)c[1], (float)c[2], (float)c[3]});
		return true;
	}
	case MikanVariantType::MATRIX4F:
	{
		double c[16];
		if (!parseComponents(valueTokens, 16, c, outError))
			return false;

		MikanMatrix4f m;
		float* fields[16]= {&m.x0, &m.x1, &m.x2, &m.x3, &m.y0, &m.y1, &m.y2, &m.y3,
							&m.z0, &m.z1, &m.z2, &m.z3, &m.w0, &m.w1, &m.w2, &m.w3};
		for (size_t i= 0; i < 16; ++i)
			*fields[i]= (float)c[i];
		outValue.setValue(m);
		return true;
	}
	case MikanVariantType::VECTOR2D:
	{
		double c[2];
		if (!parseComponents(valueTokens, 2, c, outError))
			return false;
		outValue.setValue(MikanVector2d{c[0], c[1]});
		return true;
	}
	case MikanVariantType::VECTOR3D:
	{
		double c[3];
		if (!parseComponents(valueTokens, 3, c, outError))
			return false;
		outValue.setValue(MikanVector3d{c[0], c[1], c[2]});
		return true;
	}
	case MikanVariantType::VECTOR4D:
	{
		double c[4];
		if (!parseComponents(valueTokens, 4, c, outError))
			return false;
		outValue.setValue(MikanVector4d{c[0], c[1], c[2], c[3]});
		return true;
	}
	case MikanVariantType::QUATERNIOND:
	{
		double c[4];
		if (!parseComponents(valueTokens, 4, c, outError))
			return false;
		outValue.setValue(MikanQuatd{c[0], c[1], c[2], c[3]});
		return true;
	}
	default:
		outError= std::string("unsupported type ") + mikanVariantTypeToString(dataType) + " for text conversion";
		return false;
	}
}
} // namespace AutomationVariantText
