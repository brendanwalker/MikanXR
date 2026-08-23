#include "TestTransformObject_GL.h"
#include "TestObjectSystem.h"
#include "TestObjectDataStore.h"

#include "MikanComponentTypes.h"
#include "MikanTransformTypes.h"
#include "MikanVariantTypes.h"
#include "SerializableObjectPtr.h"

TestTransformObject_GL::TestTransformObject_GL(TestObjectSystem* ownerSystem)
	: TestObject(ownerSystem)
	, m_parentTransformId(INVALID_MIKAN_ID)
	, m_relativePosition(0.f)
	, m_relativeRotation(1.f, 0.f, 0.f, 0.f)
	, m_relativeScale(1.f, 1.f, 1.f)
	, m_relativeTransform(1.f)
{
}

void TestTransformObject_GL::Initialize(const Serialization::PolymorphicObjectPtr& inValuesObject)
{
	TestObject::Initialize(inValuesObject);

	const auto* componentValues= inValuesObject.getTypedPointer<MikanTransformComponentValues>();

	m_parentTransformId= componentValues->parent_transform_id;
	m_relativePosition= MikanVector3f_to_glm_vec3(componentValues->relative_position);
	m_relativeRotation= MikanQuatf_to_glm_quat(componentValues->relative_quaternion);
	m_relativeScale= MikanVector3f_to_glm_vec3(componentValues->relative_scale);

	onMikanTransformDataChanged();
	onMikanAttachmentChanged();
}

bool TestTransformObject_GL::ApplyMikanValue(const std::string& fieldName, const MikanVariant& fieldValue)
{
	if (fieldName == "parent_transform_id")
	{
		m_parentTransformId= fieldValue.getIntValue();
		onMikanAttachmentChanged();
		return true;
	}
	else if (fieldName == "relative_position")
	{
		m_relativePosition= MikanVector3f_to_glm_vec3(fieldValue.getVector3fValue());
		onMikanTransformDataChanged();
		return true;
	}
	else if (fieldName == "relative_quaternion")
	{
		m_relativeRotation= MikanQuatf_to_glm_quat(fieldValue.getQuaternionfValue());
		onMikanTransformDataChanged();
		return true;
	}
	else if (fieldName == "relative_scale")
	{
		m_relativeScale= MikanVector3f_to_glm_vec3(fieldValue.getVector3fValue());
		onMikanTransformDataChanged();
		return true;
	}
	else
	{
		return TestObject::ApplyMikanValue(fieldName, fieldValue);
	}
}

void TestTransformObject_GL::onMikanTransformDataChanged()
{
	const glm::mat4 scale= glm::scale(glm::mat4(1.f), m_relativeScale);
	const glm::mat4 rotation= glm::mat4_cast(m_relativeRotation);
	const glm::mat4 translation= glm::translate(glm::mat4(1.f), m_relativePosition);

	m_relativeTransform= translation * (rotation * scale);
}

void TestTransformObject_GL::onMikanAttachmentChanged()
{
	auto* ParentObject= GetOwnerSystem()->GetOwnerDataStore()->FindObject(m_parentTransformId);

	m_parentTransformObject= reinterpret_cast<TestTransformObject_GL*>(ParentObject);
}