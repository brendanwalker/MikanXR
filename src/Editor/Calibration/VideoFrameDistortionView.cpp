#include "App.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "IMkGraphicsContext.h"
#include "IMkFrameBuffer.h"
#include "IMkTexture.h"
#include "IMkTriangulatedMesh.h"
#include "MikanShaderCache.h"
#include "Logger.h"
#include "MathTypeConversion.h"
#include "MkScopedObjectBinding.h"
#include "MkStateStack.h"
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
	unsigned int frameQueueSize,
	VideoFrameSection videoFramesection)
	: m_videoFrameSection(videoFramesection)
	, m_videoDisplayMode(eVideoDisplayMode::mode_bgr)
	, m_videoSourceComponent(videoSourceComponent)
	, m_bVideoIsStreaming(false)
	, m_bufferBitmask(bufferBitmask)
	, m_frameWidth(0)
	, m_frameHeight(0)
	, m_fps(0.f)
	// Video frame source buffer (from video thread)
	, m_bgrSourceBuffer(nullptr)
	, m_bgrSourceBufferWidth(0)
	, m_bgrSourceBufferHeight(0)
	, m_lastVideoFrameWriteIndex{0}
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
	IMkShaderCache* shaderCache = getGraphicsContext()->getShaderCache();

	// Init Video Frame Texture Queue Entries
	m_videoFrameQueue = new VideoFrameQueueEntry[frameQueueSize];
	for (int queueIndex = 0; queueIndex < frameQueueSize; ++queueIndex)
	{
		VideoFrameQueueEntry& frameEntry = m_videoFrameQueue[queueIndex];

		frameEntry.videoTexture = IMkTexturePtr();
		frameEntry.frameIndex = -1;
	}
		
	// Create a mesh used to render the video frame
	// (Assigns the INTERNAL_MATERIAL_PT_FULLSCREEN_RGB_TEXTURE material on the mesh)
	m_fullscreenRGBVideoQuad= createFullscreenQuadMesh(getGraphicsContext(), true);

	// Create a special material for to draw when no video is available
	m_noVideoMaterial = shaderCache->getMaterialByName(INTERNAL_MATERIAL_PT_PM5544_TEST_CARD);
	m_noVideoMaterialInstance = createMkMaterialInstance(m_noVideoMaterial);

	// Optionally create a frame buffer for GPU based undistortion rendering
	if (bufferBitmask & VIDEO_FRAME_HAS_GL_BGR_UNDISTORT_FLAG)
	{
		// Create the RGB frame buffer; size and resources are initialized lazily in glComputeUndistortion
		m_undistortionFrameBuffer = createMkFrameBuffer("Undistortion Frame Buffer");
		m_undistortionFrameBuffer->setFrameBufferType(IMkFrameBuffer::eFrameBufferType::COLOR);
		m_undistortionFrameBuffer->setColorFormat(IMkFrameBuffer::eColorFormat::RGB);

		// Setup the undistortion material
		m_undistortionMaterial = shaderCache->getMaterialByName(INTERNAL_MATERIAL_PT_UNDISTORT_FULLSCREEN_RGB_TEXTURE);
		m_undistortMaterialInstance = createMkMaterialInstance(m_undistortionMaterial);
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

	// Update intrinsics first in case the resolution changed
	MikanVideoSourceIntrinsics mikanIntrinsics;
	if (m_videoSourceComponent->getCameraIntrinsics(mikanIntrinsics))
	{
		if (mikanIntrinsics.intrinsics_type == MikanIntrinsicsType::MONO_CAMERA_INTRINSICS)
		{
			m_intrinsics->init(mikanIntrinsics.getMonoIntrinsics());
		}
		else
		{
			//TODO: Handle stereo intrinsics
			assert(false && "Unsupported Intrinsics type");
		}
	}
	else
	{
		m_intrinsics->init(width, height);
		MIKAN_LOG_WARNING("VideoFrameDistortionView")
			<< "VideoSource " << m_videoSourceComponent->getDevicePath()
			<< " is not distortion calibrated. Using estimated focal length and no distortion.";
	}

	// Distortion state
	if (m_bufferBitmask & VIDEO_FRAME_HAS_CV_BGR_UNDISTORT_FLAG)
	{
		if (m_bgrUndistortBuffer != nullptr)
		{
			delete m_bgrUndistortBuffer;
		}
		m_bgrUndistortBuffer = new cv::Mat(cv::Size(m_frameWidth, m_frameHeight), CV_8UC3);
	}
	if (m_bufferBitmask & VIDEO_FRAME_HAS_CV_BGR_UNDISTORT_FLAG ||
		m_bufferBitmask & VIDEO_FRAME_HAS_GL_BGR_UNDISTORT_FLAG)
	{
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
	if (m_bufferBitmask & VIDEO_FRAME_HAS_CV_GRAYSCALE_FLAG)
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

	// Keep the undistortion framebuffer size in sync so it gets re-created on the next render
	if (m_undistortionFrameBuffer)
	{
		m_undistortionFrameBuffer->setSize(m_frameWidth, m_frameHeight);
	}

	// Create a texture to render the video frame to
	if (m_bufferBitmask & VIDEO_FRAME_HAS_GL_TEXTURE_FLAG)
	{
		if (m_bufferBitmask & VIDEO_FRAME_HAS_GL_BGR_UNDISTORT_FLAG)
		{
			// Allocate a gl texture to copy the source video frame into
			// This is only used when doing shader based undistortuin
			m_bgrSourceTextureMap = CreateMkTexture(
				m_frameWidth,
				m_frameHeight,
				nullptr,
				MK_RGB, // texture format
				MK_BGR); // buffer format
			m_bgrSourceTextureMap->setGenerateMipMap(false);
			m_bgrSourceTextureMap->setPixelBufferObjectMode(IMkTexture::PixelBufferObjectMode::DoublePBOWrite);
			m_bgrSourceTextureMap->createTexture();
		}

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

void VideoFrameDistortionView::writeVideoFrame(
	const unsigned char* videoBuffer,
	const cv::Size& bufferDimensions,
	bool bIsFlipped)
{
	EASY_FUNCTION();
	std::lock_guard<std::mutex> bufferLock(m_bgrSourceBufferMutex);

	const int srcBufferWidth = bufferDimensions.width;
	const int srcBufferHeight= bufferDimensions.height;
	const cv::Mat videoBufferMat(
		srcBufferHeight, srcBufferWidth, CV_8UC3, const_cast<unsigned char*>(videoBuffer));

	// (Re)create the target buffer to match source size
	if (m_bgrSourceBufferWidth != srcBufferWidth || m_bgrSourceBufferHeight != srcBufferHeight)
	{
		if (m_bgrSourceBuffer != nullptr)
		{
			delete m_bgrSourceBuffer;
		}

		// Allocate a new bgr source buffer
		m_bgrSourceBuffer = new cv::Mat(srcBufferHeight, srcBufferWidth, CV_8UC3);
		m_bgrSourceBufferWidth = srcBufferWidth;
		m_bgrSourceBufferHeight = srcBufferHeight;
	}

	if (bIsFlipped)
	{
		cv::flip(videoBufferMat, *m_bgrSourceBuffer, +1);
	}
	else
	{
		videoBufferMat.copyTo(*m_bgrSourceBuffer);
	}

	// Atomically increment the frame index on the write thread
	m_lastVideoFrameWriteIndex++;
}

void VideoFrameDistortionView::writeStereoVideoFrameSection(
	const unsigned char* videoBuffer,
	const cv::Size& bufferDimensions,
	const bool bIsFlipped,
	const cv::Rect& bufferBounds)
{
	EASY_FUNCTION();
	std::lock_guard<std::mutex> bufferLock(m_bgrSourceBufferMutex);

	const int srcBufferWidth = bufferDimensions.width;
	const int srcBufferHeight = bufferDimensions.height;
	const cv::Mat videoBufferMat(
		srcBufferHeight, srcBufferWidth, CV_8UC3, const_cast<unsigned char*>(videoBuffer));


	// (Re)create the target buffer to match source size
	const int srcSectionWidth = bufferBounds.size().width;
	const int srcSectionHeight = bufferBounds.size().height;
	if (m_bgrSourceBufferWidth != srcSectionWidth || m_bgrSourceBufferHeight != srcSectionHeight)
	{
		if (m_bgrSourceBuffer != nullptr)
		{
			delete m_bgrSourceBuffer;
		}

		// Allocate a new bgr source buffer
		m_bgrSourceBuffer = new cv::Mat(srcBufferHeight, srcBufferWidth, CV_8UC3);
		m_bgrSourceBufferWidth = srcSectionWidth;
		m_bgrSourceBufferHeight = srcSectionHeight;
	}

	if (bIsFlipped)
	{
		cv::flip(videoBufferMat(bufferBounds), *m_bgrSourceBuffer, +1);
	}
	else
	{
		videoBufferMat(bufferBounds).copyTo(*m_bgrSourceBuffer);
	}

	// Atomically increment the frame index on the write thread
	m_lastVideoFrameWriteIndex++;
}

bool VideoFrameDistortionView::hasNewVideoFrame() const
{
	return m_lastVideoFrameReadIndex != m_lastVideoFrameWriteIndex;
}

int64_t VideoFrameDistortionView::readNextVideoFrameIndex()
{
	EASY_FUNCTION();

	if (m_videoSourceComponent->getVideoStreamingStatus() == eVideoStreamingStatus::started)
	{
		// Copy the image from the video view
		if (hasNewVideoFrame())
		{
			// Update framerate statistics
			const auto now = std::chrono::steady_clock::now();
			const float deltaSeconds = fminf(
				std::chrono::duration<float>(now - m_lastFrameTimestamp).count(),
				0.1f);
			const float fps = deltaSeconds > 0.f ? (1.0f / deltaSeconds) : 0.f;
			m_fps = (m_fps * 0.9f) + (fps * 0.1f);
			m_lastFrameTimestamp = now;

			// Read the next video frame into the source buffer and update the last read frame index
			m_lastVideoFrameReadIndex = m_lastVideoFrameWriteIndex;

			// Flag that we are receiving videoframes
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

	// Lock the source buffer state while we are processing it
	std::lock_guard<std::mutex> bufferLock(m_bgrSourceBufferMutex);

	// Reallocate the frame buffer if the video source has changed resolution
	// (This can happen on streaming video sources)
	ensureFrameBufferSize(m_bgrSourceBufferWidth, m_bgrSourceBufferHeight);

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
			// Copy BGR OpenCV buffer directly to texture with no processing
			copyOpenCVMatIntoGLTexture(*m_bgrSourceBuffer, writeTexture);
			break;
		case eVideoDisplayMode::mode_undistored:
			{
				if (m_undistortionFrameBuffer)
				{
					// Copy BGR OpenCV buffer directly to texture with no processing
					copyOpenCVMatIntoGLTexture(*m_bgrSourceBuffer, m_bgrSourceTextureMap);

					// Compute the undistortion using an OpenGL shader
					glComputeUndistortion(writeTexture);
				}
				else if (m_bgrUndistortBuffer)
				{
					// Apply undistortion to the video frame using OpenCV
					cvComputeUndistortion();

					// Copy the BGR OpenCV frame buffer to the output texture
					copyOpenCVMatIntoGLTexture(*m_bgrUndistortBuffer, writeTexture);
				}
			}
			break;
		case eVideoDisplayMode::mode_grayscale:
			if (m_bgrGsDisplayBuffer != nullptr)
			{
				// Apply undistortion to the video frame using OpenCV
				cvComputeUndistortion();

				// Copy the BGR-Grayscale OpenCV frame buffer to the output texture
				copyOpenCVMatIntoGLTexture(*m_bgrGsDisplayBuffer, writeTexture);
			}
			break;
		default:
			assert(0 && "unreachable");
			break;
		}
	}
}

void VideoFrameDistortionView::glComputeUndistortion(IMkTexturePtr writeTexture)
{
	IMkGraphicsContext* graphicsContext = getGraphicsContext();

	// Point the framebuffer at the target queue slot texture.
	// If the FBO already exists this is a cheap glFramebufferTexture2D re-attach;
	// otherwise the texture is stored and used when createResources() is called below.
	m_undistortionFrameBuffer->attachColorTexture(writeTexture);

	// Lazily create (or re-create after a resolution change) the FBO
	if (!m_undistortionFrameBuffer->isValid())
	{
		if (!m_undistortionFrameBuffer->createResources())
		{
			MIKAN_LOG_ERROR("glComputeUndistortion") << "Failed to create undistortion framebuffer";
			return;
		}
	}

	// Run the undistortion shader on the frame buffer
	{
		MkScopedObjectBinding colorFramebufferBinding(
			graphicsContext->getMkStateStack().getCurrentState(),
			"VideoFrameDistortionView Framebuffer Scope",
			m_undistortionFrameBuffer);

		if (colorFramebufferBinding)
		{
			if (auto materialBinding = m_undistortionMaterial->bindMaterial())
			{
				m_undistortMaterialInstance->setTextureBySemantic(eUniformSemantic::rgbTexture, m_bgrSourceTextureMap);
				m_undistortMaterialInstance->setTextureBySemantic(eUniformSemantic::distortionTexture, m_distortionTextureMap);

				// Draw the color texture
				if (auto materialInstanceBinding = m_undistortMaterialInstance->bindMaterialInstance(materialBinding))
				{
					m_fullscreenRGBVideoQuad->drawElements();
				}
			}
		}
	}
}

void VideoFrameDistortionView::cvComputeUndistortion()
{
	if (m_bgrUndistortBuffer == nullptr || m_distortionMapX == nullptr || m_distortionMapY == nullptr)
	{
		return;
	}

	EASY_BLOCK("OpenCV Undistort");

	// Apply the X and Y undistortion maps to create an undistorted 24-BPP image (for display)
	if (!m_bColorUndistortDisabled &&
		m_bgrUndistortBuffer != nullptr)
	{
		EASY_BLOCK("Color Undistort");

		cv::remap(
			*m_bgrSourceBuffer, *m_bgrUndistortBuffer,
			*m_distortionMapX, *m_distortionMapY,
			cv::INTER_LINEAR, cv::BORDER_CONSTANT);
	}

	// Also, optionally do grayscale conversion (and maybe undistortion)
	if (m_bgrSourceBuffer != nullptr &&
		m_gsSourceBuffer != nullptr &&
		m_bgrGsDisplayBuffer != nullptr)
	{
		if (!m_bGrayscaleUndistortDisabled && m_gsUndistortBuffer != nullptr)
		{
			{
				EASY_BLOCK("BGR -> Grayscale");

				cv::cvtColor(*m_bgrSourceBuffer, *m_gsSourceBuffer, cv::COLOR_BGR2GRAY);
			}

			{
				EASY_BLOCK("BGR -> Grayscale");

				cv::remap(
					*m_gsSourceBuffer, *m_gsUndistortBuffer,
					*m_distortionMapX, *m_distortionMapY,
					cv::INTER_LINEAR, cv::BORDER_CONSTANT);
			}

			{
				EASY_BLOCK("Grayscale -> BGR");

				cv::cvtColor(*m_gsUndistortBuffer, *m_bgrGsDisplayBuffer, cv::COLOR_GRAY2BGR);
			}
		}
		else if (m_bGrayscaleUndistortDisabled)
		{
			{
				EASY_BLOCK("BGR -> Grayscale");

				cv::cvtColor(*m_bgrSourceBuffer, *m_gsSourceBuffer, cv::COLOR_BGR2GRAY);
			}

			{
				EASY_BLOCK("Grayscale -> BGR");

				cv::cvtColor(*m_gsSourceBuffer, *m_bgrGsDisplayBuffer, cv::COLOR_GRAY2BGR);
			}
		}
	}
}

int64_t VideoFrameDistortionView::readAndProcessVideoFrame()
{
	EASY_FUNCTION();

	if (hasNewVideoFrame())
	{
		processVideoFrame(readNextVideoFrameIndex());
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
	if (m_fullscreenRGBVideoQuad != nullptr)
	{
		// Draw the undistorted video texture
		IMkTexturePtr videoTexture = getVideoTexture();
		if (videoTexture != nullptr && m_bVideoIsStreaming)
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
		// Draw the "no-video" shader
		else
		{
			if (auto materialBinding = m_noVideoMaterial->bindMaterial())
			{
				//TODO: "Time" and "ScreenSize" are uniforms that all materials
				// should have available by default in the graphics context
				IEditorWindow* ownerWindow = m_videoSourceComponent->getOwnerEditorWindow();
				const double currentTimeSeconds = ownerWindow->getOwnerApp()->getSecondsSinceAppStart();
				const float shaderTime = (float)fmodf(currentTimeSeconds, 1000.0);
				const glm::vec2 screenSize(ownerWindow->getWidth(), ownerWindow->getHeight());
				m_noVideoMaterialInstance->setVec2BySemantic(eUniformSemantic::screenSize, screenSize);
				m_noVideoMaterialInstance->setFloatBySemantic(eUniformSemantic::floatConstant0, shaderTime);

				// Draw the no-video shader
				if (auto materialInstanceBinding = m_noVideoMaterialInstance->bindMaterialInstance(materialBinding))
				{
					m_fullscreenRGBVideoQuad->drawElements();
				}
			}
		}
	}
}

void VideoFrameDistortionView::copyOpenCVMatIntoGLTexture(const cv::Mat& mat, IMkTexturePtr texture)
{
	size_t bufferSize = mat.step[0] * mat.rows;

	texture->copyBufferIntoTexture(mat.data, bufferSize);
}
