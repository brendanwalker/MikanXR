// -- includes -----
#include "AnchorObjectSystem.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MathMikan.h"
#include "MathGLM.h"
#include "ProfileConfig.h"
#include "PathUtils.h"
#include "StencilObjectSystem.h"
#include "StringUtils.h"
#include "SinglecastDelegate.h"

#include <glm/gtx/matrix_decompose.hpp>

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

// -- WMF Stereo Tracker Config
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
	, cameraParentAnchorId(INVALID_MIKAN_ID)
	, cameraScale(1.f)
	, matVRDevicePath("")
	, originVRDevicePath("")
	, originVerticalAlignFlag(false)
	, calibrationComponentName("front_rolled")
	, vrFrameDelay(0)
	, videoFrameQueueSize(3)
	// Fastener
	, nextFastenerId(0)
	, debugRenderFasteners(true)
	// Compositor
	, compositorScriptFilePath("")
	// Output Settings
	, outputFilePath("")
{
	anchorConfig= addChildConfig<AnchorObjectSystemConfig>("anchors");
	stencilConfig= addChildConfig<StencilObjectSystemConfig>("stencils");
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
	pt["cameraParentAnchorId"]= cameraParentAnchorId;
	pt["cameraScale"]= cameraScale;
	pt["matVRDevicePath"]= matVRDevicePath;
	pt["originVRDevicePath"]= originVRDevicePath;
	pt["originVerticalAlignFlag"]= originVerticalAlignFlag;
	pt["calibrationComponentName"]= calibrationComponentName;
	pt["vrFrameDelay"]= vrFrameDelay;
	pt["videoFrameQueueSize"]= videoFrameQueueSize;
	// Fasteners
	pt["nextFastenerId"]= nextFastenerId;
	pt["debugRenderFasteners"]= debugRenderFasteners;
	// Compositor
	pt["compositorScript"]= compositorScriptFilePath.string();
	// Output Settings
	pt["outputPath"]= outputFilePath.string();

	// Write the anchor system config
	pt[anchorConfig->getConfigName()]= anchorConfig->writeToJSON();

	// Write the stencil system config
	pt[stencilConfig->getConfigName()]= stencilConfig->writeToJSON();

	// Write out the spatial fasteners
	std::vector<configuru::Config> fastenerConfigs;
	for (const MikanSpatialFastenerInfo& fastener : spatialFastenerList)
	{
		configuru::Config fastenerConfig{
			{"id", fastener.fastener_id},
			{"parent_object_id", fastener.parent_object_id},
			{"parent_object_type", k_fastenerParentTypeStrings[fastener.parent_object_type]},
			{"name", fastener.fastener_name},
		};

		writeVector3f(fastenerConfig, "point0", fastener.fastener_points[0]);
		writeVector3f(fastenerConfig, "point1", fastener.fastener_points[1]);
		writeVector3f(fastenerConfig, "point2", fastener.fastener_points[2]);

		fastenerConfigs.push_back(fastenerConfig);
	}
	pt.insert_or_assign(std::string("spatialFasteners"), fastenerConfigs);

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
	cameraParentAnchorId = pt.get_or<int>("cameraParentAnchorId", cameraParentAnchorId);
	cameraScale = pt.get_or<float>("cameraScale", cameraScale);

	matVRDevicePath = pt.get_or<std::string>("matVRDevicePath", matVRDevicePath);
	originVRDevicePath = pt.get_or<std::string>("originVRDevicePath", originVRDevicePath);
	originVerticalAlignFlag = pt.get_or<bool>("originVerticalAlignFlag", originVerticalAlignFlag);
	calibrationComponentName = pt.get_or<std::string>("calibrationComponentName", calibrationComponentName);
	vrFrameDelay = pt.get_or<int>("vrFrameDelay", vrFrameDelay);
	videoFrameQueueSize = int_min(int_max(pt.get_or<int>("videoFrameQueueSize", videoFrameQueueSize), 1), 8);

	// Read the anchor system config
	if (pt.has_key(anchorConfig->getConfigName()))
	{
		anchorConfig->readFromJSON(pt[anchorConfig->getConfigName()]);
	}

	// Read the stencil system config
	if (pt.has_key(stencilConfig->getConfigName()))
	{
		stencilConfig->readFromJSON(pt[stencilConfig->getConfigName()]);
	}

	// Fasteners
	nextFastenerId= pt.get_or<int>("nextFastenerId", nextFastenerId);
	debugRenderFasteners= pt.get_or<bool>("debugRenderFasteners", debugRenderFasteners);

	// Read in the spatial fasteners
	spatialFastenerList.clear();
	if (pt.has_key("spatialFasteners"))
	{
		for (const configuru::Config& fastenerConfig : pt["spatialFasteners"].as_array())
		{
			if (fastenerConfig.has_key("id") && 
				fastenerConfig.has_key("parent_object_id") &&
				fastenerConfig.has_key("parent_object_type") &&
				fastenerConfig.has_key("name") && 
				fastenerConfig.has_key("point0") &&
				fastenerConfig.has_key("point1") &&
				fastenerConfig.has_key("point2"))
			{
				MikanSpatialFastenerInfo fastenerInfo;
				memset(&fastenerInfo, 0, sizeof(fastenerInfo));

				std::string fastenerName = fastenerConfig.get<std::string>("name");
				strncpy(fastenerInfo.fastener_name, fastenerName.c_str(), sizeof(fastenerInfo.fastener_name) - 1);

				fastenerInfo.fastener_id = fastenerConfig.get<int>("id");
				fastenerInfo.parent_object_id = fastenerConfig.get<int>("parent_object_id");

				// Parse the parent object type enum
				{
					const std::string parentTypeString =
						fastenerConfig.get_or<std::string>(
							"parent_object_type",
							k_fastenerParentTypeStrings[MikanFastenerParentType_SpatialAnchor]);

					fastenerInfo.parent_object_type = MikanFastenerParentType_UNKNOWN;
					for (size_t enum_index = 0; enum_index < MikanFastenerParentType_COUNT; ++enum_index)
					{
						if (parentTypeString == k_fastenerParentTypeStrings[enum_index])
						{
							fastenerInfo.parent_object_type = (MikanFastenerParentType)enum_index;
							break;
						}
					}
				}

				readVector3f(fastenerConfig, "point0", fastenerInfo.fastener_points[0]);
				readVector3f(fastenerConfig, "point1", fastenerInfo.fastener_points[1]);
				readVector3f(fastenerConfig, "point2", fastenerInfo.fastener_points[2]);

				spatialFastenerList.push_back(fastenerInfo);
			}
		}
	}

	// Compositor
	compositorScriptFilePath = pt.get_or<std::string>("compositorScript", compositorScriptFilePath.string());

	// Output Path
	outputFilePath = pt.get_or<std::string>("outputPath", outputFilePath.string());
}

