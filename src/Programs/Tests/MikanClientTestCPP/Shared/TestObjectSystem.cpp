#include "TestObjectSystem.h"
#include "TestObject.h"
#include "TestObjectDataStore.h"
#include "Logger.h"
#include "MikanAPI.h"
#include "MikanPropertyRequests.h"

#include <vector>
#include <ranges>

TestObjectSystem::TestObjectSystem(TestObjectDataStore* ownerDataStore) 
	: m_ownerDataStore(ownerDataStore)
{
}

void TestObjectSystem::Initialize(
	const char* inSystemName,
	const char* inComponentClassName,
	TestObjectSystem::TestObjectFactory factory)
{
	m_systemName= inSystemName;
	m_componentClassName= inComponentClassName;
	m_dataObjectFactory= factory;
}

bool TestObjectSystem::FetchAllComponents()
{
	IMikanAPI* MikanAPI= GetOwnerDataStore()->GetMikanAPI();
	bool bUpdatedSystem= false;

	GetComponentListRequest ListRequest;
	ListRequest.ownerSystem.setUtf8Value(m_systemName.c_str());
	ListRequest.componentClassName.setUtf8Value(m_componentClassName.c_str());

	auto ListResponse= MikanAPI->sendRequest(ListRequest).fetchResponse();
	if (ListResponse->resultCode == MikanAPIResult::Success)
	{
		auto ComponentList= std::static_pointer_cast<ComponentListResponse>(ListResponse);
		const auto& newIdList= ComponentList->componentIdList;

		MIKAN_LOG_INFO("FetchAllComponents") << 
			"Fetch Components of class " << m_componentClassName << " from System " << m_systemName;

		auto objectKeysView= m_objectDataTable | std::views::keys;
		std::vector<MikanComponentID> existingIDs(objectKeysView.begin(), objectKeysView.end());
		for (MikanComponentID existingID : existingIDs)
		{
			if (std::find(newIdList.begin(), newIdList.end(), existingID) == newIdList.end())
			{
				MIKAN_LOG_INFO("FetchAllComponents")
					<< "Removed ComponentID " << existingID << " from System " << m_systemName;

				m_objectDataTable.erase(existingID);
			}
		}

		for (MikanComponentID componentId : newIdList)
		{
			ComponentGetValuesRequest valuesRequest;
			valuesRequest.ownerSystem.setUtf8Value(m_systemName.c_str());
			valuesRequest.componentId= componentId;

			auto valuesResponse= MikanAPI->sendRequest(valuesRequest).fetchResponse();
			if (valuesResponse->resultCode == MikanAPIResult::Success)
			{
				auto componentValuesResponse= std::static_pointer_cast<ComponentGetValuesResponse>(valuesResponse);

				MIKAN_LOG_INFO("FetchAllComponents")
					<< "Added new ComponentID " << componentId << " from System " << m_systemName;

				TestObjectPtr newTestObject= m_dataObjectFactory(this);
				newTestObject->Initialize(componentValuesResponse->valuesObject);

				m_objectDataTable.insert({componentId, newTestObject});
			}
			else
			{
				MIKAN_LOG_ERROR("FetchAllComponents") << 
					"Failed to add new ComponentID " << componentId << 
					" from System " << m_systemName <<
					"(ErrorCode " << (int)valuesResponse->resultCode << ")";
			}
		}
	}
	else
	{
		MIKAN_LOG_ERROR("FetchAllComponents")
			<< "Failed to fetch Components of class " << m_componentClassName << " from System " << m_systemName;
	}

	return bUpdatedSystem;
}

void TestObjectSystem::FlushAllComponents() { m_objectDataTable.clear(); }

void TestObjectSystem::HandleMikanConnected() { FetchAllComponents(); }

void TestObjectSystem::HandleMikanDisconnected() { FlushAllComponents(); }

bool TestObjectSystem::HandleComponentListChanged() { return FetchAllComponents(); }

void TestObjectSystem::ApplyMikanValue(
	MikanComponentID componentId,
	const std::string& fieldName,
	const MikanVariant& fieldValue)
{
	auto iter= m_objectDataTable.find(componentId);
	if (iter != m_objectDataTable.end())
	{
		TestObjectPtr objectPtr= iter->second;

		// Try and apply the new value to the component data.
		if (objectPtr->ApplyMikanValue(fieldName, fieldValue))
		{
			MIKAN_LOG_INFO("ApplyMikanValue") << 
				"Applied update to Field " << fieldName << 
				" on Component ID " << componentId << " in System" << m_systemName;
		}
	}
	else
	{
		MIKAN_LOG_INFO("ApplyMikanValue") << "Failed to apply update to Field " << fieldName << " on Component ID "
										  << componentId << " in System" << m_systemName;
	}
}

TestObject* TestObjectSystem::FindObjectById(MikanComponentID componentId)
{
	auto iter= m_objectDataTable.find(componentId);
	if (iter != m_objectDataTable.end())
	{
		return iter->second.get();
	}

	return nullptr;
}

TestObject* TestObjectSystem::FindObjectByName(const std::string& componentName)
{
	for (const auto& pair : m_objectDataTable)
	{
		TestObjectPtr objectPtr= pair.second;

		if (objectPtr->GetComponentName() == componentName)
		{
			return objectPtr.get();
		}
	}

	return nullptr;
}