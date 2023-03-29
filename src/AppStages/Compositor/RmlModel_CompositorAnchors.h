#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"

struct RmlModel_CompositorAnchor
{
	int anchor_id;
	Rml::Vector<int> child_fastener_ids;
};

class RmlModel_CompositorAnchors : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext, const class ProfileConfig* profile);
	virtual void dispose() override;

	void rebuildAnchorList(const ProfileConfig* profile);
	
	SinglecastDelegate<void()> OnUpdateOriginPose;
	SinglecastDelegate<void(int anchorID)> OnAddFastenerEvent;
	SinglecastDelegate<void(int fastenerID)> OnEditFastenerEvent;
	SinglecastDelegate<void(int anchorID, int fastenerID)> OnDeleteFastenerEvent;

private:
	int m_originAnchorId= -1;
	Rml::Vector<RmlModel_CompositorAnchor> m_spatialAnchors;
	bool m_bShowAnchors;

	static bool s_bHasRegisteredTypes;
};
