#include "GlCommon.h"
#include "GlTexture.h"
#include "Logger.h"
#include "MathTypeConversion.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceView.h"

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#include "assert.h"

#include <easy/profiler.h>

#define SMALL_GS_FRAME_HEIGHT 480.f

struct OpenCVMonoCameraIntrinsics
{
	cv::Matx33d intrinsic_matrix;
	cv::Matx81d distortion_coeffs;
	cv::Rect undistortBufferROI;

	void init(const MikanMonoIntrinsics& monoIntrinsics)
	{
		intrinsic_matrix = MikanMatrix3d_to_cv_mat33d(monoIntrinsics.camera_matrix);
		distortion_coeffs = Mikan_distortion_to_cv_vec8(monoIntrinsics.distortion_coefficients);
		
		undistortBufferROI.x = 0;
		undistortBufferROI.y = 0;
		undistortBufferROI.width = monoIntrinsics.pixel_width;
		undistortBufferROI.height = monoIntrinsics.pixel_height;
	}
};

VideoFrameDistortionView::VideoFrameDistortionView(
	VideoSourceViewPtr view,
	unsigned int bufferBitmask,
	unsigned int frameQueueSize)
	: m_videoDisplayMode(mode_bgr)
	, m_videoSourceView(view)
	, m_frameWidth((int)view->getFrameWidth())
	, m_frameHeight((int)view->getFrameHeight())
	, m_gsSmallToSourceScale(1.f)
	// Video frame buffers
	, m_bgrSourceBuffers(nullptr)
	, m_bgrSourceBufferCount(frameQueueSize)
	, m_bgrSourceBufferWriteIndex(0)
	, m_lastVideoFrameReadIndex(0)
	, m_bgrUndistortBuffer(nullptr)
	// Grayscale video frame buffers
	, m_gsSourceBuffer(nullptr)
	, m_gsSmallBuffer(nullptr)
	, m_gsUndistortBuffer(nullptr)
	, m_bgrGsUndistortBuffer(nullptr)
	// Camera Intrinsics / Distortion parameters
	, m_intrinsics(new OpenCVMonoCameraIntrinsics)
	// Distortion preview
	, m_distortionMapX(nullptr)
	, m_distortionMapY(nullptr)
	, m_distortionTextureMap(nullptr)
	, m_videoTexture(nullptr)
{
	// Source Video Frame data
	m_bgrSourceBuffers = new SourceBufferEntry[m_bgrSourceBufferCount];
	for (unsigned int queueIndex = 0; queueIndex < m_bgrSourceBufferCount; ++queueIndex)
	{
		SourceBufferEntry& frameEntry= m_bgrSourceBuffers[queueIndex];

		frameEntry.bgrSourceBuffer = new cv::Mat(m_frameHeight, m_frameWidth, CV_8UC3);
		frameEntry.frameIndex= 0;
	}
	
	// Distortion state
	if (bufferBitmask & VIDEO_FRAME_HAS_BGR_UNDISTORT_FLAG)
	{
		m_bgrUndistortBuffer = new cv::Mat(cv::Size(m_frameWidth, m_frameHeight), CV_8UC3);
		m_distortionMapX = new cv::Mat(cv::Size(m_frameWidth, m_frameHeight), CV_32FC1);
		m_distortionMapY = new cv::Mat(cv::Size(m_frameWidth, m_frameHeight), CV_32FC1);
	}

	// Grayscale video frame buffers
	if (bufferBitmask & VIDEO_FRAME_HAS_GRAYSCALE_FLAG)
	{
		float gsSourceToSmallScale= SMALL_GS_FRAME_HEIGHT / m_frameHeight;
		m_gsSmallToSourceScale = 1.f / gsSourceToSmallScale;
		int smallFrameWidth = ceilf(m_frameWidth * gsSourceToSmallScale);

		m_gsSourceBuffer = new cv::Mat(m_frameHeight, m_frameWidth, CV_8UC1);
		m_gsSmallBuffer = new cv::Mat(SMALL_GS_FRAME_HEIGHT, smallFrameWidth, CV_8UC1);
		m_gsUndistortBuffer = new cv::Mat(m_frameHeight, m_frameWidth, CV_8UC1);
		m_bgrGsUndistortBuffer = new cv::Mat(m_frameHeight, m_frameWidth, CV_8UC3);
	}

	// Create a texture to render the video frame to
	if (bufferBitmask & VIDEO_FRAME_HAS_GL_TEXTURE_FLAG)
	{
		m_videoTexture = new GlTexture(
			m_frameWidth,
			m_frameHeight,
			nullptr,
			GL_RGB, // texture format
			GL_BGR); // buffer format
		m_videoTexture->setGenerateMipMap(false);
		m_videoTexture->setPixelBufferObjectMode(GlTexture::PixelBufferObjectMode::DoublePBOWrite);
		m_videoTexture->createTexture();
	}

	// Generate the distortion map from the current camera intrinsics
	MikanVideoSourceIntrinsics cameraIntrinsics;
	m_videoSourceView->getCameraIntrinsics(cameraIntrinsics);
	assert(cameraIntrinsics.intrinsics_type == MONO_CAMERA_INTRINSICS);
	rebuildDistortionMap(&cameraIntrinsics.intrinsics.mono);
}

