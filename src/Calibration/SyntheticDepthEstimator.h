#pragma once

#include "OpenCVFwd.h"
#include "RendererFwd.h"

#include <glm/ext/matrix_float4x4.hpp>

#include <filesystem>
#include <memory>

class VideoFrameDistortionView;
typedef std::shared_ptr<VideoFrameDistortionView> VideoFrameDistortionViewPtr;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
#define DEPTH_OPTION_HAS_GL_TEXTURE_FLAG			0x0001
#define DEPTH_OPTION_HAS_ALL						0xffff

class SyntheticDepthEstimator
{
public:
	SyntheticDepthEstimator(class OpenCVManager* opencvManager, int optionBitmask);
	virtual ~SyntheticDepthEstimator();

	bool initialize();
	void dispose();

	inline cv::Mat* getFloatDepthDnnBuffer() const { return m_floatDepthDnnOutput; }
	inline GlTexturePtr getFloatDepthTexture() const { return m_floatDepthTextureMap; }
	inline GlTexturePtr getColorMappedDepthTexture() const { return m_colorMappedDepthTextureMap; }

	bool computeSyntheticDepth(cv::Mat* bgrSourceBuffer);
	bool saveVideoFrameToPngFile(const std::filesystem::path& pngPath);

protected:
	static void copyOpenCVMatIntoGLTexture(const cv::Mat& mat, GlTexturePtr texture);

protected:
	class OpenCVManager* m_opencvManager= nullptr;
	int m_optionBitmask= 0;

	// Synthetic depth buffer generated using MiDaS DNN
	DeepNeuralNetworkPtr m_depthDnn; // MiDaS DNN for depth estimation
	cv::Mat* m_rgbFloatDepthDnnInput = nullptr; // Small RGB float blob input for DNN
	cv::Mat* m_floatDepthDnnOutput = nullptr; // Small float blob output for DNN
	cv::Mat* m_floatNormalizedDepth = nullptr; // Normalized float depth debug buffer
	cv::Mat* m_gsDepth = nullptr; // 8-BPP Grayscale depth buffer
	cv::Mat* m_bgrDepth = nullptr; // 24-BPP(BGR color format) color-coded depth buffer
	GlTexturePtr m_floatDepthTextureMap = nullptr; // GL Texture filled in m_floatDepthDnnOutput
	GlTexturePtr m_colorMappedDepthTextureMap = nullptr; // GL Texture filled in m_bgrGsDepth
};