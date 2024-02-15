#include "FrameCompositorConstants.h"
#include "opencv2/opencv.hpp"

const std::string g_compositorLayerAlphaStrings[(int)eCompositorLayerAlphaMode::COUNT] = {
	"NoAlpha",
	"ColorKey",
	"AlphaChannel",
	"MagicPortal"
};
const std::string* k_compositorLayerAlphaStrings = g_compositorLayerAlphaStrings;

const std::string g_supportedCodecName[(int)eSupportedCodec::COUNT] = {
	"MP4V",
	"MJPG",
	"RGBA",
};
const std::string* k_supportedCodecName= g_supportedCodecName;

const std::string g_supportedCodecFileSuffix[(int)eSupportedCodec::COUNT] = {
	".m4v",
	".avi",
	".avi",
};
const std::string* k_supportedCodecFileSuffix= g_supportedCodecFileSuffix;

const int g_supportedCodecFourCC[(int)eSupportedCodec::COUNT] = {
	cv::VideoWriter::fourcc('m','p','4','v'),
	cv::VideoWriter::fourcc('M','J','P','G'),
	cv::VideoWriter::fourcc('R','G','B','A'),
};
const int* k_supportedCodecFourCC= g_supportedCodecFourCC;

const std::string g_compositorStencilModeStrings[(int)eCompositorStencilMode::COUNT] = {
	"noStencil",
	"insideStencil",
	"outsideStencil"
};
const std::string* k_compositorStencilModeStrings= g_compositorStencilModeStrings;

const std::string g_compositorBlendModeStrings[(int)eCompositorBlendMode::COUNT] = {
	"blendOff",
	"blendOn"
};
const std::string* k_compositorBlendModeStrings = g_compositorBlendModeStrings;

const std::string g_stencilCullModeStrings[(int)eStencilCullMode::COUNT] = {
	"none",
	"zAxis",
	"yAxis",
	"xAxis",
};
const std::string* k_stencilCullModeStrings = g_stencilCullModeStrings;

const std::string g_clientTextureTypeStrings[(int)eClientTextureType::COUNT] = {
	"color",
	"depth"
};
const std::string* k_clientTextureTypeStrings = g_clientTextureTypeStrings;

const std::string g_compositorEvaluatorWindow[(int)eCompositorEvaluatorWindow::COUNT] = {
	"mainWindow",
	"editorWindow"
};
const std::string* k_compositorEvaluatorWindow = g_compositorEvaluatorWindow;