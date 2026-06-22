#include "TestObject.h"
#include "TestObjectSystem.h"

#include "MikanComponentTypes.h"
#include "MikanVariantTypes.h"
#include "SerializableObjectPtr.h"

TestObject::TestObject(TestObjectSystem* ownerSystem) 
	: m_ownerSystem(ownerSystem)
{
}

void TestObject::Initialize(
	const Serialization::PolymorphicObjectPtr& inValuesObject)
{
	const auto* componentValues= inValuesObject.getTypedPointer<MikanComponentValues>();

	m_componentId= componentValues->component_id;
	m_componentName= componentValues->component_name.getUtf8Value();
}

bool TestObject::ApplyMikanValue(
	const std::string& fieldName, 
	const MikanVariant& fieldValue)
{
	if (fieldName == "component_id")
	{
		m_componentId= fieldValue.getIntValue();
		return true;
	}
	else if (fieldName == "component_name")
	{
		m_componentName= fieldValue.getUtf8Value();
		return true;
	}

	return false;
}