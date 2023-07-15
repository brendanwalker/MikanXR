// -- includes -----
#include "AnchorObjectSystem.h"
#include "EditorObjectSystem.h"
#include "MathUtility.h"
#include "ProfileConfig.h"
#include "PathUtils.h"
#include "StencilObjectSystem.h"
#include "StringUtils.h"
#include "SinglecastDelegate.h"

#define CHESSBOARD_PATTERN_W 7 // Internal corners
#define CHESSBOARD_PATTERN_H 5
#define DEFAULT_SQUARE_LEN_MM 30

#define CIRCLEGRID_PATTERN_W 4
#define CIRCLEGRID_PATTERN_H 11
#define DEFAULT_CIRCLE_DIAMETER_MM 15
#define DEFAULT_CIRCLE_SPACING_MM  20

#define DEFAULT_PUCK_HORIZONTAL_OFFSET_MM  75
#define DEFAULT_PUCK_VERTICAL_OFFSET_MM  89
#define DEFAULT_PUCK_DEPTH_OFFSET_MM  0

// -- Profile Config
const std::string ProfileConfig::k_cameraVRDevicePathPropertyId= "cameraVRDevicePath";
const std::string ProfileConfig::k_matVRDevicePathPropertyId= "matVRDevicePath";
const std::string ProfileConfig::k_originVRDevicePathPropertyId= "originVRDevicePath";
const std::string ProfileConfig::k_originVerticalAlignFlagPropertyId= "originVerticalAlignFlag";
const std::string ProfileConfig::k_renderOriginFlagPropertyId= "renderOrigin";
const std::string ProfileConfig::k_vrFrameDelayPropertyId= "vrFrameDelay";


ProfileConfig::ProfileConfig(const std::string& fnamebase)
	: CommonConfig(fnamebase)
	// Pattern Defaults
	, calibrationPatternType(eCalibrationPatternType::mode_chessboard)
	, chessbordRows(CHESSBOARD_PATTERN_H)
	, chessbordCols(CHESSBOARD_PATTERN_W)
	, squareLengthMM(DEFAULT_SQUARE_LEN_MM)
	, circleGridRows(CIRCLEGRID_PATTERN_H)
	, circleGridCols(CIRCLEGRID_PATTERN_W)
	, circleSpacingMM(DEFAULT_CIRCLE_SPACING_MM)
	, circleDiameterMM(DEFAULT_CIRCLE_DIAMETER_MM)
	, puckHorizontalOffsetMM(DEFAULT_PUCK_HORIZONTAL_OFFSET_MM)
	, puckVerticalOffsetMM(DEFAULT_PUCK_VERTICAL_OFFSET_MM)
	, puckDepthOffsetMM(DEFAULT_PUCK_DEPTH_OFFSET_MM)
	// VideoSource Defaults
	, videoSourcePath("")
	// Tracker
	, cameraVRDevicePath("")
	, matVRDevicePath("")
	, originVRDevicePath("")
	, originVerticalAlignFlag(false)
	, calibrationComponentName("front_rolled")
	, videoFrameQueueSize(3)
	// Compositor
	, compositorScriptFilePath("")
	// Output Settings
	, outputFilePath("")
{
	anchorConfig= std::make_shared<AnchorObjectSystemConfig>("anchors");
	addChildConfig(anchorConfig);

	editorConfig= std::make_shared<EditorObjectSystemConfig>("editor");
	addChildConfig(editorConfig);

	stencilConfig= std::make_shared<StencilObjectSystemConfig>("stencils");
	addChildConfig(stencilConfig);
};

configuru::Config ProfileConfig::writeToJSON()
{
	configuru::Config pt= CommonConfig::writeToJSON();

	// Pattern Defaults
	pt["calibrationPatternType"]= k_patternTypeStrings[(int)eCalibrationPatternType::mode_chessboard];
	pt["chessbordRows"]= chessbordRows;
	pt["chessbordCols"]= chessbordCols;
	pt["squareLengthMM"]= squareLengthMM;
	pt["circleGridRows"]= circleGridRows;
	pt["circleGridCols"]= circleGridCols;
	pt["circleSpacingMM"]= circleSpacingMM;
	pt["circleDiameterMM"]= circleDiameterMM;
	pt["puckHorizontalOffsetMM"]= puckHorizontalOffsetMM;
	pt["puckVerticalOffsetMM"]= puckVerticalOffsetMM;
	pt["puckDepthOffsetMM"]= puckDepthOffsetMM;
	// VideoSource Defaults
	pt["videoSourcePath"]= videoSourcePath;
	// Tracker
	pt["cameraVRDevicePath"]= cameraVRDevicePath;
	pt["matVRDevicePath"]= matVRDevicePath;
	pt["originVRDevicePath"]= originVRDevicePath;
	pt["originVerticalAlignFlag"]= originVerticalAlignFlag;
	pt["calibrationComponentName"]= calibrationComponentName;
	pt[k_vrFrameDelayPropertyId]= m_vrFrameDelay;
	pt["videoFrameQueueSize"]= videoFrameQueueSize;
	// Compositor
	pt["compositorScript"]= compositorScriptFilePath.string();
	// Output Settings
	pt["outputPath"]= outputFilePath.string();
	// Renderer Flags
	pt[k_renderOriginFlagPropertyId]= m_bRenderOrigin;

	// Write the anchor system config
	pt[anchorConfig->getConfigName()]= anchorConfig->writeToJSON();

	// Write the editor system config
	pt[editorConfig->getConfigName()] = editorConfig->writeToJSON();

	// Write the stencil system config
	pt[stencilConfig->getConfigName()]= stencilConfig->writeToJSON();

	return pt;
}

void ProfileConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	// Pattern Defaults
	const std::string patternString =
		pt.get_or<std::string>(
			"calibrationPatternType",
			k_patternTypeStrings[(int)eCalibrationPatternType::mode_chessboard]);
	calibrationPatternType = 
		StringUtils::FindEnumValue<eCalibrationPatternType>(
			patternString, 
			k_patternTypeStrings);
	chessbordRows = pt.get_or<int>("chessbordRows", chessbordRows);
	chessbordCols = pt.get_or<int>("chessbordCols", chessbordCols);
	squareLengthMM = pt.get_or<float>("squareLengthMM", squareLengthMM);
	circleGridRows = pt.get_or<int>("circleGridRows", circleGridRows);
	circleGridCols = pt.get_or<int>("circleGridCols", circleGridCols);
	circleSpacingMM = pt.get_or<float>("circleSpacingMM", circleSpacingMM);
	circleDiameterMM = pt.get_or<float>("circleDiameterMM", circleDiameterMM);
	puckHorizontalOffsetMM = pt.get_or<float>("puckHorizontalOffsetMM", puckHorizontalOffsetMM);
	puckVerticalOffsetMM = pt.get_or<float>("puckVerticalOffsetMM", puckVerticalOffsetMM);
	puckDepthOffsetMM = pt.get_or<float>("puckDepthOffsetMM", puckDepthOffsetMM);

	// VideoSource Defaults
	videoSourcePath = pt.get_or<std::string>("videoSourcePath", videoSourcePath);

	// VR Devices
	cameraVRDevicePath = pt.get_or<std::string>("cameraVRDevicePath", cameraVRDevicePath);

	matVRDevicePath = pt.get_or<std::string>("matVRDevicePath", matVRDevicePath);
	originVRDevicePath = pt.get_or<std::string>("originVRDevicePath", originVRDevicePath);
	originVerticalAlignFlag = pt.get_or<bool>("originVerticalAlignFlag", originVerticalAlignFlag);
	calibrationComponentName = pt.get_or<std::string>("calibrationComponentName", calibrationComponentName);
	m_vrFrameDelay = pt.get_or<int>(k_vrFrameDelayPropertyId, m_vrFrameDelay);
	videoFrameQueueSize = int_min(int_max(pt.get_or<int>("videoFrameQueueSize", videoFrameQueueSize), 1), 8);
	m_bRenderOrigin = pt.get_or<bool>(k_renderOriginFlagPropertyId, m_bRenderOrigin);

	// Read the anchor system config
	if (pt.has_key(anchorConfig->getConfigName()))
	{
		anchorConfig->readFromJSON(pt[anchorConfig->getConfigName()]);
	}

	// Read the editor system config
	if (pt.has_key(editorConfig->getConfigName()))
	{
		editorConfig->readFromJSON(pt[editorConfig->getConfigName()]);
	}

	// Read the stencil system config
	if (pt.has_key(stencilConfig->getConfigName()))
	{
		stencilConfig->readFromJSON(pt[stencilConfig->getConfigName()]);
	}

	// Compositor
	compositorScriptFilePath = pt.get_or<std::string>("compositorScript", compositorScriptFilePath.string());

	// Output Path
	outputFilePath = pt.get_or<std::string>("outputPath", outputFilePath.string());
}

std::filesystem::path ProfileConfig::generateTimestampedFilePath(
	const std::string& prefix, 
	const std::string& suffix) const
{
	const std::filesystem::path parentDir= !outputFilePath.empty() ? outputFilePath : std::filesystem::current_path();

	return PathUtils::makeTimestampedFilePath(parentDir, prefix, suffix);
}

void ProfileConfig::setRenderOriginFlag(bool flag)
{
	if (m_bRenderOrigin != flag)
	{
		m_bRenderOrigin = flag;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_renderOriginFlagPropertyId));
	}
}

void ProfileConfig::setVRFrameDelay(int frameDelay)
{
	if (m_vrFrameDelay != frameDelay)
	{
		m_vrFrameDelay= frameDelay;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_vrFrameDelayPropertyId));
	}
}