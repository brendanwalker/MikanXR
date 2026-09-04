#include "ScriptVariablePersistenceTests.h"
#include "unit_test.h"

#include "CommonConfig.h"
#include "MikanVariantTypes.h"
#include "MulticastDelegate.h"
#include "Script/ScriptComponent.h"
#include "ScriptVariableTable.h"

#include <assert.h>
#include <memory>
#include <set>
#include <stdio.h>
#include <string>

namespace
{
// MulticastDelegate binds member functions rather than lambdas, so the listener
// has to be an object instead of a capture.
struct PropertyChangeListener
{
	int notificationCount= 0;
	std::set<std::string> changedProperties;

	void onPropertyChanged(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet)
	{
		notificationCount++;
		for (const std::string& propertyName : changedPropertySet.getSet())
			changedProperties.insert(propertyName);
	}
};

ScriptVariableTable makeTestTable()
{
	ScriptVariableTable table;
	table.set("enabled", MikanVariant(true));
	table.set("num_rows", MikanVariant(-3));
	table.set("cone_angle", MikanVariant(27.5f));
	table.set("light_prefix", MikanVariant(std::string("gen_light_")));
	table.set("origin_offset", MikanVariant(MikanVector3f{-1.5f, 0.25f, 2.125f}));
	return table;
}

bool tablesEqual(const ScriptVariableTable& a, const ScriptVariableTable& b)
{
	if (a.getAll().size() != b.getAll().size())
		return false;

	for (const auto& [name, value] : a.getAll())
	{
		MikanVariant other;
		if (!b.get(name, other) || !ScriptVariableTable::variantsEqual(value, other))
			return false;
	}

	return true;
}

ScriptComponentPtr makeScriptComponent()
{
	auto component= std::make_shared<ScriptComponent>(MikanObjectWeakPtr());
	component->setDefinition(std::make_shared<ScriptDefinition>());
	return component;
}
} // namespace

bool run_script_variable_persistence_tests()
{
	UNIT_TEST_MODULE_BEGIN("script_variable_persistence")
	UNIT_TEST_MODULE_CALL_TEST(script_variable_test_table_json_round_trip);
	UNIT_TEST_MODULE_CALL_TEST(script_variable_test_json_string_round_trip);
	UNIT_TEST_MODULE_CALL_TEST(script_variable_test_malformed_entries);
	UNIT_TEST_MODULE_CALL_TEST(script_variable_test_definition_round_trip);
	UNIT_TEST_MODULE_CALL_TEST(script_variable_test_component_property_round_trip);
	UNIT_TEST_MODULE_CALL_TEST(script_variable_test_notifications);
	UNIT_TEST_MODULE_END()
}

