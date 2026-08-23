#pragma once

#include "MikanTypeFwd.h"
#include "MikanMathTypes.h"
#include "TestObject.h"
#include "TestOpenGLMath.h"

#include <memory>
#include <string>

class TestTransformObject_GL : public TestObject
{
public:
	TestTransformObject_GL(TestObjectSystem* ownerSystem);

	virtual void Initialize(const Serialization::PolymorphicObjectPtr& InValuesObject) override;
	virtual bool ApplyMikanValue(const std::string& FieldName, const MikanVariant& FieldValue) override;

protected:
	void onMikanTransformDataChanged();
	void onMikanAttachmentChanged();

private:
	MikanTransformID m_parentTransformId;
	glm::vec3 m_relativePosition;
	glm::quat m_relativeRotation;
	glm::vec3 m_relativeScale;

	glm::mat4 m_relativeTransform;
	TestTransformObject_GL* m_parentTransformObject= nullptr;
};