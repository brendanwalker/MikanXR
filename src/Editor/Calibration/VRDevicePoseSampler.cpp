#include "VRDevicePoseSampler.h"
#include "CameraComponent.h"
#include "MathOpenCV.h"
#include "MathTypeConversion.h"

#include "glm/gtc/quaternion.hpp"

#include <vector>

//-- VRDevicePoseSamplerState -----
struct VRDevicePoseSamplerState
{
	glm::dmat4 vrDeviceXform;
	bool bHasValidXform;

	int desiredSampleCount;
	int capturedSampleCount;
	std::vector<cv::Quatd> cv_poseQuats;
	std::vector<cv::Vec3d> cv_posePositions; // meters

	void init(int sampleCount)
	{
		desiredSampleCount= sampleCount;
		reset();
	}

	void reset()
	{
		bHasValidXform= false;
		capturedSampleCount= 0;
		cv_poseQuats.clear();
		cv_posePositions.clear();
	}
};

//-- VRDevicePoseSampler -----
VRDevicePoseSampler::VRDevicePoseSampler(VRDevicePoseViewPtr poseView, CameraComponentConstPtr cameraContext,
										 int desiredSampleCount)
	: m_state(new VRDevicePoseSamplerState)
	, m_poseView(poseView)
	, m_cameraContext(cameraContext)
{
	m_state->init(desiredSampleCount);
}

VRDevicePoseSampler::~VRDevicePoseSampler() { delete m_state; }

bool VRDevicePoseSampler::hasFinishedSampling() const
{
	return m_state->capturedSampleCount >= m_state->desiredSampleCount;
}

float VRDevicePoseSampler::getCalibrationProgress() const
{
	return (float)m_state->capturedSampleCount / (float)m_state->desiredSampleCount;
}

void VRDevicePoseSampler::resetCalibrationState() { m_state->reset(); }

bool VRDevicePoseSampler::computeVRDeviceXform()
{
	m_state->bHasValidXform= false;

	CameraComponentConstPtr cam= m_cameraContext.lock();
	if (!cam)
		return false;

	glm::dmat4 poseXform;
	if (!m_poseView->getPose(cam, poseXform))
		return false;

	m_state->vrDeviceXform= poseXform;
	m_state->bHasValidXform= true;

	return true;
}

bool VRDevicePoseSampler::hasValidVRDeviceXform() const { return m_state->bHasValidXform; }

bool VRDevicePoseSampler::sampleLastVRDeviceXform()
{
	if (!m_state->bHasValidXform)
		return false;

	const glm::dquat poseQuat= glm::quat_cast(m_state->vrDeviceXform);
	const glm::dvec3 posePos= glm::dvec3(m_state->vrDeviceXform[3]);

	m_state->cv_poseQuats.push_back(glm_dquat_to_cv_quatd(poseQuat));
	m_state->cv_posePositions.push_back(glm_dvec3_to_cv_vec3d(posePos));
	m_state->capturedSampleCount++;

	return true;
}

bool VRDevicePoseSampler::computeCalibratedDevicePose(MikanQuatd& outRotation, MikanVector3d& outTranslation)
{
	cv::Vec3d cv_avgPosition;
	cv::Quatd cv_avgQuat;

	if (hasFinishedSampling() && opencv_quaternion_compute_average(m_state->cv_poseQuats, cv_avgQuat)
		&& opencv_vec3d_compute_average(m_state->cv_posePositions, cv_avgPosition))
	{
		outRotation= cv_quatd_to_MikanQuatd(cv_avgQuat);
		outTranslation= cv_vec3d_to_MikanVector3d(cv_avgPosition);
		return true;
	}

	return false;
}