bool script_variable_test_table_json_round_trip()
{
	UNIT_TEST_BEGIN("table round-trips every supported type through JSON")

	const ScriptVariableTable source= makeTestTable();
	success= (source.getAll().size() == 5);
	assert(success);

	ScriptVariableTable restored;
	restored.readFromJSON(source.writeToJSON());
	success&= tablesEqual(source, restored);
	assert(success);

	// The typed lookup must reject a type mismatch on a present name
	MikanVariant value;
	success&= restored.getOfType("num_rows", MikanVariantType::INT, value) && value.getIntValue() == -3;
	assert(success);
	success&= !restored.getOfType("num_rows", MikanVariantType::FLOAT, value);
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool script_variable_test_json_string_round_trip()
{
	UNIT_TEST_BEGIN("single-line JSON text round-trips")

	const ScriptVariableTable source= makeTestTable();
	const std::string text= source.toJsonString();

	// One line, since the transaction log and the automation protocol carry it as one value
	success= (text.find('\n') == std::string::npos);
	assert(success);

	ScriptVariableTable restored;
	success&= restored.fromJsonString(text);
	assert(success);
	success&= tablesEqual(source, restored);
	assert(success);

	// An empty table is still a valid object
	ScriptVariableTable empty;
	success&= (empty.toJsonString() == "{}");
	assert(success);
	success&= restored.fromJsonString("{}") && restored.empty();
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool script_variable_test_malformed_entries()
{
	UNIT_TEST_BEGIN("malformed text is rejected and bad entries are skipped")

	ScriptVariableTable table= makeTestTable();

	// The guard test perturbs a string by appending _guard; the table must stay intact
	success= !table.fromJsonString("{}_guard");
	assert(success);
	success&= !table.fromJsonString("[1, 2, 3]");
	assert(success);
	success&= tablesEqual(table, makeTestTable());
	assert(success);

	// An unknown type, a type/value mismatch, and a bare value are skipped, the
	// rest of the entries survive
	const std::string mixed= "{\"good\": {\"type\": \"int\", \"value\": 7},"
							 " \"unknown\": {\"type\": \"quat\", \"value\": 1},"
							 " \"mismatch\": {\"type\": \"vec3\", \"value\": 4},"
							 " \"bare\": 12}";
	success&= table.fromJsonString(mixed);
	assert(success);
	success&= (table.getAll().size() == 1);
	assert(success);
	MikanVariant value;
	success&= table.get("good", value) && value.value_type == MikanVariantType::INT && value.getIntValue() == 7;
	assert(success);

	// Unsupported types never enter the table
	success&= !table.set("bad", MikanVariant(MikanQuatf{1.f, 0.f, 0.f, 0.f}));
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool script_variable_test_definition_round_trip()
{
	UNIT_TEST_BEGIN("definition keeps the script path and the variables together")

	ScriptDefinition source;
	source.setScriptPath(std::filesystem::path("scripts/generate_lights.lua"));
	source.setScriptVariables(makeTestTable());

	ScriptDefinition restored;
	restored.readFromJSON(source.writeToJSON());

	success= (restored.getScriptPath() == source.getScriptPath());
	assert(success);
	success&= tablesEqual(restored.getScriptVariables(), makeTestTable());
	assert(success);

	// A definition with no variables writes no key and reads back empty
	ScriptDefinition bare;
	ScriptDefinition bareRestored;
	bareRestored.setScriptVariables(makeTestTable());
	bareRestored.readFromJSON(bare.writeToJSON());
	success&= bareRestored.getScriptVariables().empty();
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool script_variable_test_component_property_round_trip()
{
	UNIT_TEST_BEGIN("script_variables property re-applies its own text")

	ScriptComponentPtr source= makeScriptComponent();
	source->getScriptDefinition()->setScriptVariables(makeTestTable());

	MikanVariant text;
	success= source->getPropertyValue(ScriptDefinition::k_scriptVariablesPropertyId, text);
	assert(success);
	success&= (text.value_type == MikanVariantType::STRING);
	assert(success);

	ScriptComponentPtr target= makeScriptComponent();
	success&= target->setPropertyValue(ScriptDefinition::k_scriptVariablesPropertyId, text);
	assert(success);
	success&= tablesEqual(target->getScriptDefinition()->getScriptVariables(), makeTestTable());
	assert(success);

	// Malformed text and a non-string value are both refused
	success&=
		!target->setPropertyValue(ScriptDefinition::k_scriptVariablesPropertyId, MikanVariant(std::string("{}_guard")));
	assert(success);
	success&= !target->setPropertyValue(ScriptDefinition::k_scriptVariablesPropertyId, MikanVariant(3));
	assert(success);
	success&= tablesEqual(target->getScriptDefinition()->getScriptVariables(), makeTestTable());
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool script_variable_test_notifications()
{
	UNIT_TEST_BEGIN("variable writes notify exactly script_variables")

	ScriptComponentPtr component= makeScriptComponent();
	ScriptDefinitionPtr definition= component->getScriptDefinition();

	PropertyChangeListener listener;
	definition->OnPropertyChanged+= MakeDelegate(&listener, &PropertyChangeListener::onPropertyChanged);

	// A single variable write
	success= component->setScriptVariable("num_rows", MikanVariant(4));
	assert(success);
	success&= (listener.notificationCount == 1);
	assert(success);
	success&= (listener.changedProperties == std::set<std::string>{ScriptDefinition::k_scriptVariablesPropertyId});
	assert(success);

	// The same value again is a no-op
	success&= component->setScriptVariable("num_rows", MikanVariant(4));
	assert(success);
	success&= (listener.notificationCount == 1);
	assert(success);

	// An unsupported type is refused before reaching the definition
	success&= !component->setScriptVariable("bad", MikanVariant(MikanQuatf{1.f, 0.f, 0.f, 0.f}));
	assert(success);
	success&= (listener.notificationCount == 1);
	assert(success);

	// The whole-table property write notifies once
	MikanVariant text(std::string("{\"cone_angle\": {\"type\": \"float\", \"value\": 12.5}}"));
	success&= component->setPropertyValue(ScriptDefinition::k_scriptVariablesPropertyId, text);
	assert(success);
	success&= (listener.notificationCount == 2);
	assert(success);
	success&= (listener.changedProperties == std::set<std::string>{ScriptDefinition::k_scriptVariablesPropertyId});
	assert(success);

	// The store interface adopts a default only when nothing of that type is stored
	MikanVariant stored;
	success&= !definition->getScriptVariableOfType("cone_angle", MikanVariantType::INT, stored);
	assert(success);
	success&= definition->getScriptVariableOfType("cone_angle", MikanVariantType::FLOAT, stored)
			  && stored.getFloatValue() == 12.5f;
	assert(success);

	definition->OnPropertyChanged-= MakeDelegate(&listener, &PropertyChangeListener::onPropertyChanged);

	UNIT_TEST_COMPLETE()
}
