#pragma once

#include "MikanTypeFwd.h"

#include <map>
#include <memory>
#include <functional>
#include <string>

struct MikanVariant;

class TestObjectDataStore;

class TestObject;
using TestObjectPtr= std::shared_ptr<TestObject>;

class TestObjectSystem
{
public:
	using TestObjectFactory= std::function<TestObjectPtr(TestObjectSystem*)>;

	TestObjectSystem(TestObjectDataStore* ownerDataStore);

	void Initialize(const char* InSystemName, const char* InComponentClassName, TestObjectFactory Factory);
	bool FetchAllComponents();
	void FlushAllComponents();
	void HandleMikanConnected();
	void HandleMikanDisconnected();
	bool HandleComponentListChanged();
	void ApplyMikanValue(MikanComponentID ComponentId, const std::string& FieldName, const MikanVariant& FieldValue);

	TestObject* FindObjectById(MikanComponentID ComponentId);
	TestObject* FindObjectByName(const std::string& ComponentName);

	class TestObjectDataStore* GetOwnerDataStore() const { return m_ownerDataStore; }
	inline const std::string& GetSystemName() const { return m_systemName; }
	inline const std::string& GetComponentClassName() const { return m_componentClassName; }
	inline const std::map<MikanComponentID, TestObjectPtr>& GetObjectDataTable() const { return m_objectDataTable; }

protected:
	TestObjectDataStore* m_ownerDataStore= nullptr;
	std::string m_systemName;
	std::string m_componentClassName;
	TestObjectFactory m_dataObjectFactory;

	std::map<MikanComponentID, TestObjectPtr> m_objectDataTable;
};
using TestObjectSystemPtr= std::shared_ptr<TestObjectSystem>;