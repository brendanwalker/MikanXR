// -- includes -----
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MathMikan.h"
#include "ProfileConfig.h"
#include "PathUtils.h"
#include "StringUtils.h"

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
	, calibrationComponentName("front_rolled")
	, vrFrameDelay(0)
	, videoFrameQueueSize(3)
	// Anchor
	, anchorVRDevicePath("")
	, nextAnchorId(0)
	, debugRenderAnchors(true)
	// Fastener
	, nextFastenerId(0)
	, debugRenderFasteners(true)
	// Stencils
	, nextStencilId(0)
	, debugRenderStencils(true)
	// Compositor
	, compositorScriptFilePath("")
	// Output Settings
	, outputFilePath("")
{
};

const configuru::Config ProfileConfig::writeToJSON()
{
	configuru::Config pt{
		// Pattern Defaults
		{"calibrationPatternType", k_patternTypeStrings[(int)eCalibrationPatternType::mode_chessboard]},
		{"chessbordRows", chessbordRows},
		{"chessbordCols", chessbordCols},
		{"squareLengthMM", squareLengthMM},
		{"circleGridRows", circleGridRows},
		{"circleGridCols", circleGridCols},
		{"circleSpacingMM", circleSpacingMM},
		{"circleDiameterMM", circleDiameterMM},
		{"puckHorizontalOffsetMM", puckHorizontalOffsetMM},
		{"puckVerticalOffsetMM", puckVerticalOffsetMM},
		{"puckDepthOffsetMM", puckDepthOffsetMM},
		// VideoSource Defaults
		{"videoSourcePath", videoSourcePath},
		// Tracker
		{"cameraVRDevicePath", cameraVRDevicePath},
		{"cameraParentAnchorId", cameraParentAnchorId},
		{"cameraScale", cameraScale},
		{"matVRDevicePath", matVRDevicePath},
		{"calibrationComponentName", calibrationComponentName},
		{"vrFrameDelay", vrFrameDelay},
		{"videoFrameQueueSize", videoFrameQueueSize},
		// Anchors
		{"anchorVRDevicePath", anchorVRDevicePath},
		{"nextAnchorId", nextAnchorId},
		{"debugRenderAnchors", debugRenderAnchors},
		// Fasteners
		{"nextFastenerId", nextFastenerId},
		{"debugRenderFasteners", debugRenderFasteners},
		// Stencils
		{"nextStencilId", nextStencilId},
		{"debugRenderStencils", debugRenderStencils},
		// Compositor
		{"compositorScript", compositorScriptFilePath.string()},
		// Output Settings
		{"outputPath", outputFilePath.string()}
	};

	// Write out the spatial anchors
	std::vector<configuru::Config> anchorConfigs;
	for (const MikanSpatialAnchorInfo& anchor : spatialAnchorList)
	{
		configuru::Config anchorConfig{
			{"id", anchor.anchor_id},
			{"name", anchor.anchor_name},
		};

		writeMatrix4f(anchorConfig, "xform", anchor.anchor_xform);

		anchorConfigs.push_back(anchorConfig);
	}
	pt.insert_or_assign(std::string("spatialAnchors"), anchorConfigs);

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

	// Write out the quad stencils
	std::vector<configuru::Config> stencilQuadConfigs;
	for (const MikanStencilQuad& stencil : quadStencilList)
	{
		configuru::Config stencilConfig{
			{"stencil_id", stencil.stencil_id},
			{"parent_anchor_id", stencil.parent_anchor_id},
			{"quad_width", stencil.quad_width},
			{"quad_height", stencil.quad_height},
			{"is_double_sided", stencil.is_double_sided},
			{"is_disabled", stencil.is_disabled},
			{"stencil_name", stencil.stencil_name}
		};

		writeVector3f(stencilConfig, "quad_center", stencil.quad_center);
		writeVector3f(stencilConfig, "quad_x_axis", stencil.quad_x_axis);
		writeVector3f(stencilConfig, "quad_y_axis", stencil.quad_y_axis);
		writeVector3f(stencilConfig, "quad_normal", stencil.quad_normal);

		stencilQuadConfigs.push_back(stencilConfig);
	}
	pt.insert_or_assign(std::string("quadStencils"), stencilQuadConfigs);

	// Write out the box stencils
	std::vector<configuru::Config> stencilBoxConfigs;
	for (const MikanStencilBox& stencil : boxStencilList)
	{
		configuru::Config stencilConfig{
			{"stencil_id", stencil.stencil_id},
			{"parent_anchor_id", stencil.parent_anchor_id},
			{"box_x_size", stencil.box_x_size},
			{"box_y_size", stencil.box_y_size},
			{"box_z_size", stencil.box_z_size},
			{"is_disabled", stencil.is_disabled},
			{"stencil_name", stencil.stencil_name}
		};

		writeVector3f(stencilConfig, "box_center", stencil.box_center);
		writeVector3f(stencilConfig, "box_x_axis", stencil.box_x_axis);
		writeVector3f(stencilConfig, "box_y_axis", stencil.box_y_axis);
		writeVector3f(stencilConfig, "box_z_axis", stencil.box_z_axis);

		stencilBoxConfigs.push_back(stencilConfig);
	}
	pt.insert_or_assign(std::string("boxStencils"), stencilBoxConfigs);

	// Write out the model stencils
	std::vector<configuru::Config> stencilModelConfigs;
	for (const MikanStencilModelConfig& stencil : modelStencilList)
	{
		configuru::Config stencilConfig{
			{"model_path", stencil.modelPath.string()},
			{"stencil_id", stencil.modelInfo.stencil_id},
			{"parent_anchor_id", stencil.modelInfo.parent_anchor_id},
			{"is_disabled", stencil.modelInfo.is_disabled},
			{"stencil_name", stencil.modelInfo.stencil_name}
		};

		writeVector3f(stencilConfig, "model_position", stencil.modelInfo.model_position);
		writeRotator3f(stencilConfig, "model_rotator", stencil.modelInfo.model_rotator);
		writeVector3f(stencilConfig, "model_scale", stencil.modelInfo.model_scale);

		stencilModelConfigs.push_back(stencilConfig);
	}
	pt.insert_or_assign(std::string("modelStencils"), stencilModelConfigs);

	return pt;
}

