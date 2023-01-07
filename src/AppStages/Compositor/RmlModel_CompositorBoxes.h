#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"

struct RmlModel_CompositorBox
{
	int stencil_id;
	int parent_anchor_id;
	Rml::Vector3f box_center;
	Rml::Vector3f angles;
	Rml::Vector3f size;
	bool disabled;
};

class RmlModel_CompositorBoxes : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext, const class ProfileConfig* profile);
	virtual void dispose() override;

	void rebuildAnchorList(const ProfileConfig* profile);
	void rebuildUIBoxesFromProfile(const ProfileConfig* profile);
	void copyUIBoxToProfile(int stencil_id, ProfileConfig* profile) const;

	SinglecastDelegate<void()> OnAddBoxStencilEvent;
	SinglecastDelegate<void(int stencilID)> OnDeleteBoxStencilEvent;
	SinglecastDelegate<void(int stencilID)> OnModifyBoxStencilEvent;

private:
	Rml::Vector<int> m_spatialAnchors;
	Rml::Vector<RmlModel_CompositorBox> m_stencilBoxes;

	static bool s_bHasRegisteredTypes;
};
