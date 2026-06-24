#pragma once

#include "TestObject.h"
#include "TestObjectSystem.h"
#include "MikanTypeFwd.h"

#include <map>
#include <memory>
#include <functional>
#include <string>

class IMikanAPI;
struct MikanVariant;

class TestObjectSystem;
using TestObjectSystemPtr= std::shared_ptr<TestObjectSystem>;

class TestObjectDataStore
{
public:
	using TestObjectFactory= TestObjectSystem::TestObjectFactory;

	TestObjectDataStore()= default;

	void initialize(IMikanAPI* InMikanAPI);
	inline IMikanAPI* GetMikanAPI() const { return MikanAPI; }

	template <typename MikanDataType, typename TestObjectType>
	void AddTypedObjectSystem()
	{
		AddObjectSystem(MikanDataType::k_ownerSystemName, MikanDataType::k_componentClassName,
						[](TestObjectSystem* OwnerSystem) -> TestObjectPtr
						{ return std::make_shared<TestObjectType>(OwnerSystem); });
	}
	void AddObjectSystem(const char* OwnerSystemName, const char* ComponentClassName, TestObjectFactory Factory);

	inline const std::map<std::string, TestObjectSystemPtr>& GetSystemsTable() const { return SystemsTable; }

	TestObjectSystem* GetObjectSystem(const std::string& SystemName);

	TestObject* FindObject(MikanComponentID ComponentId);
	TestObject* FindObject(const std::string& SystemName, MikanComponentID ComponentId);

	template <typename T>
	T* GetTypedComponentData(const std::string& SystemName, MikanComponentID ComponentId)
	{
		return Cast<T>(FindObject(SystemName, ComponentId));
	}

	void FetchAllComponents();
	void FlushAllComponents();
	void handleMikanConnected();
	void handleMikanDisconnected();
	void HandleListChanged(const std::string& SystemName);
	void handlePropertyUpdateEvent(const struct MikanPropertyUpdateEvent& PropertyUpdateEvent);

protected:
	IMikanAPI* MikanAPI= nullptr;
	std::map<std::string, TestObjectSystemPtr> SystemsTable;
};