void ProfileConfig::readFromJSON(const configuru::Config& pt)
{
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
	calibrationComponentName = pt.get_or<std::string>("calibrationComponentName", calibrationComponentName);
	vrFrameDelay = pt.get_or<int>("vrFrameDelay", vrFrameDelay);
	videoFrameQueueSize = int_min(int_max(pt.get_or<int>("videoFrameQueueSize", videoFrameQueueSize), 1), 8);

	// Anchors
	anchorVRDevicePath= pt.get_or<std::string>("anchorVRDevicePath", anchorVRDevicePath);
	nextAnchorId= pt.get_or<int>("nextAnchorId", nextAnchorId);
	debugRenderAnchors= pt.get_or<bool>("debugRenderAnchors", debugRenderAnchors);

	// Fasteners
	nextFastenerId= pt.get_or<int>("nextFastenerId", nextFastenerId);
	debugRenderFasteners= pt.get_or<bool>("debugRenderFasteners", debugRenderFasteners);

	// Read in the spatial anchors
	spatialAnchorList.clear();
	if (pt.has_key("spatialAnchors"))
	{
		for (const configuru::Config& anchorConfig : pt["spatialAnchors"].as_array())
		{
			if (anchorConfig.has_key("id") && anchorConfig.has_key("name") && anchorConfig.has_key("xform"))
			{
				MikanSpatialAnchorInfo anchorInfo;
				memset(&anchorInfo, 0, sizeof(anchorInfo));

				std::string anchorName= anchorConfig.get<std::string>("name");
				strncpy(anchorInfo.anchor_name, anchorName.c_str(), sizeof(anchorInfo.anchor_name) - 1);

				anchorInfo.anchor_id = anchorConfig.get<int>("id");

				readMatrix4f(anchorConfig, "xform", anchorInfo.anchor_xform);

				spatialAnchorList.push_back(anchorInfo);
			}
		}
	}

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

	// Stencils
	nextStencilId = pt.get_or<int>("nextStencilId", nextStencilId);
	debugRenderStencils= pt.get_or<bool>("debugRenderStencils", debugRenderStencils);

	// Compositor
	compositorScriptFilePath = pt.get_or<std::string>("compositorScript", compositorScriptFilePath.string());

	// Read in the quad stencils
	quadStencilList.clear();
	if (pt.has_key("quadStencils"))
	{
		for (const configuru::Config& stencilConfig : pt["quadStencils"].as_array())
		{
			if (stencilConfig.has_key("stencil_id"))
			{
				MikanStencilQuad stencil;
				memset(&stencil, 0, sizeof(stencil));

				stencil.stencil_id = stencilConfig.get<int>("stencil_id");
				stencil.parent_anchor_id = stencilConfig.get_or<int>("parent_anchor_id", -1);
				readVector3f(stencilConfig, "quad_center", stencil.quad_center);
				readVector3f(stencilConfig, "quad_x_axis", stencil.quad_x_axis);
				readVector3f(stencilConfig, "quad_y_axis", stencil.quad_y_axis);
				readVector3f(stencilConfig, "quad_normal", stencil.quad_normal);
				stencil.quad_width = stencilConfig.get_or<float>("quad_width", 0.25f);
				stencil.quad_height = stencilConfig.get_or<float>("quad_height", 0.25f);
				stencil.is_double_sided = stencilConfig.get_or<bool>("is_double_sided", false);
				stencil.is_disabled = stencilConfig.get_or<bool>("is_disabled", false);

				const std::string stencil_name= stencilConfig.get_or<std::string>("stencil_name", "");
				StringUtils::formatString(stencil.stencil_name, sizeof(stencil.stencil_name), "%s", stencil_name.c_str());

				quadStencilList.push_back(stencil);
			}
		}
	}

	// Read in the box stencils
	boxStencilList.clear();
	if (pt.has_key("boxStencils"))
	{
		for (const configuru::Config& stencilConfig : pt["boxStencils"].as_array())
		{
			if (stencilConfig.has_key("stencil_id"))
			{
				MikanStencilBox stencil;
				memset(&stencil, 0, sizeof(stencil));

				stencil.stencil_id = stencilConfig.get<int>("stencil_id");
				stencil.parent_anchor_id = stencilConfig.get_or<int>("parent_anchor_id", -1);
				readVector3f(stencilConfig, "box_center", stencil.box_center);
				readVector3f(stencilConfig, "box_x_axis", stencil.box_x_axis);
				readVector3f(stencilConfig, "box_y_axis", stencil.box_y_axis);
				readVector3f(stencilConfig, "box_z_axis", stencil.box_z_axis);
				stencil.box_x_size = stencilConfig.get_or<float>("box_x_size", 0.25f);
				stencil.box_y_size = stencilConfig.get_or<float>("box_y_size", 0.25f);
				stencil.box_z_size = stencilConfig.get_or<float>("box_z_size", 0.25f);
				stencil.is_disabled = stencilConfig.get_or<bool>("is_disabled", false);

				const std::string stencil_name = stencilConfig.get_or<std::string>("stencil_name", "");
				StringUtils::formatString(stencil.stencil_name, sizeof(stencil.stencil_name), "%s", stencil_name.c_str());

				boxStencilList.push_back(stencil);
			}
		}
	}
	
	// Read in the model stencils
	modelStencilList.clear();
	if (pt.has_key("modelStencils"))
	{
		for (const configuru::Config& stencilConfig : pt["modelStencils"].as_array())
		{
			if (stencilConfig.has_key("stencil_id"))
			{
				MikanStencilModelConfig modelConfig;
				MikanStencilModel& modelInfo= modelConfig.modelInfo;
				memset(&modelInfo, 0, sizeof(MikanStencilModel));

				modelConfig.modelPath= stencilConfig.get<std::string>("model_path");
				modelInfo.stencil_id = stencilConfig.get<int>("stencil_id");
				modelInfo.parent_anchor_id = stencilConfig.get_or<int>("parent_anchor_id", -1);
				readVector3f(stencilConfig, "model_position", modelInfo.model_position);
				readRotator3f(stencilConfig, "model_rotator", modelInfo.model_rotator);
				readVector3f(stencilConfig, "model_scale", modelInfo.model_scale);
				modelInfo.is_disabled = stencilConfig.get_or<bool>("is_disabled", false);

				const std::string stencil_name = stencilConfig.get_or<std::string>("stencil_name", "");
				StringUtils::formatString(modelInfo.stencil_name, sizeof(modelInfo.stencil_name), "%s", stencil_name.c_str());

				modelStencilList.push_back(modelConfig);
			}
		}
	}

	// Output Path
	outputFilePath = pt.get_or<std::string>("outputPath", outputFilePath.string());
}

