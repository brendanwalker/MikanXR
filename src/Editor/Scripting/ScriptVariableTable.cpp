#include "ScriptVariableTable.h"
#include "Logger.h"

#include <stdexcept>

namespace
{
const char* k_typeKey= "type";
const char* k_classKey= "class";
const char* k_valueKey= "value";
const char* k_componentTypeName= "component";

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

// Parse one {"type", "class", "value"} object; false on any malformed piece
bool readEntryFromJSON(const configuru::Config& entryConfig, ScriptVariable& outEntry)
{
	if (!entryConfig.is_object() || !entryConfig.has_key(k_typeKey) || !entryConfig.has_key(k_valueKey)
		|| !entryConfig[k_typeKey].is_string())
	{
		return false;
	}

	const std::string typeName= entryConfig[k_typeKey].as_string();
	if (typeName == k_componentTypeName)
	{
		if (!entryConfig.has_key(k_classKey) || !entryConfig[k_classKey].is_string()
			|| entryConfig[k_classKey].as_string().empty())
		{
			return false;
		}

		outEntry.componentClass= entryConfig[k_classKey].as_string();
		return readVariantFromJSON(MikanVariantType::INT, entryConfig[k_valueKey], outEntry.value);
	}

	MikanVariantType type= MikanVariantType::INVALID;
	outEntry.componentClass.clear();
	return ScriptVariableTable::jsonNameToType(typeName, type)
		   && readVariantFromJSON(type, entryConfig[k_valueKey], outEntry.value);
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

	outValue= it->second.value;
	return true;
}

bool ScriptVariableTable::getEntry(const std::string& name, ScriptVariable& outEntry) const
{
	auto it= m_values.find(name);
	if (it == m_values.end())
		return false;

	outEntry= it->second;
	return true;
}

bool ScriptVariableTable::getOfType(const std::string& name, MikanVariantType type, MikanVariant& outValue) const
{
	auto it= m_values.find(name);
	if (it == m_values.end() || it->second.isComponentReference() || it->second.value.value_type != type)
		return false;

	outValue= it->second.value;
	return true;
}

bool ScriptVariableTable::getComponentId(const std::string& name, const std::string& componentClass,
										 MikanComponentID& outId) const
{
	auto it= m_values.find(name);
	if (it == m_values.end() || it->second.componentClass != componentClass
		|| it->second.value.value_type != MikanVariantType::INT)
	{
		return false;
	}

	outId= it->second.value.getIntValue();
	return true;
}

bool ScriptVariableTable::set(const std::string& name, const MikanVariant& value)
{
	if (!isSupportedType(value.value_type))
		return false;

	auto it= m_values.find(name);
	if (it != m_values.end())
	{
		if (variantsEqual(it->second.value, value))
			return false;

		it->second.value= value;
		return true;
	}

	m_values[name]= ScriptVariable{value, ""};
	return true;
}

bool ScriptVariableTable::setComponentReference(const std::string& name, const std::string& componentClass,
												MikanComponentID id)
{
	if (componentClass.empty())
		return false;

	auto it= m_values.find(name);
	if (it != m_values.end() && it->second.componentClass == componentClass
		&& it->second.value.value_type == MikanVariantType::INT && it->second.value.getIntValue() == id)
	{
		return false;
	}

	m_values[name]= ScriptVariable{MikanVariant(static_cast<int>(id)), componentClass};
	return true;
}

bool ScriptVariableTable::remove(const std::string& name) { return m_values.erase(name) > 0; }

void ScriptVariableTable::clear() { m_values.clear(); }

configuru::Config ScriptVariableTable::writeToJSON() const
{
	configuru::Config pt= configuru::Config::object();

	for (const auto& [name, entry] : m_values)
	{
		configuru::Config entryConfig= configuru::Config::object();
		if (entry.isComponentReference())
		{
			entryConfig[k_typeKey]= k_componentTypeName;
			entryConfig[k_classKey]= entry.componentClass;
		}
		else
		{
			entryConfig[k_typeKey]= typeToJsonName(entry.value.value_type);
		}
		entryConfig[k_valueKey]= writeVariantToJSON(entry.value);
		pt[name]= entryConfig;
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

		ScriptVariable variable;
		if (!readEntryFromJSON(entry.value(), variable))
		{
			MIKAN_LOG_WARNING("ScriptVariableTable::readFromJSON") << "Skipping malformed script variable " << name;
			continue;
		}

		m_values[name]= variable;
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
