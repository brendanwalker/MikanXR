#include "App.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "IMkGraphicsContext.h"
#include "IMkTexture.h"
#include "IMkTriangulatedMesh.h"
#include "MikanShaderCache.h"
#include "Logger.h"
#include "MathTypeConversion.h"
#include "OpenCVManager.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceComponent.h"

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#include "assert.h"

#include <easy/profiler.h>

#define SMALL_GS_FRAME_HEIGHT	480.f

struct OpenCVMonoCameraIntrinsics
{
	cv::Matx33d distorted_intrinsic_matrix;
	cv::Matx81d distortion_coeffs;
	cv::Matx33d undistorted_intrinsic_matrix;

	void init(int pixelWidth, int pixelHeight, double focalLength)
	{
		// Camera intrinsic matrix with principal point at image center
		double cx = pixelWidth * 0.5;
		double cy = pixelHeight * 0.5;

		distorted_intrinsic_matrix = cv::Matx33d(
			focalLength, 0.0, cx,
			0.0, focalLength, cy,
			0.0, 0.0, 1.0);

		// No distortion coefficients
		distortion_coeffs = cv::Matx81d(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

		// Undistorted intrinsic matrix same as distorted when no distortion
		undistorted_intrinsic_matrix = distorted_intrinsic_matrix;
	}

	void init(int pixelWidth, int pixelHeight)
	{
		// Estimate focal length assuming typical webcam horizontal FOV of 70 degrees
		// focalLength = (pixelWidth / 2) / tan(hfov / 2)
		// For 70 degrees: focalLength ≈ pixelWidth * 1.1917
		double focalLength = pixelWidth * 1.2;

		init(pixelWidth, pixelHeight, focalLength);
	}

	void init(const MikanMonoIntrinsics& monoIntrinsics)
	{
		distorted_intrinsic_matrix = MikanMatrix3d_to_cv_mat33d(monoIntrinsics.distorted_camera_matrix);
		distortion_coeffs = Mikan_distortion_to_cv_vec8(monoIntrinsics.distortion_coefficients);
		undistorted_intrinsic_matrix = MikanMatrix3d_to_cv_mat33d(monoIntrinsics.undistorted_camera_matrix);
	}
};

VideoFrameDistortionView::VideoFrameDistortionView(
	VideoSourceComponentPtr videoSourceComponent,
	unsigned int bufferBitmask,
	unsigned int frameQueueSize)
	: m_videoDisplayMode(eVideoDisplayMode::mode_bgr)
	, m_videoSourceComponent(videoSourceComponent)
	, m_bVideoIsStreaming(false)
	, m_bufferBitmask(bufferBitmask)
	, m_frameWidth(0)
	, m_frameHeight(0)
	, m_fps(0.f)
	// Video frame buffers
	, m_bgrSourceBuffer(nullptr)
	, m_lastVideoFrameReadIndex(0)
	, m_lastFrameTimestamp()
	, m_bgrUndistortBuffer(nullptr)
	// Grayscale video frame buffers
	, m_gsSourceBuffer(nullptr)
	, m_gsUndistortBuffer(nullptr)
	, m_bgrGsDisplayBuffer(nullptr)
	// Camera Intrinsics / Distortion parameters
	, m_intrinsics(new OpenCVMonoCameraIntrinsics)
	// Distortion preview
	, m_distortionMapX(nullptr)
	, m_distortionMapY(nullptr)
	, m_distortionTextureMap(nullptr)
	// Video Frame Texture Queue
	, m_videoFrameQueue(nullptr)
	, m_videoFrameQueueSize(frameQueueSize)
	, m_videoFrameQueueLastWriteIndex(-1)
	, m_videoFrameQueuePendingWriteIndex(0)
{
	// Init Video Frame Texture Queue Entries
	m_videoFrameQueue = new VideoFrameQueueEntry[frameQueueSize];
	for (int queueIndex = 0; queueIndex < frameQueueSize; ++queueIndex)
	{
		VideoFrameQueueEntry& frameEntry = m_videoFrameQueue[queueIndex];

		frameEntry.videoTexture = IMkTexturePtr();
		frameEntry.frameIndex = -1;
	}

	// Get the current video source pixel dimensions
	int pixelWidth = 0;
	int pixelHeight = 0;
	if (videoSourceComponent->getVideoPixelDimensions(pixelWidth, pixelHeight))
	{
		// Get the current camera intrinsics being used by the video source
		MikanVideoSourceIntrinsics mikanIntrinsics;
		m_videoSourceComponent->getCameraIntrinsics(mikanIntrinsics);
		if (mikanIntrinsics.intrinsics_type == MikanIntrinsicsType::MONO_CAMERA_INTRINSICS)
		{
			m_intrinsics->init(mikanIntrinsics.getMonoIntrinsics());
		}
		else
		{
			m_intrinsics->init(pixelWidth, pixelHeight);
			MIKAN_LOG_WARNING("VideoFrameDistortionView")
				<< "VideoSource " << videoSourceComponent->getDevicePath()
				<< " is not distortion calibrated. Using estimated focal length and no distortion.";
		}

		// Resize all desired video frame buffers to match the current video source view size
		// It's possible that the video source doesn't have a valid size yet if it's a stream source
		// So we'll have to resize once the first valid frame is read.
		ensureFrameBufferSize(pixelWidth, pixelHeight);
	}

	// Create a mesh used to render the video frame
	m_fullscreenRGBVideoQuad= createFullscreenQuadMesh(getGraphicsContext(), true);

	// Optionally create a mesh for rendering when no video frame is available
	if (bufferBitmask & VIDEO_FRAME_HAS_NO_VIDEO_SHADER_FLAG)
	{
		MkMaterialConstPtr noVideoMaterial =
			getGraphicsContext()->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_PT_PM5544_TEST_CARD);

		m_fullscreenRGBNoVideoQuad =
			createFullscreenQuadMesh(
				getGraphicsContext(),
				noVideoMaterial,
				false);
	}
}