bool ProfileConfig::getSpatialAnchorInfo(
	MikanSpatialAnchorID anchorId,
	MikanSpatialAnchorInfo& outInfo) const
{
	auto it = std::find_if(
		spatialAnchorList.begin(), spatialAnchorList.end(),
		[anchorId](const MikanSpatialAnchorInfo& info) {
		return info.anchor_id == anchorId;
	});

	if (it != spatialAnchorList.end())
	{
		outInfo = *it;
		return true;
	}

	return false;
}

bool ProfileConfig::getSpatialAnchorWorldTransform(
	MikanSpatialAnchorID anchorId,
	glm::mat4& outXform) const
{
	MikanSpatialAnchorInfo anchor;

	if (getSpatialAnchorInfo(anchorId, anchor))
	{
		outXform= MikanMatrix4f_to_glm_mat4(anchor.anchor_xform);
		return true;
	}

	return false;
}

MikanSpatialAnchorID ProfileConfig::getNextSpatialAnchorId(MikanSpatialAnchorID anchorId) const
{
	// If the given anchor id is invalid, return the first valid anchor id
	if (anchorId == INVALID_MIKAN_ID)
	{
		return spatialAnchorList.size() > 0 ? spatialAnchorList[0].anchor_id : INVALID_MIKAN_ID;
	}

	// Find the list index for the anchor with the matching ID
	int listIndex= 0;
	while (listIndex < spatialAnchorList.size())
	{
		if (spatialAnchorList[listIndex].anchor_id == anchorId)
			break;

		++listIndex;
	}

	// Move to the next entry in the list
	++listIndex;

	// If this list entry is still in the list, return it
	return (listIndex < spatialAnchorList.size()) ? spatialAnchorList[listIndex].anchor_id : INVALID_MIKAN_ID;
}

