#pragma once

#include "MikanClientTypes.h"
#include "RendererFwd.h"

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"

#include <string>

enum eCameraMovementMode : int
{
	orbit,
	fly,
	stationary,
};

class GlCamera
{
public:
	GlCamera();
	virtual ~GlCamera() = default;

	void setName(const std::string& name) { m_cameraName = name; }
	const std::string& getName() const { return m_cameraName; }

	const glm::mat4& getProjectionMatrix() const { return m_projectionMatrix; }
	const glm::mat4& getViewMatrix() const { return m_viewMatrix; }
	const glm::mat4 getViewProjectionMatrix() const { return m_projectionMatrix*m_viewMatrix; }

	float getHorizontalFOVDegrees() const { return m_hFOVDegrees; }
	float getVerticalFOVDegrees() const { return m_vFOVDegrees; }
	float getZNear() const { return m_zNear; }
	float getZFar() const { return m_zFar; }

	const glm::vec3 getCameraPositionFromViewMatrix() const;
	const glm::vec3 getCameraForwardFromViewMatrix() const;
	const glm::vec3 getCameraRightFromViewMatrix() const;
	const glm::vec3 getCameraUpFromViewMatrix() const;
	const glm::mat4 getCameraTransformFromViewMatrix() const;

	eCameraMovementMode getCameraMovementMode() const { return m_movementMode; }
	void setCameraMovementMode(eCameraMovementMode mode);

	void applyMonoCameraIntrinsics(MikanVideoSourceIntrinsics* cameraIntrinsics);

	void setCameraTransform(const glm::mat4& poseXform);
	void setPosition(const glm::vec3& location);
	void lookAt(const glm::vec3& target);

	void adjustFlyForward(float distance);
	void adjustFlyRight(float distance);
	void adjustFlyUp(float distance);
	void adjustFlyYaw(float deltaDegrees);
	void adjustFlyPitch(float deltaDegrees);

	void setOrbitLocation(float yawDegrees, float pitchDegrees, float radius);
	void setOrbitYaw(float yawDegrees);
	void setOrbitPitch(float pitchDegrees);
	void setOrbitRadius(float radius);
	void setOrbitTargetPosition(const glm::vec3& cameraTarget);
	void adjustOrbitAngles(float deltaYaw, float deltaPitch);
	void adjustOrbitRadius(float deltaRadius);
	void adjustOrbitTargetPosition(const glm::vec3& deltaTarget);
	
	void computeCameraRayThruPixel(
		GlViewportConstPtr viewportPtr,
		const glm::vec2& pixelLocation,
		glm::vec3& outRayOrigin,
		glm::vec3& outRayDirection) const;

protected:
	void applyStationaryParamsToViewMatrix();
	void applyFlyParamsToViewMatrix();
	void applyOrbitParamsToViewMatrix();

	const float k_default_aspect_ratio = 16.f / 9.f;
	const float k_default_camera_vfov = 35.f;
	const float k_default_camera_z_near = 0.1f;
	const float k_default_camera_z_far = 5000.f;
	const float k_camera_min_zoom = 0.01f;

	std::string m_cameraName;

	float m_hFOVDegrees;
	float m_vFOVDegrees;
	float m_zNear;
	float m_zFar;
	glm::mat4 m_projectionMatrix;
	glm::mat4 m_viewMatrix;

	// Stationary camera parameters
	glm::mat4 m_stationaryTransform;

	// Stationary camera parameters
	float m_flyYawDegrees;
	float m_flyPitchDegrees;
	glm::mat4 m_flyTransform;

	// Orbit camera parameters
	float m_orbitYawDegrees;
	float m_orbitPitchDegrees;
	float m_orbitRadius;
	glm::vec3 m_orbitTargetPosition;
	
	eCameraMovementMode m_movementMode= eCameraMovementMode::orbit;
};