VideoFrameDistortionView::~VideoFrameDistortionView()
{
	// Free the texture we were rendering to, if any
	if (m_videoTexture != nullptr)
	{
		delete m_videoTexture;
	}
	if (m_distortionTextureMap != nullptr)
	{
		delete m_distortionTextureMap;
	}

	// Video Frame data
	if (m_bgrSourceBuffers != nullptr)
	{
		for (int queueIndex = 0; queueIndex < m_bgrSourceBufferCount; ++queueIndex)
		{
			delete m_bgrSourceBuffers[queueIndex].bgrSourceBuffer;
		}
		delete[] m_bgrSourceBuffers;
	}
	if (m_bgrUndistortBuffer != nullptr)
	{
		delete m_bgrUndistortBuffer;
	}

	// Grayscale video frame buffers
	if (m_gsSmallBuffer != nullptr)
	{
		delete m_gsSmallBuffer;
	}
	if (m_gsSourceBuffer != nullptr)
	{
		delete m_gsSourceBuffer;
	}
	if (m_gsUndistortBuffer != nullptr)
	{
		delete m_gsUndistortBuffer;
	}
	if (m_bgrGsUndistortBuffer != nullptr)
	{
		delete m_bgrGsUndistortBuffer;
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

bool VideoFrameDistortionView::hasNewVideoFrame() const
{
	return m_videoSourceView->hasNewVideoFrameAvailable(VideoFrameSection::Primary);
}

uint64_t VideoFrameDistortionView::readNextVideoFrame()
{
	EASY_FUNCTION();

	// Copy the image from the video view
	cv::Mat* bgrSourceBuffer = m_bgrSourceBuffers[m_bgrSourceBufferWriteIndex].bgrSourceBuffer;
	if (m_videoSourceView->hasNewVideoFrameAvailable(VideoFrameSection::Primary))
	{
		m_lastVideoFrameReadIndex= m_videoSourceView->readVideoFrameSectionBuffer(VideoFrameSection::Primary, bgrSourceBuffer);
		m_bgrSourceBuffers[m_bgrSourceBufferWriteIndex].frameIndex= m_lastVideoFrameReadIndex;
		m_bgrSourceBufferWriteIndex = (m_bgrSourceBufferWriteIndex + 1) % m_bgrSourceBufferCount;
	}

	return m_lastVideoFrameReadIndex;
}

bool VideoFrameDistortionView::processVideoFrame(uint64_t desiredFrameIndex)
{
	EASY_FUNCTION();

	// Find the queue entry with the matching frame index
	int desiredQueueIndex= -1;
	for (int queueIndex = 0; queueIndex < m_bgrSourceBufferCount; ++queueIndex)
	{
		if (m_bgrSourceBuffers[queueIndex].frameIndex == desiredFrameIndex)
		{
			desiredQueueIndex = queueIndex;
			break;
		}
	}

	// If no queue entry was found with the desired frame index, then bail
	if (desiredQueueIndex == -1)
	{
		MIKAN_LOG_ERROR("VideoFrameDistortionView::undistortVideoFrame") << "Missing expected frame " << desiredFrameIndex;
		for (int queueIndex = 0; queueIndex < m_bgrSourceBufferCount; ++queueIndex)
		{
			MIKAN_LOG_ERROR("VideoFrameDistortionView::undistortVideoFrame") << "   queue: " << m_bgrSourceBuffers[queueIndex].frameIndex;
		}
		return false;
	}

	cv::Mat* bgrSourceBuffer = m_bgrSourceBuffers[desiredQueueIndex].bgrSourceBuffer;

	if (m_bgrUndistortBuffer != nullptr && m_distortionMapX != nullptr && m_distortionMapY != nullptr)
	{
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

		// Also, optionally do grayscale undistortion
		if (!m_bGrayscaleUndistortDisabled &&
			m_gsUndistortBuffer != nullptr && 
			m_gsUndistortBuffer != nullptr && 
			m_bgrGsUndistortBuffer != nullptr)
		{
			EASY_BLOCK("Grayscale Convert and Remap");

			cv::cvtColor(*bgrSourceBuffer, *m_gsSourceBuffer, cv::COLOR_BGR2GRAY);
			cv::remap(
				*m_gsSourceBuffer, *m_gsUndistortBuffer,
				*m_distortionMapX, *m_distortionMapY,
				cv::INTER_LINEAR, cv::BORDER_CONSTANT);
			cv::cvtColor(*m_gsUndistortBuffer, *m_bgrGsUndistortBuffer, cv::COLOR_GRAY2BGR);

			if (m_gsSmallBuffer != nullptr)
			{
				EASY_BLOCK("Grayscale Resize");

				cv::resize(*m_gsSourceBuffer, *m_gsSmallBuffer, m_gsSmallBuffer->size());
			}
		}
	}

	// Update the video frame display texture
	if (m_videoTexture != nullptr)
	{
		EASY_BLOCK("Copy to Texture");

		switch (m_videoDisplayMode)
		{
		case mode_bgr:
			m_videoTexture->copyBufferIntoTexture(bgrSourceBuffer->data);
			break;
		case mode_undistored:
			if (m_bgrUndistortBuffer != nullptr)
			{
				m_videoTexture->copyBufferIntoTexture(m_bgrUndistortBuffer->data);
			}
			break;
		case mode_grayscale:
			if (m_bgrGsUndistortBuffer != nullptr)
			{
				m_videoTexture->copyBufferIntoTexture(m_bgrGsUndistortBuffer->data);
			}
			break;
		default:
			assert(0 && "unreachable");
			break;
		}
	}

	return true;
}

bool VideoFrameDistortionView::readAndProcessVideoFrame()
{
	EASY_FUNCTION();
	bool bIsNewFrame = false;

	if (hasNewVideoFrame())
	{
		const uint64_t newestFrameIndex= readNextVideoFrame();
		bool bUndistortedFrame= processVideoFrame(newestFrameIndex);
		assert(bUndistortedFrame);

		bIsNewFrame= true;
	}

	return bIsNewFrame;
}

void VideoFrameDistortionView::rebuildDistortionMap(
	const MikanMonoIntrinsics* instrinsics)
{
	if (m_distortionTextureMap != nullptr)
	{
		m_distortionTextureMap->disposeTexture();
		delete m_distortionTextureMap;
		m_distortionTextureMap= nullptr;
	}

	m_videoDisplayMode = mode_bgr;

	m_intrinsics->init(*instrinsics);

	if (m_distortionMapX != nullptr && m_distortionMapY != nullptr)
	{
		// Create a modified camera intrinsic matrix to crop out the unwanted border
		cv::Mat optimalIntrinsicMatrix =
			cv::getOptimalNewCameraMatrix(
				m_intrinsics->intrinsic_matrix,
				m_intrinsics->distortion_coeffs,
				cv::Size(m_frameWidth, m_frameHeight),
				0.f, // We want 0% of the garbage border
				cv::Size(m_frameWidth, m_frameHeight),
				&m_intrinsics->undistortBufferROI); // The valid pixel region of the undistortion buffer

		// (Re)create the X and Y undistortion maps used by cv::remap
		cv::initUndistortRectifyMap(
			m_intrinsics->intrinsic_matrix,
			m_intrinsics->distortion_coeffs,
			cv::noArray(), // unneeded rectification transformation computed by stereoRectify()							  
			optimalIntrinsicMatrix,
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

			m_distortionTextureMap = new GlTexture(m_frameWidth, m_frameHeight, (uint8_t *)data, GL_RG32F, GL_RG);
			m_distortionTextureMap->createTexture();

			delete[] data;
		}
	}
}

void VideoFrameDistortionView::renderSelectedVideoBuffers()
{
	if (m_videoTexture != nullptr)
	{
		m_videoTexture->renderFullscreen();
	}
}