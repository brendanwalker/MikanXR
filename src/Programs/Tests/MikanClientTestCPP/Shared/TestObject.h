#pragma once

#include "MikanTypeFwd.h"

#include <memory>
#include <string>

class TestObjectSystem;
struct MikanVariant;

namespace Serialization
{
class PolymorphicObjectPtr;
};

class TestObject
{
public: 
	TestObject(TestObjectSystem* ownerSystem);

	virtual void Initialize(const Serialization::PolymorphicObjectPtr& InValuesObject);
	virtual bool ApplyMikanValue(const std::string& FieldName, const MikanVariant& FieldValue);

	class TestObjectSystem* GetOwnerSystem() const { return m_ownerSystem; }
	MikanComponentID GetComponentId() const { return m_componentId; }
	const std::string& GetComponentName() const { return m_componentName; }

private:
	TestObjectSystem* m_ownerSystem= nullptr;
	MikanComponentID m_componentId= -1;
	std::string m_componentName;
};

using TestObjectPtr= std::shared_ptr<TestObject>;