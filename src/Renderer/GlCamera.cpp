#include "App.h"
#include "CameraMath.h"
#include "GlCamera.h"
#include "GlViewport.h"
#include "InputManager.h"
#include "MathUtility.h"
#include "Renderer.h"

#include <glm/gtc/matrix_transform.hpp>

#if defined(_WIN32)
	#include <SDL_events.h>
#else
	#include <SDL2/SDL_events.h>
#endif


GlCamera::GlCamera()
{
	Renderer* renderer= App::getInstance()->getRenderer();

	m_viewMatrix = glm::mat4(1.f);
	m_projectionMatrix =
		glm::perspective(
			degrees_to_radians(k_default_camera_vfov),
			renderer->getSDLWindowAspectRatio(),
			k_default_camera_z_near,
			k_default_camera_z_far);

	m_cameraOrbitYawDegrees = 0.0f;
	m_cameraOrbitPitchDegrees = 0.0f;
	m_cameraOrbitRadius = 1.0f;
	m_cameraTarget= glm::vec3(0.0f);
	m_cameraPosition = glm::vec3(0.0f, 0.0f, 100.0f);
	m_isLocked = false;
	setCameraOrbitLocation(m_cameraOrbitYawDegrees, m_cameraOrbitPitchDegrees, m_cameraOrbitRadius);
}

void GlCamera::setCameraOrbitLocation(float yawDegrees, float pitchDegrees, float radius)
{
	m_cameraOrbitYawDegrees = wrap_degrees(yawDegrees);
	m_cameraOrbitPitchDegrees = clampf(pitchDegrees, 0.0f, 90.0f);
	m_cameraOrbitRadius = fmaxf(radius, k_camera_min_zoom);
	recomputeModelViewMatrix();
}

void GlCamera::setCameraPose(const glm::mat4& poseXform)
{
	m_viewMatrix = computeGLMCameraViewMatrix(poseXform);
}

void GlCamera::setCameraOrbitYaw(float yawDegrees)
{
	m_cameraOrbitYawDegrees= yawDegrees;
	recomputeModelViewMatrix();
}

void GlCamera::setCameraOrbitPitch(float pitchDegrees)
{
	m_cameraOrbitPitchDegrees= pitchDegrees;
	recomputeModelViewMatrix();
}

void GlCamera::setCameraOrbitRadius(float radius)
{
	m_cameraOrbitRadius= radius;
	recomputeModelViewMatrix();
}

void GlCamera::adjustCameraOrbitAngles(float deltaYaw, float deltaPitch)
{
	if (!m_isLocked)
	{
		setCameraOrbitLocation(
			m_cameraOrbitYawDegrees + deltaYaw,
			m_cameraOrbitPitchDegrees + deltaPitch,
			m_cameraOrbitRadius);
	}
}

void GlCamera::adjustCameraOrbitRadius(float deltaRadius)
{
	if (!m_isLocked)
	{
		setCameraOrbitLocation(
			m_cameraOrbitYawDegrees,
			m_cameraOrbitPitchDegrees,
			m_cameraOrbitRadius + deltaRadius);
	}
}

void GlCamera::setCameraViewTarget(const glm::vec3& cameraTarget)
{
	m_cameraTarget= cameraTarget;
	recomputeModelViewMatrix();
}

void GlCamera::applyMonoCameraIntrinsics(MikanVideoSourceIntrinsics* cameraIntrinsics)
{
	switch (cameraIntrinsics->intrinsics_type)
	{
	case MONO_CAMERA_INTRINSICS:
		{
			const MikanMonoIntrinsics& monoIntrinsics = cameraIntrinsics->intrinsics.mono;

			m_projectionMatrix =
				glm::perspective(
					degrees_to_radians((float)monoIntrinsics.vfov),
					(float)(monoIntrinsics.pixel_width / monoIntrinsics.pixel_height),
					(float)monoIntrinsics.znear,
					(float)monoIntrinsics.zfar);
		} break;
	case STEREO_CAMERA_INTRINSICS:
		{
			const MikanStereoIntrinsics& stereoIntrinsics = cameraIntrinsics->intrinsics.stereo;

			m_projectionMatrix =
				glm::perspective(
					degrees_to_radians((float)stereoIntrinsics.vfov),
					(float)(stereoIntrinsics.pixel_width / stereoIntrinsics.pixel_height),
					(float)stereoIntrinsics.znear,
					(float)stereoIntrinsics.zfar);
		} break;
	default:
		break;
	} 
}

