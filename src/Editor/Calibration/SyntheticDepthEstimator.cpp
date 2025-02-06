#include "SyntheticDepthEstimator.h"
#include "SdlCommon.h"
#include "IMkTexture.h"
#include "Logger.h"
#include "OpenCVManager.h"
#include "DeepNeuralNetwork.h"

#include "opencv2/opencv.hpp"

#include <easy/profiler.h>

#define MIDAS_DNN_MODEL_NAME	"midas_v21_384x384"

SyntheticDepthEstimator::SyntheticDepthEstimator(
	OpenCVManager* opencvManager,
	int optionBitmask)
	: m_opencvManager(opencvManager)
	, m_optionBitmask(optionBitmask)
	, m_depthDnn()
	, m_rgbFloatDepthDnnInput(nullptr)
	, m_floatDepthDnnOutput(nullptr)
	, m_floatNormalizedDepth(nullptr)
	, m_gsDepth(nullptr)
	, m_bgrDepth(nullptr)
	, m_floatDepthTextureMap()
	, m_colorMappedDepthTextureMap()
{
}

SyntheticDepthEstimator::~SyntheticDepthEstimator()
{
	dispose();
}

bool SyntheticDepthEstimator::initialize()
{
	dispose();

	m_depthDnn = m_opencvManager->fetchDeepNeuralNetwork(MIDAS_DNN_MODEL_NAME);
	if (!m_depthDnn)
	{
		MIKAN_LOG_ERROR("SyntheticDepthEstimator::initialize") << 
			"Unable to load DNN model: " << MIDAS_DNN_MODEL_NAME;
		return false;
	}

	if (m_depthDnn->getInputChannels() != 3 || m_depthDnn->getOutputChannels() != 1)
	{
		MIKAN_LOG_ERROR("SyntheticDepthEstimator::initialize") << 
			"Invalid DNN model: " << MIDAS_DNN_MODEL_NAME;
		return false;
	}
	
	int dnnInputWidth = m_depthDnn->getInputWidth();
	int dnnInputHeight = m_depthDnn->getInputHeight();
	int dnnOutputWidth = m_depthDnn->getOutputWidth();
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

	if (m_optionBitmask & DEPTH_OPTION_HAS_GL_TEXTURE_FLAG)
	{
		// Used by shaders for frame masking/compositing
		m_floatDepthTextureMap = CreateMkTexture(
			m_depthDnn->getOutputWidth(),
			m_depthDnn->getOutputHeight(),
			nullptr,
			GL_R32F, // texture format
			GL_RED); // buffer format
		m_floatDepthTextureMap->setGenerateMipMap(false);
		m_floatDepthTextureMap->setPixelBufferObjectMode(IMkTexture::PixelBufferObjectMode::DoublePBOWrite);
		m_floatDepthTextureMap->createTexture();

		// Used by node editor preview of depth
		m_colorMappedDepthTextureMap = CreateMkTexture(
			m_depthDnn->getOutputWidth(),
			m_depthDnn->getOutputHeight(),
			nullptr,
			GL_RGB, // texture format
			GL_BGR); // buffer format
		m_colorMappedDepthTextureMap->setGenerateMipMap(false);
		m_colorMappedDepthTextureMap->setPixelBufferObjectMode(IMkTexture::PixelBufferObjectMode::DoublePBOWrite);
		m_colorMappedDepthTextureMap->createTexture();
	}

	return true;
}

void SyntheticDepthEstimator::dispose()
{
	// Free the GL textures
	m_floatDepthTextureMap = nullptr;
	m_colorMappedDepthTextureMap = nullptr;

	// Free the depth DNN
	m_depthDnn = nullptr;

	// Free depth cv::mats
	if (m_rgbFloatDepthDnnInput != nullptr)
		delete m_rgbFloatDepthDnnInput;
	if (m_floatNormalizedDepth != nullptr)
		delete m_floatNormalizedDepth;
	if (m_gsDepth != nullptr)
		delete m_gsDepth;
	if (m_bgrDepth != nullptr)
		delete m_bgrDepth;
	if (m_floatDepthDnnOutput != nullptr)
		delete m_floatDepthDnnOutput;
}

bool SyntheticDepthEstimator::computeSyntheticDepth(cv::Mat* bgrSourceBuffer)
{
	if (bgrSourceBuffer == nullptr ||
		m_rgbFloatDepthDnnInput == nullptr ||
		m_floatDepthDnnOutput == nullptr ||
		!m_depthDnn)
	{
		return false;
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
	if (!m_depthDnn->evaluateForwardPass(m_rgbFloatDepthDnnInput, m_floatDepthDnnOutput))
	{
		return false;
	}

	// Create an accessor Mat(WxH) to read from the DNN output(1xWxH)
	// No actual copy occurs here, just a view into the DNN output buffer
	cv::Mat outputAccessor = getFloatDepthDnnBufferAccessor();

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

	// Copy the depth map into a texture with normalized float values
	if (m_floatNormalizedDepth)
	{
		EASY_BLOCK("NormalizedFloat to texture");

		copyOpenCVMatIntoGLTexture(*m_floatNormalizedDepth, m_floatDepthTextureMap);
	}

	return true;
}

cv::Mat SyntheticDepthEstimator::getFloatDepthDnnBufferAccessor() const
{
	const std::vector<int32_t> size = {m_floatDepthDnnOutput->size[1], m_floatDepthDnnOutput->size[2]};

	return cv::Mat(2, &size[0], CV_32F, m_floatDepthDnnOutput->ptr<float>());
}

void SyntheticDepthEstimator::copyOpenCVMatIntoGLTexture(const cv::Mat& mat, GlTexturePtr texture)
{
	size_t bufferSize = mat.step[0] * mat.rows;

	texture->copyBufferIntoTexture(mat.data, bufferSize);
}


bool SyntheticDepthEstimator::saveVideoFrameToPngFile(const std::filesystem::path& pngPath)
{
	return true;
}