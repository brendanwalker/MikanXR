#pragma once

#include "CommonConfig.h"
#include "MikanTypeFwd.h"
#include "MikanVariantTypes.h"

#include <map>
#include <string>

// The definition-side storage a script context consults while a chunk
// registers variables: a stored value wins over the script's default, and a
// default with no stored value is adopted into the store
class IScriptVariableStore
{
public:
	virtual ~IScriptVariableStore()= default;

	virtual bool getScriptVariableOfType(const std::string& name, MikanVariantType type,
										 MikanVariant& outValue) const= 0;
	virtual void setScriptVariable(const std::string& name, const MikanVariant& value)= 0;
	virtual bool getScriptComponentVariable(const std::string& name, const std::string& componentClass,
											MikanComponentID& outComponentId) const= 0;
	virtual void setScriptComponentVariable(const std::string& name, const std::string& componentClass,
											MikanComponentID componentId)= 0;
};

// One persisted script parameter. A component reference is an INT component
// id (INVALID_MIKAN_ID for none) tagged with the class it may point at.
struct ScriptVariable
{
	MikanVariant value;
	std::string componentClass;

	inline bool isComponentReference() const { return !componentClass.empty(); }
};

// One script's persisted parameter values keyed by Lua global name.
// Supported plain types: BOOL, INT, FLOAT, STRING, VECTOR3F.
class ScriptVariableTable
{
public:
	static bool isSupportedType(MikanVariantType type);
	static const char* typeToJsonName(MikanVariantType type);
	static bool jsonNameToType(const std::string& jsonName, MikanVariantType& outType);
	static bool variantsEqual(const MikanVariant& a, const MikanVariant& b);

	inline bool empty() const { return m_values.empty(); }
	bool has(const std::string& name) const;
	bool get(const std::string& name, MikanVariant& outValue) const;
	bool getEntry(const std::string& name, ScriptVariable& outEntry) const;
	// Plain entries only: a component reference never matches a plain type
	bool getOfType(const std::string& name, MikanVariantType type, MikanVariant& outValue) const;
	// The id of a component reference of exactly this class
	bool getComponentId(const std::string& name, const std::string& componentClass, MikanComponentID& outId) const;
	// False when the type is unsupported or the stored value already matches.
	// An existing component reference keeps its class: the class belongs to the
	// registration, the write only carries the id.
	bool set(const std::string& name, const MikanVariant& value);
	// False when the stored reference already has this class and id
	bool setComponentReference(const std::string& name, const std::string& componentClass, MikanComponentID id);
	bool remove(const std::string& name);
	void clear();
	inline const std::map<std::string, ScriptVariable>& getAll() const { return m_values; }

	// Object of {"type": name, "value": ...} entries keyed by variable name; a
	// component reference is {"type": "component", "class": name, "value": id}.
	// A malformed or unknown-type entry is skipped with a warning.
	configuru::Config writeToJSON() const;
	void readFromJSON(const configuru::Config& pt);
	// Single-line JSON of writeToJSON(), the text form the script_variables
	// property carries
	std::string toJsonString() const;
	// False and untouched when the text is not a JSON object
	bool fromJsonString(const std::string& jsonText);

private:
	std::map<std::string, ScriptVariable> m_values;
};
