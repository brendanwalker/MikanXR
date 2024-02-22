#include "GlCommon.h"
#include "GlTexture.h"
#include "Logger.h"
#include "MathTypeConversion.h"
#include "OpenCVManager.h"
#include "DeepNeuralNetwork.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceView.h"

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#include "assert.h"

#include <easy/profiler.h>

#define MIDAS_DNN_MODEL_NAME	"midas_v21_384x384"
#define SMALL_GS_FRAME_HEIGHT	480.f

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
	OpenCVManager* opencvManager,
	VideoSourceViewPtr view,
	unsigned int bufferBitmask,
	unsigned int frameQueueSize)
	: m_videoDisplayMode(eVideoDisplayMode::mode_bgr)
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
	// Synthetic Depth Buffer
	, m_depthDnn() 
	, m_rgbFloatDepthDnnInput(nullptr)
	, m_floatDepthDnnOutput(nullptr)
	, m_floatNormalizedDepth(nullptr)
	, m_gsDepth(nullptr)
	, m_bgrDepth(nullptr)
	, m_bgrUpscaledDepth(nullptr)
	, m_floatDepthTextureMap()
	, m_colorMappedDepthTextureMap()
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
		m_videoTexture = std::make_shared<GlTexture>(
			m_frameWidth,
			m_frameHeight,
			nullptr,
			GL_RGB, // texture format
			GL_BGR); // buffer format
		m_videoTexture->setGenerateMipMap(false);
		m_videoTexture->setPixelBufferObjectMode(GlTexture::PixelBufferObjectMode::DoublePBOWrite);
		m_videoTexture->createTexture();
	}

	// Synthetic depth buffers
	if (bufferBitmask & VIDEO_FRAME_HAS_DEPTH_FLAG)
	{
		m_depthDnn = opencvManager->fetchDeepNeuralNetwork(MIDAS_DNN_MODEL_NAME);
		if (m_depthDnn && m_depthDnn->getInputChannels() == 3 && m_depthDnn->getOutputChannels() == 1)
		{
			int dnnInputWidth = m_depthDnn->getInputWidth();
			int dnnInputHeight = m_depthDnn->getInputHeight();
			int dnnOutputWidth= m_depthDnn->getOutputWidth();
			int dnnOutputHeight = m_depthDnn->getOutputHeight();

			// Input to the DNN is 1, 3-channel RGB float image
			// Output from the DNN is a 1-channel float depth image
			int inputSize[] = {1, 3, dnnInputHeight, dnnInputWidth};
			int outputSize[] = {1, dnnOutputHeight, dnnOutputWidth};
			m_rgbFloatDepthDnnInput = new cv::Mat(4, inputSize, CV_32F);
			m_floatDepthDnnOutput = new cv::Mat(3, outputSize, CV_32F);

			// Normalized float depth output
			m_floatNormalizedDepth = new cv::Mat(dnnOutputHeight, dnnOutputWidth, CV_32F);
			// Grayscale depth output for (8-BPP)
			m_gsDepth = new cv::Mat(dnnOutputHeight, dnnOutputWidth, CV_8UC1);
			// Grayscale depth output for (24-BPP, BGR color format)
			m_bgrDepth = new cv::Mat(dnnOutputHeight, dnnOutputWidth, CV_8UC3);

			// Optional buffers for debugging the depth DNN
			if (bufferBitmask & VIDEO_FRAME_HAS_DEPTH_UPSCALE_FLAG)
			{
				// Scaled up to video frame size depth output for (24-BPP, BGR color format)
				m_bgrUpscaledDepth = new cv::Mat(m_frameHeight, m_frameWidth, CV_8UC3);
			}

			if (bufferBitmask & VIDEO_FRAME_HAS_GL_TEXTURE_FLAG)
			{
				// Used by shaders for frame masking/compositing
				m_floatDepthTextureMap = std::make_shared<GlTexture>(
					m_depthDnn->getOutputWidth(),
					m_depthDnn->getOutputHeight(),
					nullptr,
					GL_R32F, // texture format
					GL_RED); // buffer format
				m_floatDepthTextureMap->setGenerateMipMap(false);
				m_floatDepthTextureMap->setPixelBufferObjectMode(GlTexture::PixelBufferObjectMode::DoublePBOWrite);
				m_floatDepthTextureMap->createTexture();

				// Used by node editor preview of depth
				m_colorMappedDepthTextureMap = std::make_shared<GlTexture>(
					m_depthDnn->getOutputWidth(),
					m_depthDnn->getOutputHeight(),
					nullptr,
					GL_RGB, // texture format
					GL_BGR); // buffer format
				m_colorMappedDepthTextureMap->setGenerateMipMap(false);
				m_colorMappedDepthTextureMap->setPixelBufferObjectMode(GlTexture::PixelBufferObjectMode::DoublePBOWrite);
				m_colorMappedDepthTextureMap->createTexture();
			}
		}
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
	m_videoTexture= nullptr;
	m_distortionTextureMap= nullptr;
	m_floatDepthTextureMap= nullptr;
	m_colorMappedDepthTextureMap= nullptr;

	// Free the depth DNN
	m_depthDnn= nullptr;

	// Free depth cv::mats
	if (m_rgbFloatDepthDnnInput != nullptr)
		delete m_rgbFloatDepthDnnInput;
	if (m_floatNormalizedDepth != nullptr)
		delete m_floatNormalizedDepth;
	if (m_gsDepth != nullptr)
		delete m_gsDepth;
	if (m_bgrDepth != nullptr)
		delete m_bgrDepth;
	if (m_bgrUpscaledDepth != nullptr)
		delete m_bgrUpscaledDepth;
	if (m_floatDepthDnnOutput != nullptr)
		delete m_floatDepthDnnOutput;

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

	// Apply undistortion maps to the video frame (if valid and desired)
	computeUndistortion(bgrSourceBuffer);

	// Compute synthetic depth from the video frame (if valid and desired)
	computeSyntheticDepth(bgrSourceBuffer);

	// Update the video frame display texture
	if (m_videoTexture != nullptr)
	{
		EASY_BLOCK("Copy to Texture");

		switch (m_videoDisplayMode)
		{
		case eVideoDisplayMode::mode_bgr:
			copyOpenCVMatIntoGLTexture(*bgrSourceBuffer, m_videoTexture);
			break;
		case eVideoDisplayMode::mode_undistored:
			if (m_bgrUndistortBuffer != nullptr)
			{
				copyOpenCVMatIntoGLTexture(*m_bgrUndistortBuffer, m_videoTexture);
			}
			break;
		case eVideoDisplayMode::mode_grayscale:
			if (m_bgrGsUndistortBuffer != nullptr)
			{
				copyOpenCVMatIntoGLTexture(*m_bgrGsUndistortBuffer, m_videoTexture);
			}
			break;
		case eVideoDisplayMode::mode_depth:
			if (m_bgrUpscaledDepth != nullptr)
			{
				copyOpenCVMatIntoGLTexture(*m_bgrUpscaledDepth, m_videoTexture);
			}
			break;
		default:
			assert(0 && "unreachable");
			break;
		}
	}

	return true;
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

void VideoFrameDistortionView::computeSyntheticDepth(cv::Mat* bgrSourceBuffer)
{
	if (bgrSourceBuffer == nullptr ||
		m_rgbFloatDepthDnnInput == nullptr || 
		m_floatDepthDnnOutput == nullptr ||
		!m_depthDnn ||
		m_bDepthDisabled)
	{
		return;
	}

	EASY_BLOCK("ComputeSyntheticDepth");

	// https://pyimagesearch.com/2017/11/06/deep-learning-opencvs-blobfromimage-works/
	// Convert the BGR video frame to a RGB float blob resized for the DNN input
	{
		EASY_BLOCK("blobFromImage");
		cv::dnn::blobFromImage(
			*bgrSourceBuffer, // in: Source image
			*m_rgbFloatDepthDnnInput, // out: DNN input blob (1x3xWxH image matrix)
			1 / 255.f, // Normalize pixel values to [0,1] range
			cv::Size(m_depthDnn->getInputWidth(), m_depthDnn->getInputHeight()), // DNN image input size
			cv::Scalar(123.675, 116.28, 103.53), // ImageNet training set mean subtraction constant (see above link)
			true,	// Swap the R and B channels since the DNN expects RGB order
			false); // No need to crop the image after resize
	}

	// Evaluate the DNN to get the depth map
	m_depthDnn->evaluateForwardPass(m_rgbFloatDepthDnnInput, m_floatDepthDnnOutput);

	// Create an accessor Mat(WxH) to read from the DNN output(1xWxH)
	// No actual copy occurs here, just a view into the DNN output buffer
	const std::vector<int32_t> size = {m_floatDepthDnnOutput->size[1], m_floatDepthDnnOutput->size[2]};
	auto outputAccessor = cv::Mat(2, &size[0], CV_32F, m_floatDepthDnnOutput->ptr<float>());

	// Generate an unscaled BGR debug visualization of the depth map, if requested
	if (m_floatNormalizedDepth != nullptr)
	{
		EASY_BLOCK("Normalize Depth");

		// Find the min and max depth values
		double min, max;
		cv::minMaxLoc(outputAccessor, &min, &max);

		// Scale and invert the float depth values from [min,max] to [1.f,0.f] 
		// since the DNN returns larger values for pixels closer to the camera
		// f'(x,y) = (max - f(x,y)) / (max - min)
		const double range = max - min;
		outputAccessor.convertTo(*m_floatNormalizedDepth, CV_32F, -1.0 / range, max / range);
	}

	// Generate an unscaled BGR debug visualization of the depth map, if requested
	if (m_floatNormalizedDepth != nullptr && m_gsDepth != nullptr)
	{
		EASY_BLOCK("FloatToUbyte Depth");

		// Convert the [0.f,1.f] float value to [0,255] ubyte values
		m_floatNormalizedDepth->convertTo(*m_gsDepth, CV_8U, 255.0);
	}

	// Generate an unscaled BGR debug visualization of the depth map, if requested
	if (m_gsDepth != nullptr && m_bgrDepth != nullptr)
	{
		EASY_BLOCK("Colorize Ubyte Depth");

		// Convert the grayscale buffer from 1 to 3 channels (BGR) 
		cv::applyColorMap(*m_gsDepth, *m_bgrDepth, cv::COLORMAP_JET);
	}

	// If requested, copy the color-coded depth map into a texture
	if (m_bgrDepth != nullptr && m_colorMappedDepthTextureMap)
	{
		EASY_BLOCK("BGR depth to texture");

		copyOpenCVMatIntoGLTexture(*m_bgrDepth, m_colorMappedDepthTextureMap);
	}

	// If requested, resize the depth map to the original video frame size
	if (m_bgrDepth != nullptr && m_bgrUpscaledDepth != nullptr)
	{
		EASY_BLOCK("Upscale BGR depth");

		cv::resize(*m_bgrDepth, *m_bgrUpscaledDepth, m_bgrUpscaledDepth->size());
	}

	// Copy the depth map into a texture with normalized float values
	if (m_floatNormalizedDepth)
	{
		EASY_BLOCK("NormalizedFloat to texture");

		copyOpenCVMatIntoGLTexture(*m_floatNormalizedDepth, m_floatDepthTextureMap);
	}
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
	m_distortionTextureMap= nullptr;

	m_videoDisplayMode = eVideoDisplayMode::mode_bgr;

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

			m_distortionTextureMap = std::make_shared<GlTexture>(m_frameWidth, m_frameHeight, (uint8_t *)data, GL_RG32F, GL_RG);
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

void VideoFrameDistortionView::copyOpenCVMatIntoGLTexture(const cv::Mat& mat, GlTexturePtr texture)
{
	size_t bufferSize = mat.step[0] * mat.rows;

	texture->copyBufferIntoTexture(mat.data, bufferSize);
}
