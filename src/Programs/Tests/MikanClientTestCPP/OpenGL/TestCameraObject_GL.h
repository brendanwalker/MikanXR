#pragma once

#include "TestTransformObject_GL.h"

class TestCameraObject_GL : public TestTransformObject_GL
{
public: 
	TestCameraObject_GL(TestObjectSystem* ownerSystem);

	virtual void Initialize(const Serialization::PolymorphicObjectPtr& InValuesObject) override;
	virtual bool ApplyMikanValue(const std::string& FieldName, const MikanVariant& FieldValue) override;

protected:
	void updateCameraProjectionMatrix();

private:
	MikanStageID StageId = -1;
	int m_pixelWidth= 0;
	int m_pixelHeight = 0;
	float m_focalLengthX= 0.f;
	float m_focalLengthY = 0.f;
	float m_principalPointX = 0.f;
	float m_principalPointY = 0.f;
	float m_zMin = 0.f;
	float m_zMax = 0.f;

	glm::mat4 m_projMatrix;
};