bool ProfileConfig::findSpatialAnchorInfoByName(
	const char* anchorName,
	MikanSpatialAnchorInfo& outInfo) const
{
	auto it = std::find_if(
		spatialAnchorList.begin(), spatialAnchorList.end(),
		[anchorName](const MikanSpatialAnchorInfo& info) {
		return strncmp(info.anchor_name, anchorName, MAX_MIKAN_ANCHOR_NAME_LEN) == 0;
	});

	if (it != spatialAnchorList.end())
	{
		outInfo = *it;
		return true;
	}

	return false;
}

bool ProfileConfig::canAddAnchor() const
{
	return (spatialAnchorList.size() < MAX_MIKAN_SPATIAL_ANCHORS);
}

bool ProfileConfig::addNewAnchor(const char* anchorName, const MikanMatrix4f& xform)
{
	if (!canAddAnchor())
		return false;

	MikanSpatialAnchorInfo anchor;
	memset(&anchor, 0, sizeof(MikanSpatialAnchorInfo));
	anchor.anchor_id= nextAnchorId;
	strncpy(anchor.anchor_name, anchorName, sizeof(anchor.anchor_name)-1);
	anchor.anchor_xform= xform;
	nextAnchorId++;

	spatialAnchorList.push_back(anchor);
	markDirty();

	return true;
}

