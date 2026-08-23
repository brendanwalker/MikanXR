#pragma once

#include "ComponentFwd.h"
#include "MikanTypeFwd.h"

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"

class LightFixtureTriangulator
{
public:
	LightFixtureTriangulator(CameraComponentPtr cameraComponent, class VideoFrameDistortionView* distortionView);
	virtual ~LightFixtureTriangulator();

	bool hasInitialSample() const;
	bool hasTriangulatedPosition() const;
	void resetCalibrationState();

	/// Save the current camera pose as the initial sample pose.
	void sampleCameraPose();

	/// Save the current mouse screen position as a 2D pixel sample.
	void sampleMouseScreenPosition();

	/// Re-compute triangulation from the initial sample and the current mouse + camera pose.
	/// Call every frame during capture position 2 state.
	void computeCurrentTriangulation();

	/// Get the most recently triangulated 3D world position.
	bool getTriangulatedPosition(glm::vec3& outPosition) const;

	// -- Rendering helpers --
	void renderInitialPoint2d();
	void renderCurrentTriangulation();
	void renderTriangulatedPosition();

protected:
	glm::vec2 computeMouseScreenPosition() const;

	float m_frameWidth;
	float m_frameHeight;

	CameraComponentPtr m_cameraComponent;
	class VideoFrameDistortionView* m_distortionView;

	// Internal state (pimpl-style to keep the header clean)
	struct LightFixtureTriangulationState* m_state;
};
