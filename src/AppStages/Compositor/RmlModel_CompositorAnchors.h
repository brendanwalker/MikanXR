#pragma once

#include "ObjectSystemFwd.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"

struct RmlModel_CompositorAnchor
{
	int anchor_id;
};

class RmlModel_CompositorAnchors : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		ProfileConfigPtr profile,
		AnchorObjectSystemPtr anchorSystemPtr);
	virtual void dispose() override;

	void rebuildAnchorList();

private:
	void anchorSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);

	void onUpdateOriginEvent();
	void onAddAnchorEvent();
	void onEditAnchorEvent(MikanSpatialAnchorID anchor_id);

	ProfileConfigPtr m_profile;
	AnchorObjectSystemPtr m_anchorSystemPtr;

	int m_originAnchorId= -1;
	Rml::Vector<RmlModel_CompositorAnchor> m_spatialAnchors;
	bool m_bShowAnchors;

	static bool s_bHasRegisteredTypes;
};
