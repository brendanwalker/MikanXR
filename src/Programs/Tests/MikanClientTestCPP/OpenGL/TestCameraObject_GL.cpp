#include "TestCameraObject_GL.h"
#include "TestObjectSystem.h"
#include "TestOpenGLMath.h"

#include "MikanCameraTypes.h"
#include "MikanVariantTypes.h"
#include "SerializableObjectPtr.h"

TestCameraObject_GL::TestCameraObject_GL(TestObjectSystem* ownerSystem)
	: TestTransformObject_GL(ownerSystem)
{
}

void TestCameraObject_GL::Initialize(const Serialization::PolymorphicObjectPtr& inValuesObject)
{
	TestTransformObject_GL::Initialize(inValuesObject);

	const auto* componentValues= inValuesObject.getTypedPointer<MikanCameraComponentValues>();

	// m_focalLengthX;
	// m_focalLengthY;
	// m_principalPointX;
	// m_principalPointY;
	// m_pixelWidth;
	// m_pixelHeight;
	// m_zMin;
	// m_zMax;
}

bool TestCameraObject_GL::ApplyMikanValue(const std::string& fieldName, const MikanVariant& fieldValue)
{
	// if (fieldName == "component_id")
	//{
	//	m_componentId= fieldValue.getIntValue();
	//	return true;
	// }

	return false;
}

void TestCameraObject_GL::updateCameraProjectionMatrix()
{
	m_projMatrix= mikan_camera_intrinsics_to_glm_projection_matrix(m_focalLengthX, m_focalLengthY, m_principalPointX,
																   m_principalPointY, m_pixelWidth, m_pixelHeight,
																   m_zMin, m_zMax);
}