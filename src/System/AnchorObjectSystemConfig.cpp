#include "AnchorObjectSystemConfig.h"
#include "MathTypeConversion.h"

const configuru::Config AnchorObjectSystemConfig::writeToJSON()
{
	configuru::Config pt = configuru::Config::object();

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

	return pt;
}

void AnchorObjectSystemConfig::readFromJSON(const configuru::Config& pt)
{
	anchorVRDevicePath = pt.get_or<std::string>("anchorVRDevicePath", anchorVRDevicePath);
	nextAnchorId = pt.get_or<int>("nextAnchorId", nextAnchorId);
	debugRenderAnchors = pt.get_or<bool>("debugRenderAnchors", debugRenderAnchors);

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

				std::string anchorName = anchorConfig.get<std::string>("name");
				strncpy(anchorInfo.anchor_name, anchorName.c_str(), sizeof(anchorInfo.anchor_name) - 1);

				anchorInfo.anchor_id = anchorConfig.get<int>("id");

				readMatrix4f(anchorConfig, "xform", anchorInfo.anchor_xform);

				spatialAnchorList.push_back(anchorInfo);
			}
		}
	}

	// Special case: Origin spatial anchor
	const MikanSpatialAnchorInfo* originAnchorInfo= getSpatialAnchorInfoByName(ORIGIN_SPATIAL_ANCHOR_NAME);
	if (originAnchorInfo != nullptr)
	{
		originAnchorId = originAnchorInfo->anchor_id;
	}
	else
	{
		const MikanMatrix4f originXform = glm_mat4_to_MikanMatrix4f(glm::mat4(1.f));

		originAnchorId = nextAnchorId;
		addNewAnchor(ORIGIN_SPATIAL_ANCHOR_NAME, originXform);
	}
}

bool AnchorObjectSystemConfig::canAddAnchor() const
{
	return (spatialAnchorList.size() < MAX_MIKAN_SPATIAL_ANCHORS);
}

const MikanSpatialAnchorInfo* AnchorObjectSystemConfig::getSpatialAnchorInfo(MikanSpatialAnchorID anchorId) const
{
	auto it = std::find_if(
		spatialAnchorList.begin(), spatialAnchorList.end(),
		[anchorId](const MikanSpatialAnchorInfo& info) {
			return info.anchor_id == anchorId;
		});

	if (it != spatialAnchorList.end())
	{
		return &(*it);
	}

	return nullptr;
}

const MikanSpatialAnchorInfo* AnchorObjectSystemConfig::getSpatialAnchorInfoByName(const std::string& anchorName) const
{
	auto it = std::find_if(
		spatialAnchorList.begin(), spatialAnchorList.end(),
		[anchorName](const MikanSpatialAnchorInfo& info) {
			return strncmp(info.anchor_name, anchorName.c_str(), MAX_MIKAN_ANCHOR_NAME_LEN) == 0;
		});

	if (it != spatialAnchorList.end())
	{
		return &(*it);
	}

	return nullptr;
}


MikanSpatialAnchorID AnchorObjectSystemConfig::addNewAnchor(const std::string& anchorName, const MikanMatrix4f& xform)
{
	if (!canAddAnchor())
		return INVALID_MIKAN_ID;

	MikanSpatialAnchorInfo anchor;
	memset(&anchor, 0, sizeof(MikanSpatialAnchorInfo));
	anchor.anchor_id = nextAnchorId;
	strncpy(anchor.anchor_name, anchorName.c_str(), sizeof(anchor.anchor_name) - 1);
	anchor.anchor_xform = xform;
	nextAnchorId++;

	spatialAnchorList.push_back(anchor);
	markDirty();

	return anchor.anchor_id;
}

bool AnchorObjectSystemConfig::removeAnchor(MikanSpatialAnchorID anchorId)
{
	auto it = std::find_if(
		spatialAnchorList.begin(), spatialAnchorList.end(),
		[anchorId](const MikanSpatialAnchorInfo& info) {
		return info.anchor_id == anchorId;
	});

	if (it != spatialAnchorList.end() &&
		it->anchor_id != originAnchorId)
	{
		spatialAnchorList.erase(it);
		markDirty();

		return true;
	}

	return false;
}