bool ProfileConfig::updateAnchor(const MikanSpatialAnchorInfo& info)
{
	MikanSpatialAnchorID anchorId= info.anchor_id;

	auto it = std::find_if(
		spatialAnchorList.begin(), spatialAnchorList.end(),
		[anchorId](const MikanSpatialAnchorInfo& info) {
		return info.anchor_id == anchorId;
	});

	if (it != spatialAnchorList.end())
	{
		*it= info;
		markDirty();
		return true;
	}

	return false;
}

bool ProfileConfig::removeAnchor(MikanSpatialAnchorID anchorId)
{
	auto it = std::find_if(
		spatialAnchorList.begin(), spatialAnchorList.end(),
		[anchorId](const MikanSpatialAnchorInfo& info) {
		return info.anchor_id == anchorId;
	});

	if (it != spatialAnchorList.end())
	{
		spatialAnchorList.erase(it);
		markDirty();

		return true;
	}

	return false;
}

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

			if (getModelStencilWorldTransform(fastener->parent_object_id, xform))
				return xform;
			else if (getBoxStencilWorldTransform(fastener->parent_object_id, xform))
				return xform;
			else if (getQuadStencilWorldTransform(fastener->parent_object_id, xform))
				return xform;
		}
	}

	return glm::mat4(1.f);
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

bool ProfileConfig::getQuadStencilInfo(MikanStencilID stencilId, MikanStencilQuad& outQuad) const
{
	auto it = std::find_if(
		quadStencilList.begin(), quadStencilList.end(),
		[stencilId](const MikanStencilQuad& quad) {
		return quad.stencil_id == stencilId;
	});

	if (it != quadStencilList.end())
	{
		outQuad = *it;
		return true;
	}

	return false;
}

bool ProfileConfig::getBoxStencilInfo(MikanStencilID stencilId, MikanStencilBox& outBox) const
{
	auto it = std::find_if(
		boxStencilList.begin(), boxStencilList.end(),
		[stencilId](const MikanStencilBox& box) {
		return box.stencil_id == stencilId;
	});

	if (it != boxStencilList.end())
	{
		outBox = *it;
		return true;
	}

	return false;
}

bool ProfileConfig::canAddStencil() const
{
	return (boxStencilList.size() + quadStencilList.size() + modelStencilList.size() < MAX_MIKAN_STENCILS);
}

bool ProfileConfig::removeStencil(MikanStencilID stencilId)
{
	// Try quad stencil list first
	{
		auto it = std::find_if(
			quadStencilList.begin(), quadStencilList.end(),
			[stencilId](const MikanStencilQuad& q) {
			return q.stencil_id == stencilId;
		});

		if (it != quadStencilList.end())
		{
			quadStencilList.erase(it);
			markDirty();

			return true;
		}
	}

	// Then try the box stencil list
	{
		auto it = std::find_if(
			boxStencilList.begin(), boxStencilList.end(),
			[stencilId](const MikanStencilBox& b) {
			return b.stencil_id == stencilId;
		});

		if (it != boxStencilList.end())
		{
			boxStencilList.erase(it);
			markDirty();

			return true;
		}
	}

	// Then try model stencil list last
	{
		auto it = std::find_if(
			modelStencilList.begin(), modelStencilList.end(),
			[stencilId](const MikanStencilModelConfig& m) {
			return m.modelInfo.stencil_id == stencilId;
		});

		if (it != modelStencilList.end())
		{
			modelStencilList.erase(it);
			markDirty();

			return true;
		}
	}


	return false;
}

