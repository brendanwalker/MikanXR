#pragma once

#include "MikanClientTypes.h"
#include "RendererFwd.h"

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"

class GlCamera
{
public:
	GlCamera();
	virtual ~GlCamera() = default;

	const glm::mat4& getProjectionMatrix() const { return m_projectionMatrix; }
	const glm::mat4& getModelViewMatrix() const { return m_modelViewMatrix; }
	const glm::mat4 getViewProjectionMatrix() const { return m_projectionMatrix*m_modelViewMatrix; }

	const glm::vec3 getCameraPosition() const;
	const glm::vec3 getCameraForward() const;
	const glm::vec3 getCameraRight() const;
	const glm::vec3 getCameraUp() const;
	const glm::mat4 getCameraTransform() const;
	void computeCameraRayThruPixel(
		GlViewportConstPtr viewportPtr,
		const glm::vec2& pixelLocation,
		glm::vec3& outRayOrigin,
		glm::vec3& outRayDirection) const;

	bool getIsLocked() { return m_isLocked; }
	void setIsLocked(bool locked) { m_isLocked= locked; }
	void setModelViewMatrix(const glm::mat4& modelViewMat) { m_modelViewMatrix= modelViewMat; }
	void setCameraPose(const glm::mat4& poseXform);
	void setCameraOrbitLocation(float yawDegrees, float pitchDegrees, float radius);
	void setCameraOrbitYaw(float yawDegrees);
	void setCameraOrbitPitch(float pitchDegrees);
	void setCameraOrbitRadius(float radius);
	void setCameraViewTarget(const glm::vec3& cameraTarget);
	void adjustCameraOrbitAngles(float deltaYaw, float deltaPitch);
	void adjustCameraOrbitRadius(float deltaRadius);
	void applyMonoCameraIntrinsics(MikanVideoSourceIntrinsics* cameraIntrinsics);
	void recomputeModelViewMatrix();
	void resetOrientation();
	void reset();

protected:
	const float k_default_camera_vfov = 35.f;
	const float k_default_camera_z_near = 0.1f;
	const float k_default_camera_z_far = 5000.f;
	const float k_camera_min_zoom = 0.01f;

	glm::mat4 m_projectionMatrix;
	glm::mat4 m_modelViewMatrix;

	float m_cameraOrbitYawDegrees;
	float m_cameraOrbitPitchDegrees;
	float m_cameraOrbitRadius;
	glm::vec3 m_cameraTarget;
	glm::vec3 m_cameraPosition;
	bool m_isLocked;
};