void GlCamera::resetOrientation()
{
	setCameraOrbitLocation(0.0f, 0.0f, m_cameraOrbitRadius);
}

void GlCamera::reset()
{
	setCameraOrbitLocation(0.0f, 0.0f, k_camera_min_zoom);
}

void GlCamera::recomputeModelViewMatrix()
{
	float yawRadians = degrees_to_radians(m_cameraOrbitYawDegrees);
	float pitchRadians = degrees_to_radians(m_cameraOrbitPitchDegrees);
	float xzRadiusAtPitch = m_cameraOrbitRadius * cosf(pitchRadians);
	m_cameraPosition = glm::vec3(
		m_cameraTarget.x + xzRadiusAtPitch * sinf(yawRadians),
		m_cameraTarget.y + m_cameraOrbitRadius * sinf(pitchRadians),
		m_cameraTarget.z + xzRadiusAtPitch * cosf(yawRadians));

	if (fabsf(m_cameraOrbitPitchDegrees) < 85.0f)
	{
		m_viewMatrix =
			glm::lookAt(
				m_cameraPosition,
				m_cameraTarget, // Look at tracking origin
				glm::vec3(0, 1, 0));    // +Y is up.
	}
	else
	{
		m_viewMatrix =
			glm::lookAt(
				m_cameraPosition,
				m_cameraTarget, // Look at tracking origin
				glm::vec3(sinf(yawRadians), 0.0f, -cosf(yawRadians)));
	}
}

const glm::vec3 GlCamera::getCameraPosition() const
{
	// Assumes no scaling 
	const glm::mat3 rotMat(m_viewMatrix);
	const glm::vec3 d(m_viewMatrix[3]);
	const glm::vec3 position = -d * rotMat;

	return position;
}

const glm::vec3 GlCamera::getCameraRight() const
{
	return glm::vec3(m_viewMatrix[0][0], m_viewMatrix[1][0], m_viewMatrix[2][0]);
}

const glm::vec3 GlCamera::getCameraUp() const
{
	return glm::vec3(m_viewMatrix[0][1], m_viewMatrix[1][1], m_viewMatrix[2][1]);
}

const glm::vec3 GlCamera::getCameraForward() const
{
	// Camera forward is along negative Z-axis
	return glm::vec3(m_viewMatrix[0][2], m_viewMatrix[1][2], m_viewMatrix[2][2]) * -1.f;
}

const glm::mat4 GlCamera::getCameraTransform() const
{
	return
		glm::mat4(
			glm::vec4(getCameraRight(), 0.f),
			glm::vec4(getCameraUp(), 0.f),
			glm::vec4(getCameraForward()*-1.f, 0.f), // Camera forward is along negative Z-axis
			glm::vec4(getCameraPosition(), 1.f));
}

void GlCamera::computeCameraRayThruPixel(
	GlViewportConstPtr viewport,
	const glm::vec2& viewportLocation,
	glm::vec3& outRayOrigin,
	glm::vec3& outRayDirection) const
{
	glm::i32vec2 viewportSize= viewport->getViewportSize();
	const float viewportWidth= (float)viewportSize.x;
	const float viewportHeight= (float)viewportSize.y;

	// https://antongerdelan.net/opengl/raycasting.html
	// Convert the pixel location into normalized device coordinates
	const glm::vec3 ray_nds(
		((2.f * viewportLocation.x) / viewportWidth) - 1.f,
		1.f - ((2.f * viewportLocation.y) / viewportHeight),
		1.f);
	
	// Convert the nds ray into a 4d-clip space ray
	const glm::vec4 ray_clip(ray_nds.x, ray_nds.y, -1.f, 1.0);
	
	// Convert the clip space ray back into an eye space ray
	glm::vec4 ray_eye= glm::inverse(m_projectionMatrix) * ray_clip;
	ray_eye= glm::vec4(ray_eye.x, ray_eye.y, -1.f, 0.f);

	// Convert the eye space ray to world space
	outRayDirection= glm::inverse(m_viewMatrix) * ray_eye;
	outRayOrigin= getCameraPosition();
}