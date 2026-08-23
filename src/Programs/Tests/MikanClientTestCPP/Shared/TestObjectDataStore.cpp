#include "TestObjectDataStore.h"
#include "TestCameraObject_GL.h"

#include "Logger.h"

#include "MikanCameraTypes.h"
#include "MikanPropertyEvents.h"

void TestObjectDataStore::initialize(IMikanAPI* InMikanAPI)
{
	MikanAPI= InMikanAPI;

	// Anchor
	// AddTypedObjectSystem<MikanAnchorComponentValues, TestAnchorObject>();
	// Shapes
	// AddTypedObjectSystem<MikanQuadShapeComponentValues, TestQuadShapeObject>();
	// AddTypedObjectSystem<MikanBoxShapeComponentValues, TestBoxShapeObject>();
	// AddTypedObjectSystem<MikanModelShapeComponentValues, TestModelShapeObject>();
	// Stencils
	// AddTypedObjectSystem<MikanQuadStencilComponentValues, TestQuadStencilObject>();
	// AddTypedObjectSystem<MikanBoxStencilComponentValues, TestBoxStencilObject>();
	// AddTypedObjectSystem<MikanModelStencilComponentValues, TestModelStencilObject>();
	// Camera
	AddTypedObjectSystem<MikanCameraComponentValues, TestCameraObject_GL>();
	// Lights
	// AddTypedObjectSystem<MikanRGBSpotLightComponentValues, TestRGBSpotLightObject>();
	// AddTypedObjectSystem<MikanRGBPixelGridComponentValues, TestRGBPixelGridObject>();
	// Stage & Scene
	// AddTypedObjectSystem<MikanStageComponentValues, TestStageObject>();
	// AddTypedObjectSystem<MikanSceneComponentValues, TestSceneObject>();
}

void TestObjectDataStore::AddObjectSystem(const char* OwnerSystemName, const char* ComponentClassName,
										  TestObjectFactory Factory)
{
	auto ComponentSystemPtr= std::make_shared<TestObjectSystem>(this);
	ComponentSystemPtr->Initialize(OwnerSystemName, ComponentClassName, Factory);

	SystemsTable.insert({OwnerSystemName, ComponentSystemPtr});
}

TestObjectSystem* TestObjectDataStore::GetObjectSystem(const std::string& SystemName)
{
	auto iter= SystemsTable.find(SystemName);
	if (iter != SystemsTable.end())
	{
		return iter->second.get();
	}

	return nullptr;
}

TestObject* TestObjectDataStore::FindObject(MikanComponentID ComponentId)
{
	if (ComponentId == INVALID_MIKAN_ID)
	{
		return nullptr;
	}

	for (const auto& pair : SystemsTable)
	{
		TestObjectSystemPtr system= pair.second;

		return system->FindObjectById(ComponentId);
	}

	return nullptr;
}

TestObject* TestObjectDataStore::FindObject(const std::string& SystemName, MikanComponentID ComponentId)
{
	TestObjectSystem* system= GetObjectSystem(SystemName);
	if (system)
	{
		return system->FindObjectById(ComponentId);
	}

	return nullptr;
}

void TestObjectDataStore::FetchAllComponents()
{
	for (const auto& pair : SystemsTable)
	{
		pair.second->FetchAllComponents();
	}
}

void TestObjectDataStore::FlushAllComponents()
{
	for (const auto& pair : SystemsTable)
	{
		pair.second->FlushAllComponents();
	}
}

void TestObjectDataStore::handleMikanConnected() { FetchAllComponents(); }

void TestObjectDataStore::handleMikanDisconnected() { FlushAllComponents(); }

void TestObjectDataStore::HandleListChanged(const std::string& SystemName)
{
	TestObjectSystem* system= GetObjectSystem(SystemName);
	if (system)
	{
		system->HandleComponentListChanged();
	}
}

void TestObjectDataStore::handlePropertyUpdateEvent(const struct MikanPropertyUpdateEvent& PropertyUpdateEvent)
{
	const std::string systemName= PropertyUpdateEvent.propertyValue.ownerSystem.getUtf8Value();
	TestObjectSystem* system= GetObjectSystem(systemName);

	if (system)
	{
		MikanComponentID ComponentId= PropertyUpdateEvent.propertyValue.componentId;
		const std::string fieldName= PropertyUpdateEvent.propertyValue.fieldName.getUtf8Value();
		const MikanVariant& FieldValue= PropertyUpdateEvent.propertyValue.fieldValue;

		system->ApplyMikanValue(ComponentId, fieldName, FieldValue);
	}
	else
	{
		MIKAN_LOG_INFO("handlePropertyUpdateEvent") << "Failed to update property. Unknown System " << systemName;
	}
}