#if 0
bool ProfileConfig::getSpatialFastenerInfo(
	MikanSpatialFastenerID fastenerId,
	MikanSpatialFastenerInfo& outInfo) const
{
	auto it = std::find_if(
		spatialFastenerList.begin(), spatialFastenerList.end(),
		[fastenerId](const MikanSpatialFastenerInfo& info) {
		return info.fastener_id == fastenerId;
	});

	if (it != spatialFastenerList.end())
	{
		outInfo = *it;
		return true;
	}

	return false;
}

bool ProfileConfig::getFastenerWorldTransform(MikanSpatialFastenerID fastenerId, glm::mat4& outXform) const
{
	MikanSpatialFastenerInfo fastener;
	if (getSpatialFastenerInfo(fastenerId, fastener))
	{
		outXform = getFastenerWorldTransform(&fastener);
		return true;
	}

	return false;
}

glm::mat4 ProfileConfig::getFastenerWorldTransform(const MikanSpatialFastenerInfo* fastener) const
{
	if (fastener->parent_object_id != INVALID_MIKAN_ID)
	{
		if (fastener->parent_object_type == MikanFastenerParentType_SpatialAnchor)
		{
			glm::mat4 xform;

			if (getSpatialAnchorWorldTransform(fastener->parent_object_id, xform))
				return xform;
		}
		else if (fastener->parent_object_type == MikanFastenerParentType_Stencil)
		{
			glm::mat4 xform;

			if (getStencilWorldTransform(fastener->parent_object_id, xform))
				return xform;
		}
	}

	return glm::mat4(1.f);
}

void ProfileConfig::getFastenerLocalPoints(
	const MikanSpatialFastenerInfo* fastener, 
	glm::vec3 outLocalPoints[3]) const
{
	for (int i = 0; i < 3; ++i)
	{
		outLocalPoints[i] = MikanVector3f_to_glm_vec3(fastener->fastener_points[i]);
	}
}

void ProfileConfig::getFastenerWorldPoints(
	const MikanSpatialFastenerInfo* fastener, 
	glm::vec3 outWorldPoints[3]) const
{
	const glm::mat4 localToWorld= getFastenerWorldTransform(fastener);

	glm::vec3 localPoints[3];
	getFastenerLocalPoints(fastener, localPoints);
	for (int i = 0; i < 3; ++i)
	{
		outWorldPoints[i] = localToWorld * glm::vec4(localPoints[i], 1.f);
	}
}

