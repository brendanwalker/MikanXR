#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"

struct RmlModel_CompositorModel
{
	int stencil_id;
	int parent_anchor_id;
	Rml::String model_path;
	Rml::Vector3f model_position;
	Rml::Vector3f model_angles;
	Rml::Vector3f model_scale;
	bool disabled;
};

class RmlModel_CompositorModels : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext, const class ProfileConfig* profile);
	virtual void dispose() override;

	void rebuildAnchorList(const ProfileConfig* profile);
	void rebuildUIModelsFromProfile(const ProfileConfig* profile);
	void copyUIModelToProfile(int stencil_id, ProfileConfig* profile) const;

	SinglecastDelegate<void()> OnAddModelStencilEvent;
	SinglecastDelegate<void(int stencilID)> OnDeleteModelStencilEvent;
	SinglecastDelegate<void(int stencilID)> OnModifyModelStencilEvent;
	SinglecastDelegate<void(int stencilID)> OnSelectModelStencilPathEvent;

private:
	Rml::Vector<int> m_spatialAnchors;
	Rml::Vector<RmlModel_CompositorModel> m_stencilModels;

	static bool s_bHasRegisteredTypes;
};
