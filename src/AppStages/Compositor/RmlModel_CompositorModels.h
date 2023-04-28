#pragma once

#include "ObjectSystemFwd.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"

struct RmlModel_CompositorModel
{
	Rml::String stencil_name;
	int stencil_id;
	int parent_anchor_id;
	Rml::Vector<int> child_fastener_ids;
	Rml::String model_path;
	Rml::Vector3f model_position;
	Rml::Vector3f model_angles;
	Rml::Vector3f model_scale;
	bool disabled;
};

class RmlModel_CompositorModels : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		AnchorObjectSystemPtr anchorSystemPtr,
		StencilObjectSystemPtr objectSystemPtr);
	virtual void dispose() override;

	void rebuildAnchorList();
	void rebuildUIModelsFromProfile();
	void copyUIModelToProfile(int stencil_id) const;

	SinglecastDelegate<void()> OnAddModelStencilEvent;
	SinglecastDelegate<void(int stencilID)> OnDeleteModelStencilEvent;
	SinglecastDelegate<void(int stencilID, int anchorId)> OnModifyModelStencilParentAnchorEvent;
	SinglecastDelegate<void(int stencilID)> OnModifyModelStencilEvent;
	SinglecastDelegate<void(int stencilID)> OnSelectModelStencilPathEvent;
	SinglecastDelegate<void(int stencilID)> OnAddFastenerEvent;
	SinglecastDelegate<void(int fastenerID)> OnSnapFastenerEvent;
	SinglecastDelegate<void(int fastenerID)> OnEditFastenerEvent;
	SinglecastDelegate<void(int stencilID, int fastenerID)> OnDeleteFastenerEvent;

private:
	AnchorObjectSystemPtr m_anchorSystemPtr;
	StencilObjectSystemPtr m_stencilSystemPtr;
	Rml::Vector<int> m_spatialAnchors;
	Rml::Vector<RmlModel_CompositorModel> m_stencilModels;

	static bool s_bHasRegisteredTypes;
};
