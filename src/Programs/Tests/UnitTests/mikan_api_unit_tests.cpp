//-- includes -----
#include <assert.h>
#include <string>

#include "MikanAPITypes.h"
#include "MikanRemoteControlRequests.h"
#include "TypeRegistry.h"
#include "JsonSerializer.h"
#include "JsonDeserializer.h"

#include "unit_test.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

//-- public interface -----
bool run_mikan_api_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("mikan_api")
		UNIT_TEST_MODULE_CALL_TEST(mikan_api_test_type_registry_build);
		UNIT_TEST_MODULE_CALL_TEST(mikan_api_test_type_registry_lookup);
		UNIT_TEST_MODULE_CALL_TEST(mikan_api_test_type_registry_unknown_returns_null);
		UNIT_TEST_MODULE_CALL_TEST(mikan_api_test_type_name_set_on_construct);
		UNIT_TEST_MODULE_CALL_TEST(mikan_api_test_request_no_type_id_in_json);
		UNIT_TEST_MODULE_CALL_TEST(mikan_api_test_response_round_trip);
	UNIT_TEST_MODULE_END()
}

//-- TypeRegistry tests -----

bool mikan_api_test_type_registry_build()
{
	UNIT_TEST_BEGIN("type registry build")

		Serialization::TypeRegistry::buildFromRfkDatabase();

		// After building, core Mikan types must be registered
		rfk::Struct const* requestStruct =
			Serialization::TypeRegistry::getStructByName("MikanRequest");
		assert(requestStruct != nullptr);

		rfk::Struct const* responseStruct =
			Serialization::TypeRegistry::getStructByName("MikanResponse");
		assert(responseStruct != nullptr);

		rfk::Struct const* eventStruct =
			Serialization::TypeRegistry::getStructByName("MikanEvent");
		assert(eventStruct != nullptr);

	UNIT_TEST_COMPLETE()
}

bool mikan_api_test_type_registry_lookup()
{
	UNIT_TEST_BEGIN("type registry lookup by name")

		Serialization::TypeRegistry::buildFromRfkDatabase();

		// Concrete request type must also be found
		rfk::Struct const* cmdStruct =
			Serialization::TypeRegistry::getStructByName("MikanRemoteControlCommand");
		assert(cmdStruct != nullptr);

		// The returned struct's name must match what we queried
		assert(std::string(cmdStruct->getName()) == "MikanRemoteControlCommand");

		// The returned struct must be usable for dynamic instantiation
		auto instance = cmdStruct->makeSharedInstance<MikanRequest>();
		assert(instance != nullptr);
		assert(std::string(instance->requestTypeName.getValue()) == "MikanRemoteControlCommand");

	UNIT_TEST_COMPLETE()
}

bool mikan_api_test_type_registry_unknown_returns_null()
{
	UNIT_TEST_BEGIN("type registry unknown type returns null")

		Serialization::TypeRegistry::buildFromRfkDatabase();

		rfk::Struct const* unknownStruct =
			Serialization::TypeRegistry::getStructByName("ThisTypeDoesNotExist");
		assert(unknownStruct == nullptr);

	UNIT_TEST_COMPLETE()
}

//-- MikanAPITypes constructor tests -----

bool mikan_api_test_type_name_set_on_construct()
{
	UNIT_TEST_BEGIN("type name set in constructor")

		// Base types set their own name
		MikanRequest request;
		assert(std::string(request.requestTypeName.getValue()) == "MikanRequest");

		MikanResponse response;
		assert(std::string(response.responseTypeName.getValue()) == "MikanResponse");

		MikanEvent event;
		assert(std::string(event.eventTypeName.getValue()) == "MikanEvent");

		// Derived types set the derived type name, not the base name
		MikanRemoteControlCommand cmd;
		assert(std::string(cmd.requestTypeName.getValue()) == "MikanRemoteControlCommand");

	UNIT_TEST_COMPLETE()
}

//-- Wire format tests -----

bool mikan_api_test_request_no_type_id_in_json()
{
	UNIT_TEST_BEGIN("request JSON has typeName but no typeId")

		MikanRemoteControlCommand cmd;
		cmd.command = "test_command";

		std::string jsonString;
		bool bSerialize = Serialization::serializeToJsonString(cmd, jsonString);
		assert(bSerialize);

		json j = json::parse(jsonString);

		// typeName must be present (needed for server routing)
		assert(j.contains("requestTypeName"));
		assert(j["requestTypeName"].get<std::string>() == "MikanRemoteControlCommand");

	UNIT_TEST_COMPLETE()
}

//-- Round-trip deserialization test -----

bool mikan_api_test_response_round_trip()
{
	UNIT_TEST_BEGIN("response JSON round-trip via TypeRegistry")

		Serialization::TypeRegistry::buildFromRfkDatabase();

		// Simulate server producing a response JSON
		MikanResponse expected;
		expected.requestId = 99;
		expected.resultCode = MikanAPIResult::Success;

		std::string jsonString;
		bool bSerialize = Serialization::serializeToJsonString(expected, jsonString);
		assert(bSerialize);

		// Simulate client parsing: first parse a header to extract responseTypeName
		json j = json::parse(jsonString);
		MikanResponse header = {};
		bool bHeader = Serialization::deserializeFromJson(
			j, &header, MikanResponse::staticGetArchetype());
		assert(bHeader);

		// Look up the concrete struct type by name and create an instance
		rfk::Struct const* responseStruct =
			Serialization::TypeRegistry::getStructByName(header.responseTypeName.getValue());
		assert(responseStruct != nullptr);

		auto instance = responseStruct->makeSharedInstance<MikanResponse>();
		assert(instance != nullptr);

		// Deserialize the full response into the dynamically created instance
		bool bDeserialize = Serialization::deserializeFromJson(j, instance.get(), *responseStruct);
		assert(bDeserialize);

		// Verify field values survived the round-trip
		assert(instance->requestId == expected.requestId);
		assert(instance->resultCode == expected.resultCode);
		assert(std::string(instance->responseTypeName.getValue()) ==
			std::string(expected.responseTypeName.getValue()));

	UNIT_TEST_COMPLETE()
}