VideoFrameDistortionView::~VideoFrameDistortionView()
{
	// Free any textures in	the video frame queue
	if (m_videoFrameQueue != nullptr)
	{
		for (int queueIndex = 0; queueIndex < m_videoFrameQueueSize; ++queueIndex)
		{
			VideoFrameQueueEntry& frameEntry = m_videoFrameQueue[queueIndex];

			if (frameEntry.videoTexture)
			{
				frameEntry.videoTexture->disposeTexture();
				frameEntry.videoTexture = IMkTexturePtr();
			}
		}

		delete[] m_videoFrameQueue;
	}

	// Free the texture we were rendering to, if any
	m_distortionTextureMap= nullptr;

	// Video Frame data
	if (m_bgrSourceBuffer != nullptr)
	{
		delete m_bgrSourceBuffer;
	}

	if (m_bgrUndistortBuffer != nullptr)
	{
		delete m_bgrUndistortBuffer;
	}

	// Grayscale video frame buffers
	if (m_gsSourceBuffer != nullptr)
	{
		delete m_gsSourceBuffer;
	}
	if (m_gsUndistortBuffer != nullptr)
	{
		delete m_gsUndistortBuffer;
	}
	if (m_bgrGsDisplayBuffer != nullptr)
	{
		delete m_bgrGsDisplayBuffer;
	}

	// Camera Intrinsics
	delete m_intrinsics;

	// Distortion state
	if (m_distortionMapX != nullptr)
	{
		delete m_distortionMapX;
	}
	if (m_distortionMapY != nullptr)
	{
		delete m_distortionMapY;
	}
}

