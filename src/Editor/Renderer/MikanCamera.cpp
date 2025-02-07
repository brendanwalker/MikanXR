#include "App.h"
#include "CameraMath.h"
#include "MikanCamera.h"
#include "GlViewport.h"
#include "InputManager.h"
#include "MathUtility.h"
#include "MathGLM.h"
#include "MikanVideoSourceTypes.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#if defined(_WIN32)
	#include <SDL_events.h>
#else
	#include <SDL2/SDL_events.h>
#endif


MikanCamera::MikanCamera()
{
	m_viewMatrix = glm::mat4(1.f);

	m_hFOVDegrees= k_default_camera_vfov*k_default_aspect_ratio;
	m_vFOVDegrees= k_default_camera_vfov;
	m_zNear= k_default_camera_z_near;
	m_zFar= k_default_camera_z_far;

	m_projectionMatrix =
		glm::perspective(
			degrees_to_radians(m_vFOVDegrees),
			k_default_aspect_ratio,
			m_zNear,
			m_zFar);

	// Stationary camera parameters
	m_stationaryTransform= glm::mat4(1.f);

	// Fly camera parameters
	m_flyTransform = glm::mat4(1.f);

	// Orbit camera parameters
	m_orbitYawDegrees = 0.0f;
	m_orbitPitchDegrees = 0.0f;
	m_orbitRadius = k_camera_min_zoom;
	m_orbitTargetPosition= glm::vec3(0.0f);

	// Default to fly movement
	m_flyYawDegrees = 0.0f;
	m_flyPitchDegrees = 0.0f;
	m_movementMode = eCameraMovementMode::fly;
	applyFlyParamsToViewMatrix();
}

void MikanCamera::setOrbitLocation(float yawDegrees, float pitchDegrees, float radius)
{
	m_orbitYawDegrees = wrap_degrees(yawDegrees);
	m_orbitPitchDegrees = clampf(pitchDegrees, 0.0f, 90.0f);
	m_orbitRadius = fmaxf(radius, k_camera_min_zoom);
	applyOrbitParamsToViewMatrix();
}

void MikanCamera::setCameraMovementMode(eCameraMovementMode newMode)
{
	if (newMode != m_movementMode)
	{
		switch (newMode)
		{
			case orbit:
				applyOrbitParamsToViewMatrix();
				break;
			case fly:
				applyFlyParamsToViewMatrix();
				break;
			case stationary:
				applyStationaryParamsToViewMatrix();
				break;
		}
		m_movementMode= newMode;
	}
}

void MikanCamera::setCameraTransform(const glm::mat4& xform)
{
	switch (m_movementMode)
	{
		case orbit:
			// Ignored in this movement mode
			break;
		case fly:
			m_flyTransform= xform;
			applyFlyParamsToViewMatrix();
			break;
		case stationary:
			m_stationaryTransform= xform;
			applyStationaryParamsToViewMatrix();
			break;
	}
}

void MikanCamera::setPosition(const glm::vec3& location)
{
	switch (m_movementMode)
	{
		case orbit:
			// Ignored in this movement mode
			break;
		case fly:
			m_flyTransform[3]= glm::vec4(location, 1.f);
			applyFlyParamsToViewMatrix();
			break;
		case stationary:
			m_stationaryTransform[3]= glm::vec4(location, 1.f);
			applyStationaryParamsToViewMatrix();
			break;
	}
}

void MikanCamera::lookAt(const glm::vec3& target)
{
	switch (m_movementMode)
	{
		case orbit:
			// Ignored in this movement mode
			break;
		case fly:
			{
				m_viewMatrix =
					glm::lookAt(
						glm::vec3(m_flyTransform[3]),
						target,
						glm::vec3(0, 1, 0));    // +Y is up.
				m_flyTransform= getCameraTransformFromViewMatrix();
			}
			break;
		case stationary:
			{
				m_viewMatrix =
					glm::lookAt(
						glm::vec3(m_stationaryTransform[3]),
						target,
						glm::vec3(0, 1, 0));    // +Y is up.
				m_flyTransform = getCameraTransformFromViewMatrix();
			}
			break;
	}
}

void MikanCamera::adjustFlyForward(float distance)
{
	if (m_movementMode == eCameraMovementMode::fly)
	{
		const glm::vec3 position= glm_mat4_get_position(m_flyTransform);
		const glm::vec3 delta= -glm_mat4_get_z_axis(m_flyTransform) * distance;
		glm_mat4_set_position(m_flyTransform, position+delta);

		applyFlyParamsToViewMatrix();
	}
}

void MikanCamera::adjustFlyRight(float distance)
{
	if (m_movementMode == eCameraMovementMode::fly)
	{
		const glm::vec3 position = glm_mat4_get_position(m_flyTransform);
		const glm::vec3 delta = glm_mat4_get_x_axis(m_flyTransform) * distance;
		glm_mat4_set_position(m_flyTransform, position + delta);

		applyFlyParamsToViewMatrix();
	}
}