eStencilType ProfileConfig::getStencilType(MikanStencilID stencilId) const
{
	// Try quad stencil list first
	{
		auto it = std::find_if(
			quadStencilList.begin(), quadStencilList.end(),
			[stencilId](const MikanStencilQuad& q) {
			return q.stencil_id == stencilId;
		});

		if (it != quadStencilList.end())
		{
			return eStencilType::quad;
		}
	}

	// Then try the box stencil list
	{
		auto it = std::find_if(
			boxStencilList.begin(), boxStencilList.end(),
			[stencilId](const MikanStencilBox& b) {
			return b.stencil_id == stencilId;
		});

		if (it != boxStencilList.end())
		{
			return eStencilType::box;
		}
	}

	// Then try model stencil list last
	{
		auto it = std::find_if(
			modelStencilList.begin(), modelStencilList.end(),
			[stencilId](const MikanStencilModelConfig& m) {
			return m.modelInfo.stencil_id == stencilId;
		});

		if (it != modelStencilList.end())
		{
			return eStencilType::model;
		}
	}


	return eStencilType::INVALID;
}

bool ProfileConfig::getStencilName(MikanStencilID stencilId, std::string& outStencilName) const
{
	// Try quad stencil list first
	{
		auto it = std::find_if(
			quadStencilList.begin(), quadStencilList.end(),
			[stencilId](const MikanStencilQuad& q) {
			return q.stencil_id == stencilId;
		});

		if (it != quadStencilList.end())
		{
			outStencilName= it->stencil_name;
			return true;
		}
	}

	// Then try the box stencil list
	{
		auto it = std::find_if(
			boxStencilList.begin(), boxStencilList.end(),
			[stencilId](const MikanStencilBox& b) {
			return b.stencil_id == stencilId;
		});

		if (it != boxStencilList.end())
		{
			outStencilName= it->stencil_name;
			return true;
		}
	}

	// Then try model stencil list last
	{
		auto it = std::find_if(
			modelStencilList.begin(), modelStencilList.end(),
			[stencilId](const MikanStencilModelConfig& m) {
			return m.modelInfo.stencil_id == stencilId;
		});

		if (it != modelStencilList.end())
		{
			outStencilName= it->modelInfo.stencil_name;
			return true;
		}
	}


	return false;
}

MikanStencilID ProfileConfig::addNewQuadStencil(const MikanStencilQuad& quad)
{
	if (!canAddStencil())
		return INVALID_MIKAN_ID;

	MikanStencilQuad newStencil= quad;	
	newStencil.stencil_id = nextStencilId;
	nextStencilId++;

	quadStencilList.push_back(newStencil);
	markDirty();

	return newStencil.stencil_id;
}

bool ProfileConfig::updateQuadStencil(const MikanStencilQuad& quad)
{
	MikanStencilID stencilId = quad.stencil_id;

	auto it = std::find_if(
		quadStencilList.begin(), quadStencilList.end(),
		[stencilId](const MikanStencilQuad& q) {
		return q.stencil_id == stencilId;
	});

	if (it != quadStencilList.end())
	{
		*it = quad;
		markDirty();
		return true;
	}

	return false;
}

bool ProfileConfig::getQuadStencilWorldTransform(MikanStencilID stencilId, glm::mat4& outXform) const
{
	MikanStencilQuad stencil;
	if (getQuadStencilInfo(stencilId, stencil))
	{
		outXform = getQuadStencilWorldTransform(&stencil);
		return true;
	}

	return false;
}

