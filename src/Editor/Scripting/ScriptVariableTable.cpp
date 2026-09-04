#include "ScriptVariableTable.h"
#include "Logger.h"

#include <stdexcept>

namespace
{
const char* k_typeKey= "type";
const char* k_valueKey= "value";

configuru::FormatOptions makeSingleLineJsonFormat()
{
	configuru::FormatOptions options= configuru::make_json_options();
	options.indentation= "";
	return options;
}

bool readVariantFromJSON(MikanVariantType type, const configuru::Config& valueConfig, MikanVariant& outValue)
{
	switch (type)
	{
	case MikanVariantType::BOOL:
		if (!valueConfig.is_bool())
			return false;
		outValue.setValue(valueConfig.as_bool());
		return true;
	case MikanVariantType::INT:
		if (!valueConfig.is_number())
			return false;
		outValue.setValue(valueConfig.is_int() ? valueConfig.as_integer<int>() : (int)valueConfig.as_float());
		return true;
	case MikanVariantType::FLOAT:
		if (!valueConfig.is_number())
			return false;
		outValue.setValue(valueConfig.as_float());
		return true;
	case MikanVariantType::STRING:
		if (!valueConfig.is_string())
			return false;
		outValue.setValue(valueConfig.as_string());
		return true;
	case MikanVariantType::VECTOR3F:
	{
		if (!valueConfig.is_array() || valueConfig.array_size() != 3)
			return false;

		MikanVector3f v;
		float* components[3]= {&v.x, &v.y, &v.z};
		for (size_t i= 0; i < 3; ++i)
		{
			const configuru::Config& component= valueConfig[i];
			if (!component.is_number())
				return false;
			*components[i]= component.as_float();
		}
		outValue.setValue(v);
		return true;
	}
	default:
		return false;
	}
}

configuru::Config writeVariantToJSON(const MikanVariant& value)
{
	switch (value.value_type)
	{
	case MikanVariantType::BOOL:
		return configuru::Config(value.getBoolValue());
	case MikanVariantType::INT:
		return configuru::Config(value.getIntValue());
	case MikanVariantType::FLOAT:
		return configuru::Config(value.getFloatValue());
	case MikanVariantType::STRING:
		return configuru::Config(std::string(value.getUtf8Value()));
	case MikanVariantType::VECTOR3F:
	{
		const MikanVector3f& v= value.getVector3fValue();
		return configuru::Config::array({v.x, v.y, v.z});
	}
	default:
		return configuru::Config();
	}
}
} // namespace

// -- ScriptVariableTable -----
bool ScriptVariableTable::isSupportedType(MikanVariantType type)
{
	switch (type)
	{
	case MikanVariantType::BOOL:
	case MikanVariantType::INT:
	case MikanVariantType::FLOAT:
	case MikanVariantType::STRING:
	case MikanVariantType::VECTOR3F:
		return true;
	default:
		return false;
	}
}

const char* ScriptVariableTable::typeToJsonName(MikanVariantType type)
{
	switch (type)
	{
	case MikanVariantType::BOOL:
		return "bool";
	case MikanVariantType::INT:
		return "int";
	case MikanVariantType::FLOAT:
		return "float";
	case MikanVariantType::STRING:
		return "string";
	case MikanVariantType::VECTOR3F:
		return "vec3";
	default:
		return "";
	}
}

bool ScriptVariableTable::jsonNameToType(const std::string& jsonName, MikanVariantType& outType)
{
	static const MikanVariantType k_supportedTypes[]= {
		MikanVariantType::BOOL,   MikanVariantType::INT,      MikanVariantType::FLOAT,
		MikanVariantType::STRING, MikanVariantType::VECTOR3F,
	};

	for (MikanVariantType type : k_supportedTypes)
	{
		if (jsonName == typeToJsonName(type))
		{
			outType= type;
			return true;
		}
	}

	return false;
}

bool ScriptVariableTable::variantsEqual(const MikanVariant& a, const MikanVariant& b)
{
	if (a.value_type != b.value_type)
		return false;

	switch (a.value_type)
	{
	case MikanVariantType::BOOL:
		return a.getBoolValue() == b.getBoolValue();
	case MikanVariantType::INT:
		return a.getIntValue() == b.getIntValue();
	case MikanVariantType::FLOAT:
		return a.getFloatValue() == b.getFloatValue();
	case MikanVariantType::STRING:
		return a.getSerializationStringValue() == b.getSerializationStringValue();
	case MikanVariantType::VECTOR3F:
	{
		const MikanVector3f& va= a.getVector3fValue();
		const MikanVector3f& vb= b.getVector3fValue();
		return va.x == vb.x && va.y == vb.y && va.z == vb.z;
	}
	default:
		return false;
	}
}

bool ScriptVariableTable::has(const std::string& name) const { return m_values.find(name) != m_values.end(); }

bool ScriptVariableTable::get(const std::string& name, MikanVariant& outValue) const
{
	auto it= m_values.find(name);
	if (it == m_values.end())
		return false;

	outValue= it->second;
	return true;
}

bool ScriptVariableTable::getOfType(const std::string& name, MikanVariantType type, MikanVariant& outValue) const
{
	auto it= m_values.find(name);
	if (it == m_values.end() || it->second.value_type != type)
		return false;

	outValue= it->second;
	return true;
}

bool ScriptVariableTable::set(const std::string& name, const MikanVariant& value)
{
	if (!isSupportedType(value.value_type))
		return false;

	auto it= m_values.find(name);
	if (it != m_values.end() && variantsEqual(it->second, value))
		return false;

	m_values[name]= value;
	return true;
}

bool ScriptVariableTable::remove(const std::string& name) { return m_values.erase(name) > 0; }

void ScriptVariableTable::clear() { m_values.clear(); }

configuru::Config ScriptVariableTable::writeToJSON() const
{
	configuru::Config pt= configuru::Config::object();

	for (const auto& [name, value] : m_values)
	{
		configuru::Config entry= configuru::Config::object();
		entry[k_typeKey]= typeToJsonName(value.value_type);
		entry[k_valueKey]= writeVariantToJSON(value);
		pt[name]= entry;
	}

	return pt;
}

void ScriptVariableTable::readFromJSON(const configuru::Config& pt)
{
	m_values.clear();

	if (!pt.is_object())
		return;

	for (const auto& entry : pt.as_object())
	{
		const std::string& name= entry.key();
		const configuru::Config& entryConfig= entry.value();

		MikanVariantType type= MikanVariantType::INVALID;
		MikanVariant value;
		const bool bValid= entryConfig.is_object() && entryConfig.has_key(k_typeKey) && entryConfig.has_key(k_valueKey)
						   && entryConfig[k_typeKey].is_string()
						   && jsonNameToType(entryConfig[k_typeKey].as_string(), type)
						   && readVariantFromJSON(type, entryConfig[k_valueKey], value);
		if (!bValid)
		{
			MIKAN_LOG_WARNING("ScriptVariableTable::readFromJSON") << "Skipping malformed script variable " << name;
			continue;
		}

		m_values[name]= value;
	}
}

std::string ScriptVariableTable::toJsonString() const
{
	return configuru::dump_string(writeToJSON(), makeSingleLineJsonFormat());
}

bool ScriptVariableTable::fromJsonString(const std::string& jsonText)
{
	try
	{
		configuru::Config pt= configuru::parse_string(jsonText.c_str(), configuru::JSON, "ScriptVariableTable");
		if (!pt.is_object())
			return false;

		readFromJSON(pt);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}