void VideoFrameDistortionView::ensureFrameBufferSize(int width, int height)
{
	// If the frame size hasn't changed, then bail
	if (m_frameWidth == width && m_frameHeight == height)
	{
		return;
	}

	// Update the frame size
	m_frameWidth = width;
	m_frameHeight = height;

	// Free any existing buffer
	if (m_bgrSourceBuffer != nullptr)
	{
		delete m_bgrSourceBuffer;
	}

	// Allocate a new bgr source buffer
	m_bgrSourceBuffer = new cv::Mat(m_frameHeight, m_frameWidth, CV_8UC3);

	// Distortion state
	if (m_bufferBitmask & VIDEO_FRAME_HAS_BGR_UNDISTORT_FLAG)
	{
		if (m_bgrUndistortBuffer != nullptr)
		{
			delete m_bgrUndistortBuffer;
		}
		m_bgrUndistortBuffer = new cv::Mat(cv::Size(m_frameWidth, m_frameHeight), CV_8UC3);

		if (m_distortionMapX != nullptr)
		{
			delete m_distortionMapX;
		}
		m_distortionMapX = new cv::Mat(cv::Size(m_frameWidth, m_frameHeight), CV_32FC1);

		if (m_distortionMapY != nullptr)
		{
			delete m_distortionMapY;
		}
		m_distortionMapY = new cv::Mat(cv::Size(m_frameWidth, m_frameHeight), CV_32FC1);
	}

	// Grayscale video frame buffers
	if (m_bufferBitmask & VIDEO_FRAME_HAS_GRAYSCALE_FLAG)
	{
		if (m_gsSourceBuffer != nullptr)
		{
			delete m_gsSourceBuffer;
		}
		m_gsSourceBuffer = new cv::Mat(m_frameHeight, m_frameWidth, CV_8UC1);

		if (m_gsUndistortBuffer != nullptr)
		{
			delete m_gsUndistortBuffer;
		}
		m_gsUndistortBuffer = new cv::Mat(m_frameHeight, m_frameWidth, CV_8UC1);

		if (m_bgrGsDisplayBuffer != nullptr)
		{
			delete m_bgrGsDisplayBuffer;
		}
		m_bgrGsDisplayBuffer = new cv::Mat(m_frameHeight, m_frameWidth, CV_8UC3);
	}

	// Create a texture to render the video frame to
	if (m_bufferBitmask & VIDEO_FRAME_HAS_GL_TEXTURE_FLAG)
	{
		// Re-init all video frame queue for the new frame size
		for (int queueIndex = 0; queueIndex < m_videoFrameQueueSize; ++queueIndex)
		{
			VideoFrameQueueEntry& frameEntry = m_videoFrameQueue[queueIndex];

			// Free any existing texture for this queue entry
			if (frameEntry.videoTexture)
			{
				frameEntry.videoTexture->disposeTexture();
			}

			// Allocate a new texture for this queue entry
			IMkTexturePtr videoTexture= CreateMkTexture(
				m_frameWidth,
				m_frameHeight,
				nullptr,
				MK_RGB, // texture format
				MK_BGR); // buffer format
			videoTexture->setGenerateMipMap(false);
			videoTexture->setPixelBufferObjectMode(IMkTexture::PixelBufferObjectMode::DoublePBOWrite);
			videoTexture->createTexture();

			// Update the queue entry with the new texture and reset the frame index
			frameEntry.videoTexture = videoTexture;
			frameEntry.frameIndex = -1;
		}

		// Reset the video frame queue write indices since all frames have been invalidated by the size change
		m_videoFrameQueueLastWriteIndex = -1;
		m_videoFrameQueuePendingWriteIndex = 0;
	}

	// Generate the distortion map for the new frame size
	rebuildDistortionMap();
}

IMkGraphicsContext* VideoFrameDistortionView::getGraphicsContext() const
{
	if (m_videoSourceComponent != nullptr)
	{
		return m_videoSourceComponent->getGraphicsContext();
	}

	return nullptr;
}

bool VideoFrameDistortionView::hasNewVideoFrame() const
{
	return m_videoSourceComponent->hasNewVideoFrameAvailable(VideoFrameSection::Primary);
}

int64_t VideoFrameDistortionView::readNextVideoFrame()
{
	EASY_FUNCTION();

	if (m_videoSourceComponent->getVideoStreamingStatus() == eVideoStreamingStatus::started)
	{
		// Copy the image from the video view
		if (m_videoSourceComponent->hasNewVideoFrameAvailable(VideoFrameSection::Primary))
		{
			const auto now = std::chrono::steady_clock::now();
			const float deltaSeconds = fminf(
				std::chrono::duration<float>(now - m_lastFrameTimestamp).count(),
				0.1f);
			const float fps = deltaSeconds > 0.f ? (1.0f / deltaSeconds) : 0.f;
			m_fps = (m_fps * 0.9f) + (fps * 0.1f);
			m_lastFrameTimestamp = now;

			// Reallocate the frame buffer if the video source has changed resolution
			// (This can happen on streaming video sources)
			int frameWidth, frameHeight;
			m_videoSourceComponent->getVideoPixelDimensions(frameWidth, frameHeight);
			ensureFrameBufferSize(frameWidth, frameHeight);

			// Read the next video frame into the source buffer and update the last read frame index
			m_lastVideoFrameReadIndex = 
				m_videoSourceComponent->readVideoFrameSectionBuffer(
					VideoFrameSection::Primary, m_bgrSourceBuffer);

			m_bVideoIsStreaming = true;
		}
	}
	else
	{
		m_bVideoIsStreaming = false;
	}

	return m_lastVideoFrameReadIndex;
}