glm::mat4 ProfileConfig::getQuadStencilWorldTransform(
	const MikanStencilQuad* stencil) const
{
	const glm::vec3 xAxis = MikanVector3f_to_glm_vec3(stencil->quad_x_axis);
	const glm::vec3 yAxis = MikanVector3f_to_glm_vec3(stencil->quad_y_axis);
	const glm::vec3 zAxis = MikanVector3f_to_glm_vec3(stencil->quad_normal);
	const glm::vec3 position = MikanVector3f_to_glm_vec3(stencil->quad_center);
	const glm::mat4 localXform =
		glm::mat4(
			glm::vec4(xAxis, 0.f),
			glm::vec4(yAxis, 0.f),
			glm::vec4(zAxis, 0.f),
			glm::vec4(position, 1.f));

	glm::mat4 worldXform;
	MikanSpatialAnchorInfo anchor;
	if (getSpatialAnchorInfo(stencil->parent_anchor_id, anchor))
	{		
		worldXform= MikanMatrix4f_to_glm_mat4(anchor.anchor_xform) * localXform;
	}
	else
	{
		worldXform = localXform;
	}

	return worldXform;
}

MikanStencilID ProfileConfig::addNewBoxStencil(const MikanStencilBox& box)
{
	if (!canAddStencil())
		return INVALID_MIKAN_ID;

	MikanStencilBox newStencil = box;
	newStencil.stencil_id = nextStencilId;
	nextStencilId++;

	boxStencilList.push_back(newStencil);
	markDirty();

	return newStencil.stencil_id;
}

bool ProfileConfig::updateBoxStencil(const MikanStencilBox& box)
{
	MikanStencilID stencilId = box.stencil_id;

	auto it = std::find_if(
		boxStencilList.begin(), boxStencilList.end(),
		[stencilId](const MikanStencilBox& b) {
			return b.stencil_id == stencilId;
		});

	if (it != boxStencilList.end())
	{
		*it = box;
		markDirty();
		return true;
	}

	return false;
}

bool ProfileConfig::getBoxStencilWorldTransform(MikanStencilID stencilId, glm::mat4& outXform) const
{
	MikanStencilBox stencil;
	if (getBoxStencilInfo(stencilId, stencil))
	{
		outXform = getBoxStencilWorldTransform(&stencil);
		return true;
	}

	return false;
}

glm::mat4 ProfileConfig::getBoxStencilWorldTransform(
	const MikanStencilBox* stencil) const
{
	const glm::vec3 xAxis = MikanVector3f_to_glm_vec3(stencil->box_x_axis);
	const glm::vec3 yAxis = MikanVector3f_to_glm_vec3(stencil->box_y_axis);
	const glm::vec3 zAxis = MikanVector3f_to_glm_vec3(stencil->box_z_axis);
	const glm::vec3 position = MikanVector3f_to_glm_vec3(stencil->box_center);
	const glm::mat4 localXform =
		glm::mat4(
			glm::vec4(xAxis, 0.f),
			glm::vec4(yAxis, 0.f),
			glm::vec4(zAxis, 0.f),
			glm::vec4(position, 1.f));

	glm::mat4 worldXform;
	MikanSpatialAnchorInfo anchor;
	if (getSpatialAnchorInfo(stencil->parent_anchor_id, anchor))
	{
		worldXform = MikanMatrix4f_to_glm_mat4(anchor.anchor_xform) * localXform;
	}
	else
	{
		worldXform = localXform;
	}

	return worldXform;
}

const MikanStencilModelConfig* ProfileConfig::getModelStencilConfig(MikanStencilID stencilId) const
{
	auto it = std::find_if(
		modelStencilList.begin(),
		modelStencilList.end(),
		[stencilId](const MikanStencilModelConfig& stencil) {
			return stencil.modelInfo.stencil_id == stencilId;
		});

	if (it != modelStencilList.end())
	{
		const MikanStencilModelConfig& modelConfig = *it;

		return &modelConfig;
	}
	else
	{
		return nullptr;
	}
}

