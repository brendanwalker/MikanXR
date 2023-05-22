#pragma once

#include "CommonConfigFwd.h"
#include "MikanClientTypes.h"
#include "ObjectSystemFwd.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"

struct RmlModel_SpatialAnchor
{
	int anchor_id;
	Rml::String anchor_name;
};

class RmlModel_SpatialAnchors : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext, AnchorObjectSystemPtr anchorSystemPtr);
	virtual void dispose() override;

	const std::string& getAnchorVRDevicePath() const { return m_anchorVRDevicePath; }

	void rebuildVRDevicePaths();
	void rebuildAnchorList();

	SinglecastDelegate<void()> OnAddNewAnchor;
	SinglecastDelegate<void(const std::string& vrDevicePath)> OnUpdateAnchorVRDevicePath;
	SinglecastDelegate<void(int anchorId)> OnUpdateAnchorPose;
	SinglecastDelegate<void(int anchorId, const std::string& anchorName)> OnUpdateAnchorName;
	SinglecastDelegate<void(int anchorId)> OnDeleteAnchor;
	SinglecastDelegate<void()> OnGotoMainMenu;

private:
	void anchorSystemConfigMarkedDirty(
		CommonConfigPtr configPtr,
		const class ConfigPropertyChangeSet& changedPropertySet);

	Rml::String m_anchorVRDevicePath;
	Rml::Vector<Rml::String> m_vrDeviceList;
	Rml::Vector<RmlModel_SpatialAnchor> m_spatialAnchors;
	int m_maxSpatialAnchors = MAX_MIKAN_SPATIAL_ANCHORS;
	int m_originAnchorId = INVALID_MIKAN_ID;

	AnchorObjectSystemPtr m_anchorSystemPtr;

	static bool s_bHasRegisteredTypes;
};
