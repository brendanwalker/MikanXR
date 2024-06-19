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

#define CHARUCO_PATTERN_W 11
#define CHARUCO_PATTERN_H 8
#define DEFAULT_CHARUCO_SQUARE_LEN_MM 16
#define DEFAULT_CHARUCO_MARKER_LEN_MM 12
#define DEFAULT_CHARUCO_DICTIONARY_TYPE eCharucoDictionaryType::DICT_6X6

#define DEFAULT_PUCK_HORIZONTAL_OFFSET_MM  -72
#define DEFAULT_PUCK_VERTICAL_OFFSET_MM  -67
#define DEFAULT_PUCK_DEPTH_OFFSET_MM  0

#define DEFAULT_VR_CENTER_ARUCO_ID  0
#define DEFAULT_VR_CENTER_MARKER_LEN_MM  100

// -- Profile Config
const std::string ProfileConfig::k_videoSourcePathPropertyId= "videoSourcePath";
const std::string ProfileConfig::k_cameraVRDevicePathPropertyId= "cameraVRDevicePath";
const std::string ProfileConfig::k_matVRDevicePathPropertyId= "matVRDevicePath";
const std::string ProfileConfig::k_vrDevicePoseOffsetPropertyId= "vrDevicePoseOffset";
const std::string ProfileConfig::k_renderOriginFlagPropertyId= "renderOrigin";
const std::string ProfileConfig::k_vrFrameDelayPropertyId= "vrFrameDelay";


ProfileConfig::ProfileConfig(const std::string& fnamebase)
	: CommonConfig(fnamebase)
	// Pattern Defaults
	, calibrationPatternType(eCalibrationPatternType::mode_chessboard)
	, chessbordRows(CHESSBOARD_PATTERN_H)
	, chessbordCols(CHESSBOARD_PATTERN_W)
	, squareLengthMM(DEFAULT_SQUARE_LEN_MM)
	, charucoRows(CHARUCO_PATTERN_H)
	, charucoCols(CHARUCO_PATTERN_W)
	, charucoSquareLengthMM(DEFAULT_CHARUCO_SQUARE_LEN_MM)
	, charucoMarkerLengthMM(DEFAULT_CHARUCO_MARKER_LEN_MM)
	, charucoDictionaryType(DEFAULT_CHARUCO_DICTIONARY_TYPE)
	, puckHorizontalOffsetMM(DEFAULT_PUCK_HORIZONTAL_OFFSET_MM)
	, puckVerticalOffsetMM(DEFAULT_PUCK_VERTICAL_OFFSET_MM)
	, puckDepthOffsetMM(DEFAULT_PUCK_DEPTH_OFFSET_MM)
	, vrCenterArucoId(DEFAULT_VR_CENTER_ARUCO_ID)
	, vrCenterMarkerLengthMM(DEFAULT_VR_CENTER_MARKER_LEN_MM)
	// VideoSource Defaults
	, videoSourcePath("")
	// Tracker
	, cameraVRDevicePath("")
	, matVRDevicePath("")
	, vivePuckDefaultComponentName("front_rolled")
	, videoFrameQueueSize(3)
	// Compositor
	, compositorScriptFilePath("")
	// Output Settings
	, outputFilePath("")
{
	vrDevicePoseOffset= {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f,
	};

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
	pt["calibrationPatternType"]= k_patternTypeStrings[(int)calibrationPatternType];
	pt["chessbordRows"]= chessbordRows;
	pt["chessbordCols"]= chessbordCols;
	pt["squareLengthMM"]= squareLengthMM;
	pt["charucoRows"] = charucoRows;
	pt["charucoCols"] = charucoCols;
	pt["charucoSquareLengthMM"] = charucoSquareLengthMM;
	pt["charucoMarkerLengthMM"] = charucoMarkerLengthMM;
	pt["charucoDictionaryType"] = k_charucoDictionaryStrings[(int)charucoDictionaryType];
	pt["puckHorizontalOffsetMM"]= puckHorizontalOffsetMM;
	pt["puckVerticalOffsetMM"]= puckVerticalOffsetMM;
	pt["puckDepthOffsetMM"]= puckDepthOffsetMM;
	pt["vrCenterArucoId"]= vrCenterArucoId;
	pt["vrCenterMarkerLengthMM"]= vrCenterMarkerLengthMM;
	// VideoSource Defaults
	pt["videoSourcePath"]= videoSourcePath;
	// Tracker
	pt["cameraVRDevicePath"]= cameraVRDevicePath;
	pt["matVRDevicePath"]= matVRDevicePath;
	pt["vivePuckDefaultComponentName"]= vivePuckDefaultComponentName;
	pt[k_vrFrameDelayPropertyId]= m_vrFrameDelay;
	pt["videoFrameQueueSize"]= videoFrameQueueSize;
	// Compositor
	pt["compositorScript"]= compositorScriptFilePath.string();
	// Output Settings
	pt["outputPath"]= outputFilePath.string();
	// Renderer Flags
	pt[k_renderOriginFlagPropertyId]= m_bRenderOrigin;

	writeMatrix4f(pt, "vrDevicePoseOffset", vrDevicePoseOffset);

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

	const std::string charcuoDictionaryString =
		pt.get_or<std::string>(
			"charucoDictionaryType",
			k_charucoDictionaryStrings[(int)eCharucoDictionaryType::DICT_6X6]);
	charucoDictionaryType =
		StringUtils::FindEnumValue<eCharucoDictionaryType>(
			charcuoDictionaryString,
			k_charucoDictionaryStrings);
	charucoRows = pt.get_or<int>("charucoRows", charucoRows);
	charucoCols = pt.get_or<int>("charucoCols", charucoCols);
	charucoSquareLengthMM = pt.get_or<float>("charucoSquareLengthMM", charucoSquareLengthMM);
	charucoMarkerLengthMM = pt.get_or<float>("charucoMarkerLengthMM", charucoMarkerLengthMM);

	puckHorizontalOffsetMM = pt.get_or<float>("puckHorizontalOffsetMM", puckHorizontalOffsetMM);
	puckVerticalOffsetMM = pt.get_or<float>("puckVerticalOffsetMM", puckVerticalOffsetMM);
	puckDepthOffsetMM = pt.get_or<float>("puckDepthOffsetMM", puckDepthOffsetMM);

	vrCenterArucoId = pt.get_or<int>("vrCenterArucoId", vrCenterArucoId);
	vrCenterMarkerLengthMM = pt.get_or<float>("vrCenterMarkerLengthMM", vrCenterMarkerLengthMM);

	// VideoSource Defaults
	videoSourcePath = pt.get_or<std::string>("videoSourcePath", videoSourcePath);

	// VR Devices
	cameraVRDevicePath = pt.get_or<std::string>("cameraVRDevicePath", cameraVRDevicePath);

	matVRDevicePath = pt.get_or<std::string>("matVRDevicePath", matVRDevicePath);
	vivePuckDefaultComponentName = pt.get_or<std::string>("vivePuckDefaultComponentName", vivePuckDefaultComponentName);
	m_vrFrameDelay = pt.get_or<int>(k_vrFrameDelayPropertyId, m_vrFrameDelay);
	videoFrameQueueSize = int_min(int_max(pt.get_or<int>("videoFrameQueueSize", videoFrameQueueSize), 1), 8);
	m_bRenderOrigin = pt.get_or<bool>(k_renderOriginFlagPropertyId, m_bRenderOrigin);

	readMatrix4f(pt, "vrDevicePoseOffset", vrDevicePoseOffset);

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