void VideoFrameDistortionView::processVideoFrame(int64_t newFrameIndex)
{
	EASY_FUNCTION();

	// Apply undistortion maps to the video frame (if valid and desired)
	computeUndistortion(m_bgrSourceBuffer);

	// Get the next video texture in the queue to write to
	// and record the frame index for that texture
	IMkTexturePtr writeTexture;
	if (m_videoFrameQueue != nullptr && m_videoFrameQueueSize > 0)
	{
		// Get the next video texture in the queue to write to
		writeTexture = m_videoFrameQueue[m_videoFrameQueuePendingWriteIndex].videoTexture;
	
		// Record the frame index for this texture in the queue
		m_videoFrameQueue[m_videoFrameQueuePendingWriteIndex].frameIndex = newFrameIndex;
		
		// Update the queue index of the most recently written video frame
		m_videoFrameQueueLastWriteIndex = m_videoFrameQueuePendingWriteIndex;

		// Advance the pending write index
		m_videoFrameQueuePendingWriteIndex = (m_videoFrameQueuePendingWriteIndex + 1) % m_videoFrameQueueSize;
	}

	// Update the video frame display texture
	if (writeTexture != nullptr)
	{
		EASY_BLOCK("Copy to Texture");

		switch (m_videoDisplayMode)
		{
		case eVideoDisplayMode::mode_bgr:
			copyOpenCVMatIntoGLTexture(*m_bgrSourceBuffer, writeTexture);
			break;
		case eVideoDisplayMode::mode_undistored:
			if (m_bgrUndistortBuffer != nullptr)
			{
				copyOpenCVMatIntoGLTexture(*m_bgrUndistortBuffer, writeTexture);
			}
			break;
		case eVideoDisplayMode::mode_grayscale:
			if (m_bgrGsDisplayBuffer != nullptr)
			{
				copyOpenCVMatIntoGLTexture(*m_bgrGsDisplayBuffer, writeTexture);
			}
			break;
		default:
			assert(0 && "unreachable");
			break;
		}
	}
}
void VideoFrameDistortionView::computeUndistortion(cv::Mat* bgrSourceBuffer)
{
	if (m_bgrUndistortBuffer == nullptr || m_distortionMapX == nullptr || m_distortionMapY == nullptr)
	{
		return;
	}

	EASY_BLOCK("Undistort");

	// Apply the X and Y undistortion maps to create an undistorted 24-BPP image (for display)
	if (!m_bColorUndistortDisabled &&
		m_bgrUndistortBuffer != nullptr)
	{
		EASY_BLOCK("Color Remap");

		cv::remap(
			*bgrSourceBuffer, *m_bgrUndistortBuffer,
			*m_distortionMapX, *m_distortionMapY,
			cv::INTER_LINEAR, cv::BORDER_CONSTANT);
	}

	// Also, optionally do grayscale conversion (and maybe undistortion)
	if (bgrSourceBuffer != nullptr && 
		m_gsSourceBuffer != nullptr &&
		m_bgrGsDisplayBuffer != nullptr)
	{
		if (!m_bGrayscaleUndistortDisabled && m_gsUndistortBuffer != nullptr)
		{
			EASY_BLOCK("Grayscale Convert and Remap");

			cv::cvtColor(*bgrSourceBuffer, *m_gsSourceBuffer, cv::COLOR_BGR2GRAY);
			cv::remap(
				*m_gsSourceBuffer, *m_gsUndistortBuffer,
				*m_distortionMapX, *m_distortionMapY,
				cv::INTER_LINEAR, cv::BORDER_CONSTANT);
			cv::cvtColor(*m_gsUndistortBuffer, *m_bgrGsDisplayBuffer, cv::COLOR_GRAY2BGR);
		}
		else if (m_bGrayscaleUndistortDisabled)
		{
			EASY_BLOCK("Grayscale Convert");

			cv::cvtColor(*bgrSourceBuffer, *m_gsSourceBuffer, cv::COLOR_BGR2GRAY);
			cv::cvtColor(*m_gsSourceBuffer, *m_bgrGsDisplayBuffer, cv::COLOR_GRAY2BGR);
		}
	}
}

int64_t VideoFrameDistortionView::readAndProcessVideoFrame()
{
	EASY_FUNCTION();

	if (hasNewVideoFrame())
	{
		processVideoFrame(readNextVideoFrame());
	}

	return m_lastVideoFrameReadIndex;
}

IMkTexturePtr VideoFrameDistortionView::getVideoTexture(int64_t desiredFrameIndex) const
{
	if (m_videoFrameQueue != nullptr)
	{
		// If a specific frame index was requested, then find it in the queue and return its texture
		if (desiredFrameIndex != -1)
		{
			for (int queueIndex= 0; queueIndex < m_videoFrameQueueSize; queueIndex++)
			{
				const VideoFrameQueueEntry& queueEntry = m_videoFrameQueue[queueIndex];

				if (queueEntry.frameIndex == desiredFrameIndex)
				{
					return queueEntry.videoTexture;
				}
			}
		}

		// If we get here, then either no specific frame index was requested, 
		// or the requested index wasn't found in the queue.
		// In either case, return the most recently written video texture in the queue (if valid)
		if (m_videoFrameQueueLastWriteIndex >= 0)
		{
			return m_videoFrameQueue[m_videoFrameQueueLastWriteIndex].videoTexture;
		}
	}

	return IMkTexturePtr();
}