bool ProfileConfig::getModelStencilInfo(MikanStencilID stencilId, MikanStencilModel& outInfo) const
{
	auto it = std::find_if(
		modelStencilList.begin(), modelStencilList.end(),
		[stencilId](const MikanStencilModelConfig& modelConfig) {
		return modelConfig.modelInfo.stencil_id == stencilId;
	});

	if (it != modelStencilList.end())
	{
		outInfo = it->modelInfo;
		return true;
	}

	return false;
}

bool ProfileConfig::getModelStencilWorldTransform(MikanStencilID stencilId, glm::mat4& outXform) const
{
	MikanStencilModel stencil;
	if (getModelStencilInfo(stencilId, stencil))
	{
		outXform= getModelStencilWorldTransform(&stencil);
		return true;
	}

	return false;
}

glm::mat4 ProfileConfig::getModelStencilWorldTransform(const MikanStencilModel* stencil) const
{
	MikanVector3f stencilXAxis, stencilYAxis, stencilZAxis;
	EulerAnglesToMikanOrientation(
		stencil->model_rotator.x_angle, stencil->model_rotator.y_angle, stencil->model_rotator.z_angle,
		stencilXAxis, stencilYAxis, stencilZAxis);

	const glm::vec3 xAxis = MikanVector3f_to_glm_vec3(stencilXAxis) * stencil->model_scale.x;
	const glm::vec3 yAxis = MikanVector3f_to_glm_vec3(stencilYAxis) * stencil->model_scale.y;
	const glm::vec3 zAxis = MikanVector3f_to_glm_vec3(stencilZAxis) * stencil->model_scale.z;
	const glm::vec3 position = MikanVector3f_to_glm_vec3(stencil->model_position);
	const glm::mat4 localXform =
		glm::mat4(
			glm::vec4(xAxis, 0.f),
			glm::vec4(yAxis, 0.f),
			glm::vec4(zAxis, 0.f),
			glm::vec4(position, 1.f));

	glm::mat4 worldXform;
	glm::mat4 anchorXform;
	MikanSpatialAnchorInfo anchor;
	if (getSpatialAnchorWorldTransform(stencil->parent_anchor_id, anchorXform))
	{
		worldXform = anchorXform * localXform;
	}
	else
	{
		worldXform = localXform;
	}

	return worldXform;
}

MikanStencilID ProfileConfig::addNewModelStencil(const MikanStencilModel& modelInfo)
{
	if (!canAddStencil())
		return INVALID_MIKAN_ID;

	MikanStencilModelConfig newStencil;
	newStencil.modelInfo= modelInfo;
	newStencil.modelInfo.stencil_id = nextStencilId;
	nextStencilId++;

	modelStencilList.push_back(newStencil);
	markDirty();

	return newStencil.modelInfo.stencil_id;
}

bool ProfileConfig::updateModelStencil(const MikanStencilModel& info)
{
	MikanStencilID stencilId = info.stencil_id;

	auto it = std::find_if(
		modelStencilList.begin(), modelStencilList.end(),
		[stencilId](const MikanStencilModelConfig& modelConfig) {
		return modelConfig.modelInfo.stencil_id == stencilId;
	});

	if (it != modelStencilList.end())
	{
		it->modelInfo = info;
		markDirty();
		return true;
	}

	return false;
}

bool ProfileConfig::updateModelStencilFilename(
	MikanStencilID stencilId, 
	const std::filesystem::path& filename)
{
	auto it = std::find_if(
		modelStencilList.begin(), modelStencilList.end(),
		[stencilId](const MikanStencilModelConfig& modelConfig) {
		return modelConfig.modelInfo.stencil_id == stencilId;
	});

	if (it != modelStencilList.end())
	{
		it->modelPath= filename;
		markDirty();
		return true;
	}

	return false;
}

std::filesystem::path ProfileConfig::generateTimestampedFilePath(
	const std::string& prefix, 
	const std::string& suffix) const
{
	const std::filesystem::path parentDir= !outputFilePath.empty() ? outputFilePath : std::filesystem::current_path();

	return PathUtils::makeTimestampedFilePath(parentDir, prefix, suffix);
}