void MikanCamera::adjustFlyUp(float distance)
{
	if (m_movementMode == eCameraMovementMode::fly)
	{
		const glm::vec3 position = glm_mat4_get_position(m_flyTransform);
		const glm::vec3 delta = glm_mat4_get_y_axis(m_flyTransform) * distance;
		glm_mat4_set_position(m_flyTransform, position + delta);

		applyFlyParamsToViewMatrix();
	}
}

void MikanCamera::adjustFlyYaw(float deltaDegrees)
{
	if (m_movementMode == eCameraMovementMode::fly)
	{
		const glm::vec3 position = glm_mat4_get_position(m_flyTransform);

		m_flyYawDegrees = wrap_degrees(m_flyYawDegrees + deltaDegrees);
		const float yawRadians = degrees_to_radians(m_flyYawDegrees);
		const float pitchRadians = degrees_to_radians(m_flyPitchDegrees);
		glm::mat3 yawRot= glm::rotate(glm::mat4(1.f), pitchRadians, glm::vec3(1.f, 0.f, 0.f));
		glm::mat3 pitchRot= glm::rotate(glm::mat4(1.f), yawRadians, glm::vec3(0.f, 1.f, 0.f));
		const glm::mat4 orientation = glm_composite_xform(yawRot, pitchRot);
		
		glm_mat4_set_rotation(m_flyTransform, orientation);
		glm_mat4_set_position(m_flyTransform, position);

		applyFlyParamsToViewMatrix();
	}
}

void MikanCamera::adjustFlyPitch(float deltaDegrees)
{
	if (m_movementMode == eCameraMovementMode::fly)
	{
		const glm::vec3 position = glm_mat4_get_position(m_flyTransform);

		m_flyPitchDegrees = clampf(m_flyPitchDegrees - deltaDegrees, -90.f, 90.f);
		const float yawRadians = degrees_to_radians(m_flyYawDegrees);
		const float pitchRadians = degrees_to_radians(m_flyPitchDegrees);
		glm::mat3 yawRot = glm::rotate(glm::mat4(1.f), pitchRadians, glm::vec3(1.f, 0.f, 0.f));
		glm::mat3 pitchRot = glm::rotate(glm::mat4(1.f), yawRadians, glm::vec3(0.f, 1.f, 0.f));
		const glm::mat4 orientation = glm_composite_xform(yawRot, pitchRot);

		glm_mat4_set_rotation(m_flyTransform, orientation);
		glm_mat4_set_position(m_flyTransform, position);

		applyFlyParamsToViewMatrix();
	}
}

void MikanCamera::setOrbitYaw(float yawDegrees)
{
	m_orbitYawDegrees= yawDegrees;
	applyOrbitParamsToViewMatrix();
}

void MikanCamera::setOrbitPitch(float pitchDegrees)
{
	m_orbitPitchDegrees= pitchDegrees;
	applyOrbitParamsToViewMatrix();
}

void MikanCamera::setOrbitRadius(float radius)
{
	m_orbitRadius= radius;
	applyOrbitParamsToViewMatrix();
}

void MikanCamera::setOrbitTargetPosition(const glm::vec3& cameraTarget)
{
	m_orbitTargetPosition = cameraTarget;
	applyOrbitParamsToViewMatrix();
}

void MikanCamera::adjustOrbitAngles(float deltaYaw, float deltaPitch)
{
	if (m_movementMode == eCameraMovementMode::orbit)
	{
		setOrbitLocation(
			m_orbitYawDegrees + deltaYaw,
			m_orbitPitchDegrees + deltaPitch,
			m_orbitRadius);
	}
}

void MikanCamera::adjustOrbitRadius(float deltaRadius)
{
	if (m_movementMode == eCameraMovementMode::orbit)
	{
		setOrbitLocation(
			m_orbitYawDegrees,
			m_orbitPitchDegrees,
			m_orbitRadius + deltaRadius);
	}
}

void MikanCamera::adjustOrbitTargetPosition(const glm::vec3& deltaTarget)
{
	if (m_movementMode == eCameraMovementMode::orbit)
	{
		m_orbitTargetPosition+= deltaTarget;
		applyOrbitParamsToViewMatrix();
	}
}

void MikanCamera::applyMonoCameraIntrinsics(MikanVideoSourceIntrinsics* cameraIntrinsics)
{
	assert (cameraIntrinsics->intrinsics_type == MikanIntrinsicsType::MONO_CAMERA_INTRINSICS);

	const MikanMonoIntrinsics& monoIntrinsics = cameraIntrinsics->getMonoIntrinsics();
	monoIntrinsics.undistorted_camera_matrix;

	int unusedViewport[4];
	computeOpenGLProjMatFromCameraIntrinsics(
		cameraIntrinsics->getMonoIntrinsics(),
		m_projectionMatrix,
		unusedViewport);

	m_aspectRatio = (float)monoIntrinsics.aspect_ratio;
	m_vFOVDegrees = (float)monoIntrinsics.vfov;
	m_hFOVDegrees = (float)monoIntrinsics.hfov;
	m_zNear = (float)monoIntrinsics.znear;
	m_zFar = (float)monoIntrinsics.zfar;
}