MikanSpatialFastenerID ProfileConfig::getNextSpatialFastenerId(MikanSpatialFastenerID fastenerId) const
{
	// If the given fastener id is invalid, return the first valid fastener id
	if (fastenerId == INVALID_MIKAN_ID)
	{
		return spatialFastenerList.size() > 0 ? spatialFastenerList[0].fastener_id : INVALID_MIKAN_ID;
	}

	// Find the list index for the fastener with the matching ID
	int listIndex = 0;
	while (listIndex < spatialFastenerList.size())
	{
		if (spatialFastenerList[listIndex].fastener_id == fastenerId)
			break;

		++listIndex;
	}

	// Move to the next entry in the list
	++listIndex;

	// If this list entry is still in the list, return it
	return (listIndex < spatialFastenerList.size()) ? spatialFastenerList[listIndex].fastener_id : INVALID_MIKAN_ID;
}

bool ProfileConfig::findSpatialFastenerInfoByName(
	const char* fastenerName,
	MikanSpatialFastenerInfo& outInfo) const
{
	auto it = std::find_if(
		spatialFastenerList.begin(), spatialFastenerList.end(),
		[fastenerName](const MikanSpatialFastenerInfo& info) {
			return strncmp(info.fastener_name, fastenerName, MAX_MIKAN_ANCHOR_NAME_LEN) == 0;
		});

	if (it != spatialFastenerList.end())
	{
		outInfo = *it;
		return true;
	}

	return false;
}

std::vector<MikanSpatialFastenerID> ProfileConfig::getSpatialFastenersWithParent(
	const MikanFastenerParentType parentType, 
	const MikanSpatialAnchorID parentObjectId) const
{
	std::vector<MikanSpatialFastenerID> result;

	for (const MikanSpatialFastenerInfo& info : spatialFastenerList)
	{
		if (info.parent_object_type == parentType && info.parent_object_id == parentObjectId)
		{
			result.push_back(info.fastener_id);
		}
	}

	return result;
}

std::vector<MikanSpatialFastenerID> ProfileConfig::getValidSpatialFastenerSnapTargets(
	const MikanSpatialFastenerID sourceFastenerId) const
{
	std::vector<MikanSpatialFastenerID> result;

	MikanSpatialFastenerInfo sourceFastenerInfo;
	if (getSpatialFastenerInfo(sourceFastenerId, sourceFastenerInfo) && 
		sourceFastenerInfo.parent_object_type == MikanFastenerParentType_Stencil)
	{
		for (const MikanSpatialFastenerInfo& otherFastenerInfo : spatialFastenerList)
		{
			if (otherFastenerInfo.fastener_id != sourceFastenerId && 
				otherFastenerInfo.parent_object_type == MikanFastenerParentType_SpatialAnchor)
			{
				result.push_back(otherFastenerInfo.fastener_id);
			}
		}
	}

	return result;
}

bool ProfileConfig::canAddFastener() const
{
	return (spatialFastenerList.size() < MAX_MIKAN_SPATIAL_ANCHORS);
}

MikanSpatialFastenerID ProfileConfig::addNewFastener(
	const MikanSpatialFastenerInfo& fastener)
{
	if (!canAddFastener())
		return INVALID_MIKAN_ID;

	MikanSpatialFastenerInfo newFastener = fastener;
	newFastener.fastener_id = nextFastenerId;
	nextFastenerId++;

	spatialFastenerList.push_back(newFastener);
	markDirty();

	return newFastener.fastener_id;
}

bool ProfileConfig::updateFastener(const MikanSpatialFastenerInfo& info)
{
	MikanSpatialFastenerID fastenerId = info.fastener_id;

	auto it = std::find_if(
		spatialFastenerList.begin(), spatialFastenerList.end(),
		[fastenerId](const MikanSpatialFastenerInfo& info) {
			return info.fastener_id == fastenerId;
		});

	if (it != spatialFastenerList.end())
	{
		*it = info;
		markDirty();
		return true;
	}

	return false;
}

bool ProfileConfig::removeFastener(MikanSpatialFastenerID fastenerId)
{
	auto it = std::find_if(
		spatialFastenerList.begin(), spatialFastenerList.end(),
		[fastenerId](const MikanSpatialFastenerInfo& info) {
		return info.fastener_id == fastenerId;
	});

	if (it != spatialFastenerList.end())
	{
		spatialFastenerList.erase(it);
		markDirty();

		return true;
	}

	return false;
}
#endif

std::filesystem::path ProfileConfig::generateTimestampedFilePath(
	const std::string& prefix, 
	const std::string& suffix) const
{
	const std::filesystem::path parentDir= !outputFilePath.empty() ? outputFilePath : std::filesystem::current_path();

	return PathUtils::makeTimestampedFilePath(parentDir, prefix, suffix);
}