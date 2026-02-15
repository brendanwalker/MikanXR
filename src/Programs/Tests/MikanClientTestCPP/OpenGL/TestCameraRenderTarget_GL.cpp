#include "IMkFrameBuffer.h"
#include "IMkTexture.h"
#include "IMkState.h"
#include "MikanCameraEvents.h"
#include "MikanCameraRequests.h"
#include "MkStateStack.h"
#include "Logger.h"
#include "TestCameraRenderTarget_GL.h"
#include "TestGraphicsContext_GL.h"
#include "TestOpenGLMath.h"

TestCameraRenderTarget_GL::TestCameraRenderTarget_GL(
	TestGraphicsContextPtr ownerContext,
	MikanCameraID cameraId)
	: TestCameraRenderTarget(ownerContext, cameraId)
	, m_renderTargetName("CameraRenderTarget_"+std::to_string(cameraId))
	, m_frameBuffer()
	, m_projMatrix(glm::mat4(1.f))
	, m_viewMatrix(glm::mat4(1.f))
{
}

TestCameraRenderTarget_GL::~TestCameraRenderTarget_GL()
{
	// We should have already called dispose() and cleaned this stuff up before the destructor is called
	assert(m_frameBuffer == nullptr);
	assert(!m_bHasAllocatedRemoteTexture);
}

TestGraphicsContext_GLPtr TestCameraRenderTarget_GL::getOpenGLOwnerContext() const
{
	return std::static_pointer_cast<TestGraphicsContext_GL>(m_ownerContext.lock());
}

IMkTexturePtr TestCameraRenderTarget_GL::getColorTexture() const
{
	if (m_frameBuffer)
	{
		return m_frameBuffer->getColorTexture();
	}

	return nullptr;
}

IMkTexturePtr TestCameraRenderTarget_GL::getDepthTexture() const
{
	if (m_frameBuffer)
	{
		return m_frameBuffer->getDepthTexture();
	}

	return nullptr;
}

bool TestCameraRenderTarget_GL::createGraphicsAPIResources(int textureWidth, int textureHeight)
{
	bool bSuccess = true;

	// Create the render target 
	if (m_frameBuffer == nullptr)
	{
		m_frameBuffer = createMkFrameBuffer(m_renderTargetName);
		m_frameBuffer->setFrameBufferType(IMkFrameBuffer::eFrameBufferType::COLOR_AND_DEPTH);
	}

	// Update render target size and create resources
	m_frameBuffer->setSize(textureWidth, textureHeight);
	if (!m_frameBuffer->isValid())
	{
		bSuccess= m_frameBuffer->createResources();
	}

	return bSuccess;
}

void TestCameraRenderTarget_GL::freeGraphicsAPIResources()
{
	m_width = 0;
	m_height = 0;

	if (m_frameBuffer)
	{
		m_frameBuffer->disposeResources();
		m_frameBuffer= nullptr;
	}
}

void* TestCameraRenderTarget_GL::getGraphicsApiColorTexturePtr() const
{
	if (m_frameBuffer)
	{
		IMkTexturePtr texture= m_frameBuffer->getColorTexture();
		if (texture)
		{
			return texture->getPlatformTexture();
		}
	}

	return nullptr;
}

void* TestCameraRenderTarget_GL::getGraphicsApiDepthTexturePtr() const
{
	if (m_frameBuffer)
	{
		IMkTexturePtr texture = m_frameBuffer->getDepthTexture();
		if (texture)
		{
			return texture->getPlatformTexture();
		}
	}

	return nullptr;
}

void TestCameraRenderTarget_GL::updateCameraViewMatrix(const MikanCameraNewFrameEvent& newFrameEvent)
{
	m_cameraPosition = MikanVector3f_to_glm_vec3(newFrameEvent.camera_position);
	m_cameraForward = MikanVector3f_to_glm_vec3(newFrameEvent.camera_forward);
	m_cameraUp = MikanVector3f_to_glm_vec3(newFrameEvent.camera_up);
	m_cameraRight = glm::normalize(glm::cross(m_cameraForward, m_cameraUp));

	m_viewMatrix = 
		mikan_camera_pose_to_glm_view_matrix(
			newFrameEvent.camera_forward, newFrameEvent.camera_up, newFrameEvent.camera_position);
}

void TestCameraRenderTarget_GL::updateCameraProjectionMatrix(const MikanCameraNewFrameEvent& newFrameEvent)
{
	m_projMatrix = 
		mikan_camera_intrinsics_to_glm_projection_matrix(
			newFrameEvent.focal_length.x, newFrameEvent.focal_length.y, 
			newFrameEvent.principal_point.x, newFrameEvent.principal_point.y, 
			newFrameEvent.pixel_size.x, newFrameEvent.pixel_size.y, 
			newFrameEvent.z_bounds.x, newFrameEvent.z_bounds.y);
}

void TestCameraRenderTarget_GL::bindGraphicsAPIResource()
{
	TestGraphicsContext_GLPtr ownerContext= getOpenGLOwnerContext();

	// Create a new scope to modify rendering state changes to
	assert(m_mkState == nullptr);
	m_mkState = ownerContext->getMkStateStack().pushState(m_renderTargetName);

	// Bind the frame buffer to render to
	assert(!m_frameBuffer->getIsBound());
	m_frameBuffer->bindObject(m_mkState);
}

void TestCameraRenderTarget_GL::unbindGraphicsAPIResource()
{
	TestGraphicsContext_GLPtr ownerContext= getOpenGLOwnerContext();

	// Unbind the frame buffer
	assert(m_frameBuffer->getIsBound());
	m_frameBuffer->unbindObject();

	// Restore all the rendering state we modified
	assert(m_mkState != nullptr);
	assert(m_mkState->getOwnerStateStack().getCurrentStackDepth() == m_mkState->getStackDepth());
	m_mkState->getOwnerStateStack().popState();
	m_mkState= nullptr;
}