void MikanCamera::applyStationaryParamsToViewMatrix()
{
	m_viewMatrix = computeGLMCameraViewMatrix(m_stationaryTransform);
}

void MikanCamera::applyFlyParamsToViewMatrix()
{
	m_viewMatrix = computeGLMCameraViewMatrix(m_flyTransform);
}

void MikanCamera::applyOrbitParamsToViewMatrix()
{
	const float yawRadians = degrees_to_radians(m_orbitYawDegrees);
	const float pitchRadians = degrees_to_radians(m_orbitPitchDegrees);
	const float xzRadiusAtPitch = m_orbitRadius * cosf(pitchRadians);
	const glm::vec3 cameraPosition(
		m_orbitTargetPosition.x + xzRadiusAtPitch * sinf(yawRadians),
		m_orbitTargetPosition.y + m_orbitRadius * sinf(pitchRadians),
		m_orbitTargetPosition.z + xzRadiusAtPitch * cosf(yawRadians));

	if (fabsf(m_orbitPitchDegrees) < 85.0f)
	{
		m_viewMatrix =
			glm::lookAt(
				cameraPosition,
				m_orbitTargetPosition, // Look at tracking origin
				glm::vec3(0, 1, 0));    // +Y is up.
	}
	else
	{
		m_viewMatrix =
			glm::lookAt(
				cameraPosition,
				m_orbitTargetPosition, // Look at tracking origin
				glm::vec3(sinf(yawRadians), 0.0f, -cosf(yawRadians)));
	}
}

const glm::vec3 MikanCamera::getCameraPositionFromViewMatrix() const
{
	// Assumes no scaling 
	const glm::mat3 rotMat(m_viewMatrix);
	const glm::vec3 d(m_viewMatrix[3]);
	const glm::vec3 position = -d * rotMat;

	return position;
}

const glm::vec3 MikanCamera::getCameraRightFromViewMatrix() const
{
	return glm::vec3(m_viewMatrix[0][0], m_viewMatrix[1][0], m_viewMatrix[2][0]);
}

const glm::vec3 MikanCamera::getCameraUpFromViewMatrix() const
{
	return glm::vec3(m_viewMatrix[0][1], m_viewMatrix[1][1], m_viewMatrix[2][1]);
}

const glm::vec3 MikanCamera::getCameraForwardFromViewMatrix() const
{
	// Camera forward is along negative Z-axis
	return glm::vec3(m_viewMatrix[0][2], m_viewMatrix[1][2], m_viewMatrix[2][2]) * -1.f;
}

const glm::mat4 MikanCamera::getCameraTransformFromViewMatrix() const
{
	return
		glm::mat4(
			glm::vec4(getCameraRightFromViewMatrix(), 0.f),
			glm::vec4(getCameraUpFromViewMatrix(), 0.f),
			glm::vec4(getCameraForwardFromViewMatrix()*-1.f, 0.f), // Camera forward is along negative Z-axis
			glm::vec4(getCameraPositionFromViewMatrix(), 1.f));
}

void MikanCamera::computeCameraRayThruPixel(
	GlViewportConstPtr viewport,
	const glm::vec2& viewportPixelPos,
	glm::vec3& outRayOrigin,
	glm::vec3& outRayDirection) const
{
	glm::i32vec2 viewportSize= viewport->getViewportSize();
	const float viewportWidth= (float)viewportSize.x;
	const float viewportHeight= (float)viewportSize.y;

	// https://antongerdelan.net/opengl/raycasting.html
	// Convert the pixel location into normalized device coordinates
	const glm::vec3 ray_nds(
		((2.f * viewportPixelPos.x) / viewportWidth) - 1.f,
		1.f - ((2.f * viewportPixelPos.y) / viewportHeight),
		1.f);
	
	// Convert the nds ray into a 4d-clip space ray
	const glm::vec4 ray_clip(ray_nds.x, ray_nds.y, -1.f, 1.0);
	
	// Convert the clip space ray back into an eye space ray
	glm::vec4 ray_eye= glm::inverse(m_projectionMatrix) * ray_clip;
	ray_eye= glm::vec4(ray_eye.x, ray_eye.y, -1.f, 0.f);

	// Convert the eye space ray to world space
	outRayDirection= glm::inverse(m_viewMatrix) * ray_eye;
	outRayOrigin= getCameraPositionFromViewMatrix();
}