void VideoFrameDistortionView::applyMonoCameraIntrinsics(
	const struct MikanMonoIntrinsics* instrinsics)
{
	m_intrinsics->init(*instrinsics);
	rebuildDistortionMap();
}

void VideoFrameDistortionView::rebuildDistortionMap()
{
	m_distortionTextureMap= nullptr;

	if (m_distortionMapX != nullptr && m_distortionMapY != nullptr)
	{
		// (Re)create the X and Y undistortion maps used by cv::remap
		cv::initUndistortRectifyMap(
			m_intrinsics->distorted_intrinsic_matrix,
			m_intrinsics->distortion_coeffs,
			cv::noArray(), // unneeded rectification transformation computed by stereoRectify()
			m_intrinsics->undistorted_intrinsic_matrix,
			cv::Size(m_frameWidth, m_frameHeight),
			CV_32FC1, // Distortion map type
			*m_distortionMapX, *m_distortionMapY);

		// Copy the distortion pixel offsets into a texture with normalized float values
		{
			float width = (float)m_frameWidth;
			float height = (float)m_frameHeight;
			float* data= new float[m_frameWidth*m_frameHeight*2];
			size_t write_index = 0;
			for (int y = 0; y < m_frameHeight; ++y)
			{
				for (int x = 0; x < m_frameWidth; ++x)
				{
					data[write_index + 0] = m_distortionMapX->at<float>(y, x) / width;
					data[write_index + 1] = m_distortionMapY->at<float>(y, x) / height;
					write_index+=2;
				}
			}

			m_distortionTextureMap = CreateMkTexture(m_frameWidth, m_frameHeight, (uint8_t *)data, MK_RG32F, MK_RG);
			m_distortionTextureMap->createTexture();

			delete[] data;
		}
	}
}

void VideoFrameDistortionView::renderSelectedVideoBuffers()
{
	if (m_bVideoIsStreaming)
	{
		IMkTexturePtr videoTexture = getVideoTexture();
		if (videoTexture != nullptr && m_fullscreenRGBVideoQuad != nullptr)
		{
			MkMaterialInstancePtr materialInstance = m_fullscreenRGBVideoQuad->getMaterialInstance();
			MkMaterialConstPtr material = materialInstance->getMaterial();

			if (auto materialBinding = material->bindMaterial())
			{
				// Bind the color texture
				materialInstance->setTextureBySemantic(eUniformSemantic::rgbTexture, videoTexture);

				// Draw the color texture
				if (auto materialInstanceBinding = materialInstance->bindMaterialInstance(materialBinding))
				{
					m_fullscreenRGBVideoQuad->drawElements();
				}
			}
		}
	}
	else if (m_fullscreenRGBNoVideoQuad != nullptr)
	{
		MkMaterialInstancePtr materialInstance = m_fullscreenRGBNoVideoQuad->getMaterialInstance();
		MkMaterialConstPtr material = materialInstance->getMaterial();
		if (auto materialBinding = material->bindMaterial())
		{
			//TODO: "Time" and "ScreenSize" are uniforms that all materials
			// should have available by default in the graphics context
			IEditorWindow* ownerWindow= m_videoSourceComponent->getOwnerEditorWindow();
			const double currentTimeSeconds = ownerWindow->getOwnerApp()->getSecondsSinceAppStart();
			const float shaderTime = (float)fmodf(currentTimeSeconds, 1000.0);
			const glm::vec2 screenSize(ownerWindow->getWidth(), ownerWindow->getHeight());
			materialInstance->setVec2BySemantic(eUniformSemantic::screenSize, screenSize);
			materialInstance->setFloatBySemantic(eUniformSemantic::floatConstant0, shaderTime);

			// Draw the "no video" shader
			if (auto materialInstanceBinding = materialInstance->bindMaterialInstance(materialBinding))
			{
				m_fullscreenRGBNoVideoQuad->drawElements();
			}
		}
	}
}

void VideoFrameDistortionView::copyOpenCVMatIntoGLTexture(const cv::Mat& mat, IMkTexturePtr texture)
{
	size_t bufferSize = mat.step[0] * mat.rows;

	texture->copyBufferIntoTexture(mat.data, bufferSize);
}
